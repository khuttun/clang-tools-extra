//===--- UnusedReturnValueCheck.cpp - clang-tidy---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "UnusedReturnValueCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;

namespace clang {
namespace tidy {
namespace bugprone {

namespace {

// Same as matchesName, but instead of fully qualified name, matches on a name
// where inline namespaces have been ignored.
AST_MATCHER_P(NamedDecl, matchesInlinedName, std::string, RegExp) {
  assert(!RegExp.empty());
  llvm::SmallString<128> InlinedName("::");
  llvm::raw_svector_ostream OS(InlinedName);
  PrintingPolicy Policy = Node.getASTContext().getPrintingPolicy();
  Policy.SuppressUnwrittenScope = true;
  Node.printQualifiedName(OS, Policy);
  return llvm::Regex(RegExp).match(OS.str());
}

} // namespace

UnusedReturnValueCheck::UnusedReturnValueCheck(llvm::StringRef Name,
                                               ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      FuncRegex(Options.get("FunctionsRegex",
                            "^::std::async$|"
                            "^::std::launder$|"
                            "^::std::remove$|"
                            "^::std::remove_if$|"
                            "^::std::unique$|"
                            "^::std::unique_ptr<.*>::release$|"
                            "^::std::.*::allocate$|"
                            "^::std::(.*::)*empty$")) {}

void UnusedReturnValueCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "FunctionsRegex", FuncRegex);
}

void UnusedReturnValueCheck::registerMatchers(MatchFinder *Finder) {
  auto MatchedCallExpr = expr(ignoringImplicit(ignoringParenImpCasts(
      callExpr(callee(functionDecl(
                   matchesInlinedName(FuncRegex),
                   // Don't match void overloads of checked functions.
                   unless(returns(voidType())))))
          .bind("match"))));

  auto UnusedInCompoundStmt =
      compoundStmt(forEach(MatchedCallExpr),
                   // The checker can't currently differentiate between the
                   // return statement and other statements inside GNU statement
                   // expressions, so disable the checker inside them to avoid
                   // false positives.
                   unless(hasParent(stmtExpr())));
  auto UnusedInIfStmt =
      ifStmt(eachOf(hasThen(MatchedCallExpr), hasElse(MatchedCallExpr)));
  auto UnusedInWhileStmt = whileStmt(hasBody(MatchedCallExpr));
  auto UnusedInDoStmt = doStmt(hasBody(MatchedCallExpr));
  auto UnusedInForStmt =
      forStmt(eachOf(hasLoopInit(MatchedCallExpr),
                     hasIncrement(MatchedCallExpr), hasBody(MatchedCallExpr)));
  auto UnusedInRangeForStmt = cxxForRangeStmt(hasBody(MatchedCallExpr));
  auto UnusedInCaseStmt = switchCase(forEach(MatchedCallExpr));

  Finder->addMatcher(
      stmt(anyOf(UnusedInCompoundStmt, UnusedInIfStmt, UnusedInWhileStmt,
                 UnusedInDoStmt, UnusedInForStmt, UnusedInRangeForStmt,
                 UnusedInCaseStmt)),
      this);
}

void UnusedReturnValueCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto *Matched = Result.Nodes.getNodeAs<CallExpr>("match")) {
    diag(Matched->getLocStart(),
         "the value returned by this function should be used")
        << Matched->getSourceRange();
    diag(Matched->getLocStart(),
         "cast the expression to void to silence this warning",
         DiagnosticIDs::Note);
  }
}

} // namespace bugprone
} // namespace tidy
} // namespace clang
