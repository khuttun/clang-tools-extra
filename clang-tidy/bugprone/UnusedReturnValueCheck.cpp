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
      FuncList(Options.get("Functions", "::std::async;"
                                        "::std::launder;"
                                        "::std::.*::allocate;"
                                        "::std::.*::empty")) {}

void UnusedReturnValueCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "Functions", FuncList);
}

void UnusedReturnValueCheck::registerMatchers(MatchFinder *Finder) {
  // Detect unused return values by finding CallExprs with CompoundStmt parent,
  // ignoring any implicit nodes, parentheses and casts in between

  // full regex matcher
  auto Regex = FuncList;
  std::replace(Regex.begin(), Regex.end(), ';', '|');
  Finder->addMatcher(
      compoundStmt(forEach(expr(ignoringImplicit(ignoringParenCasts(
          callExpr(callee(functionDecl(matchesName(Regex)))).bind("match")))))),
      this);

  // variadic matcher
  //  const auto FunNames = utils::options::parseStringList(FuncList);
  //  std::vector<DynTypedMatcher> Matchers;
  //  std::transform(
  //      FunNames.begin(), FunNames.end(), std::back_inserter(Matchers),
  //      [](const std::string &Name) {
  //        return expr(ignoringImplicit(ignoringParenCasts(
  //            callExpr(callee(functionDecl(matchesName(Name)))).bind("match"))));
  //      });
  //  const auto AnyCheckedCall =
  //      DynTypedMatcher::constructVariadic(
  //          DynTypedMatcher::VO_AnyOf,
  //          ast_type_traits::ASTNodeKind::getFromNodeKind<Stmt>(), Matchers)
  //          .convertTo<Stmt>();
  //  Finder->addMatcher(compoundStmt(forEach(AnyCheckedCall)), this);

  // Match function calls
  //  for (const StringRef FunName : FunNameVec) {
  //    const auto FunCallM = expr(ignoringImplicit(ignoringParenCasts(
  //        callExpr(callee(functionDecl(matchesName(FunName)))).bind("match"))));
  //    Matchers.push_back(FunCallM);
  //  }

  // Match member function calls
  //  for (const StringRef MemFunName : FunNameVec) {
  //    const auto Idx = MemFunName.rfind("::");
  //    if (Idx != StringRef::npos) {
  //      const auto TypeName = MemFunName.slice(0, Idx);
  //      const auto MethodName = MemFunName.slice(Idx + 2, StringRef::npos);
  //      if (!TypeName.empty() && !MethodName.empty()) {
  //        const auto TypeM = hasCanonicalType(hasDeclaration(
  //            cxxRecordDecl(isSameOrDerivedFrom(matchesName(TypeName)))));
  //        const auto MethodM = cxxMethodDecl(hasName(MethodName));
  //        const auto MemFunCallM = expr(ignoringImplicit(ignoringParenCasts(
  //            cxxMemberCallExpr(thisPointerType(TypeM), callee(MethodM))
  //                .bind("match"))));
  //        Matchers.push_back(MemFunCallM);
  //      }
  //    }
  //  }
}

void UnusedReturnValueCheck::check(const MatchFinder::MatchResult &Result) {
  if (const auto Matched = Result.Nodes.getNodeAs<Stmt>("match")) {
    diag(Matched->getLocStart(), "unused return value");
  }
}

} // namespace bugprone
} // namespace tidy
} // namespace clang
