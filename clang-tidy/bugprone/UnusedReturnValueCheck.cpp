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

using namespace clang::ast_matchers;
using namespace clang::ast_matchers::internal;

namespace clang {
namespace tidy {
namespace bugprone {

UnusedReturnValueCheck::UnusedReturnValueCheck(llvm::StringRef Name,
                                               ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      FuncRegex(Options.get("FunctionsRegex",
                            "^::std::async$|"
                            "^::std::launder$|"
                            "^::std::unique_ptr<.*>::release$|"
                            "^::std::.*::allocate$|"
                            "^::std::.*::empty$")) {}

void UnusedReturnValueCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "FunctionsRegex", FuncRegex);
}

void UnusedReturnValueCheck::registerMatchers(MatchFinder *Finder) {
  // Detect unused return values by finding CallExprs with CompoundStmt parent,
  // ignoring any implicit nodes, parentheses and casts in between
  Finder->addMatcher(
      compoundStmt(forEach(expr(ignoringImplicit(ignoringParenCasts(
          callExpr(callee(functionDecl(matchesName(FuncRegex))))
              .bind("match")))))),
      this);
}

void UnusedReturnValueCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto Matched = Result.Nodes.getNodeAs<Stmt>("match")) {
    diag(Matched->getLocStart(), "unused return value");
  }
}

} // namespace bugprone
} // namespace tidy
} // namespace clang
