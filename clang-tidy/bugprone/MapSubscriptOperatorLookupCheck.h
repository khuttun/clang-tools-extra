//===--- MapSubscriptOperatorLookupCheck.h - clang-tidy----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_MAPSUBSCRIPTOPERATORLOOKUPCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_MAPSUBSCRIPTOPERATORLOOKUPCHECK_H

#include "../ClangTidy.h"

namespace clang {
namespace tidy {
namespace bugprone {

/// Detects map lookups done with operator[].
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/bugprone-map-subscript-operator-lookup.html
class MapSubscriptOperatorLookupCheck : public ClangTidyCheck {
public:
  MapSubscriptOperatorLookupCheck(StringRef Name, ClangTidyContext *Context);
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  std::string MapTypes;
};

} // namespace bugprone
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_BUGPRONE_MAPSUBSCRIPTOPERATORLOOKUPCHECK_H
