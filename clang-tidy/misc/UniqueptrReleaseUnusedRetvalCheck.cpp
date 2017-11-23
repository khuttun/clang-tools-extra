//===--- UniqueptrReleaseUnusedRetvalCheck.cpp - clang-tidy----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "UniqueptrReleaseUnusedRetvalCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace misc {

void UniqueptrReleaseUnusedRetvalCheck::registerMatchers(MatchFinder *Finder) {
  // match on release() calls with CompoundStmt parent (= unused)
  auto UniquePtrType = hasCanonicalType(
      hasDeclaration(cxxRecordDecl(isSameOrDerivedFrom("::std::unique_ptr"))));
  auto ReleaseMethod = cxxMethodDecl(hasName("release"));
  auto UnusedRetVal = hasParent(compoundStmt());
  Finder->addMatcher(cxxMemberCallExpr(thisPointerType(UniquePtrType),
                                       callee(ReleaseMethod), UnusedRetVal)
                         .bind("match"),
                     this);
}

void UniqueptrReleaseUnusedRetvalCheck::check(
    const MatchFinder::MatchResult &Result) {
  if (const auto Matched = Result.Nodes.getNodeAs<Stmt>("match")) {
    diag(Matched->getLocStart(),
         "unused std::unique_ptr::release return value");
  }
}

} // namespace misc
} // namespace tidy
} // namespace clang
