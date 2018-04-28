//===--- MapSubscriptOperatorLookupCheck.cpp - clang-tidy------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MapSubscriptOperatorLookupCheck.h"
#include "../utils/OptionsUtils.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace bugprone {

MapSubscriptOperatorLookupCheck::MapSubscriptOperatorLookupCheck(
    llvm::StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      MapTypes(Options.get("MapTypes", "::std::map;::std::unordered_map")) {}

void MapSubscriptOperatorLookupCheck::storeOptions(
    ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "MapTypes", MapTypes);
}

void MapSubscriptOperatorLookupCheck::registerMatchers(MatchFinder *Finder) {
  if (!getLangOpts().CPlusPlus)
    return;

  auto MapTypeVec = utils::options::parseStringList(MapTypes);
  Finder->addMatcher(
      cxxOperatorCallExpr(
          callee(cxxMethodDecl(hasName("operator[]"),
                               ofClass(hasAnyName(std::vector<StringRef>(
                                   MapTypeVec.begin(), MapTypeVec.end()))))),
          hasParent(implicitCastExpr(
              anyOf(hasImplicitDestinationType(isConstQualified()),
                    hasCastKind(CK_LValueToRValue)))))
          .bind("match"),
      this);
}

void MapSubscriptOperatorLookupCheck::check(
    const MatchFinder::MatchResult &Result) {
  if (const auto *Matched = Result.Nodes.getNodeAs<CallExpr>("match")) {
    diag(Matched->getLocStart(), "do not use operator[] for lookup")
        << Matched->getSourceRange();
    diag(Matched->getLocStart(), "consider using find() instead",
         DiagnosticIDs::Note);
  }
}

} // namespace bugprone
} // namespace tidy
} // namespace clang
