// RUN: %check_clang_tidy %s bugprone-map-subscript-operator-lookup %t

namespace std {

template <typename T>
struct allocator {};

template <typename T>
struct equal_to {};

template <typename T>
struct hash {};

template <typename T>
struct less {};

template <typename T, typename U>
struct pair {};

template <typename Key,
          typename T,
          typename Compare = std::less<Key>,
          typename Allocator = std::allocator<std::pair<const Key, T>>>
struct map {
  T &operator[](const Key &);
  T &operator[](Key &&);
};

// the check should be able to match std lib calls even if the functions are
// declared inside inline namespaces
inline namespace v1 {

template <typename Key,
          typename T,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = std::allocator<std::pair<const Key, T>>>
struct unordered_map {
  T &operator[](const Key &);
  T &operator[](Key &&);
};

} // namespace v1
} // namespace std

struct Foo {
  int getter() const;
  void setter(int);
};

void warning() {
  std::map<int, Foo> Map;
  auto Foo1 = Map[42];
  // CHECK-MESSAGES: [[@LINE-1]]:15: warning: do not use operator[] for lookup [bugprone-map-subscript-operator-lookup]

  std::unordered_map<int, Foo> UMap;
  auto Foo2 = UMap[42];
  // CHECK-MESSAGES: [[@LINE-1]]:15: warning: do not use operator[] for lookup [bugprone-map-subscript-operator-lookup]
}

void noWarning() {
  std::map<int, Foo> MapOk;
  MapOk[42] = Foo{};

  std::unordered_map<int, Foo> UMapOk;
  UMapOk[42] = Foo{};
}
