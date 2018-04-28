// RUN: %check_clang_tidy %s bugprone-map-subscript-operator-lookup %t \
// RUN: -config='{CheckOptions: \
// RUN:  [{key: bugprone-map-subscript-operator-lookup.MapTypes, \
// RUN:    value: "::ns::Map"}]}' \
// RUN: --

namespace ns {

template <typename K, typename V>
struct Map {
  V &operator[](const K &);
  V &operator[](K &&);
};

template <typename K, typename V>
struct DerivedMap : public Map<K, V> {};

using IntMap = Map<int, int>;

} // namespace ns

void warning() {
  ns::Map<int, int> M;
  auto A = M[1];
  // CHECK-MESSAGES: [[@LINE-1]]:12: warning: do not use operator[] for lookup [bugprone-map-subscript-operator-lookup]

  ns::DerivedMap<int, int> DM;
  auto B = DM[1];
  // CHECK-MESSAGES: [[@LINE-1]]:12: warning: do not use operator[] for lookup [bugprone-map-subscript-operator-lookup]

  ns::IntMap IM;
  auto C = IM[1];
  // CHECK-MESSAGES: [[@LINE-1]]:12: warning: do not use operator[] for lookup [bugprone-map-subscript-operator-lookup]
}

void noWarning() {
  ns::Map<int, int> MOk;
  MOk[1] = 2;

  ns::DerivedMap<int, int> DMOk;
  DMOk[1] = 2;

  ns::IntMap IMOk;
  IMOk[1] = 2;
}
