// RUN: %check_clang_tidy %s bugprone-unused-return-value %t

namespace std {

using size_t = decltype(sizeof(int));

using max_align_t = long double;

struct future {};

enum class launch {
  async,
  deferred
};

template <typename Function, typename... Args>
future async(Function &&, Args &&...);

template <typename Function, typename... Args>
future async(launch, Function &&, Args &&...);

template <typename T>
T *launder(T *);

template <typename T>
struct allocator {
  using value_type = T;
  T *allocate(std::size_t);
  T *allocate(std::size_t, const void *);
};

template <typename Alloc>
struct allocator_traits {
  using value_type = typename Alloc::value_type;
  using pointer = value_type *;
  using size_type = size_t;
  using const_void_pointer = const void *;
  static pointer allocate(Alloc &, size_type);
  static pointer allocate(Alloc &, size_type, const_void_pointer);
};

template <typename OuterAlloc, typename... InnerAlloc>
struct scoped_allocator_adaptor : public OuterAlloc {
  using pointer = typename allocator_traits<OuterAlloc>::pointer;
  using size_type = typename allocator_traits<OuterAlloc>::size_type;
  using const_void_pointer = typename allocator_traits<OuterAlloc>::const_void_pointer;
  pointer allocate(size_type);
  pointer allocate(size_type, const_void_pointer);
};

template <typename T, typename Allocator = allocator<T>>
struct vector {
  bool empty() const noexcept;
};

namespace filesystem {

struct path {
  bool empty() const noexcept;
};

} // namespace filesystem

namespace pmr {

struct memory_resource {
  void *allocate(size_t, size_t = alignof(max_align_t));
};

template <typename T>
struct polymorphic_allocator {
  T *allocate(size_t);
};

} // namespace pmr

} // namespace std

struct Foo {
  void f();
};

int increment(int i) {
  return i + 1;
}

void useFuture(const std::future &fut);

struct FooAlloc {
  using value_type = Foo;
};

void WarningBasic() {
  std::async(increment, 42);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  std::async(std::launch::async, increment, 42);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  Foo F;
  std::launder(&F);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  std::allocator<Foo> FA;
  FA.allocate(1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]
  FA.allocate(1, nullptr);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  std::allocator_traits<std::allocator<Foo>>::allocate(FA, 1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]
  std::allocator_traits<std::allocator<Foo>>::allocate(FA, 1, nullptr);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  std::scoped_allocator_adaptor<FooAlloc, FooAlloc> SAA;
  SAA.allocate(1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]
  SAA.allocate(1, nullptr);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  std::vector<Foo> FV;
  FV.empty();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  std::filesystem::path P;
  P.empty();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  std::pmr::memory_resource MR;
  MR.allocate(1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  std::pmr::polymorphic_allocator<Foo> PA;
  PA.allocate(1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]
}

void WarningSpecial() {
  (std::async(increment, 42));
  // CHECK-MESSAGES: [[@LINE-1]]:4: warning: unused return value [bugprone-unused-return-value]

  Foo FCast;
  reinterpret_cast<char *>(std::launder(&FCast));
  // CHECK-MESSAGES: [[@LINE-1]]:28: warning: unused return value [bugprone-unused-return-value]

  std::vector<Foo> FVParenCast;
  (FVParenCast.empty());
  // CHECK-MESSAGES: [[@LINE-1]]:4: warning: unused return value [bugprone-unused-return-value]
  reinterpret_cast<char *>(FVParenCast.empty());
  // CHECK-MESSAGES: [[@LINE-1]]:28: warning: unused return value [bugprone-unused-return-value]
}

void NoWarning() {
  auto AsyncRetval1 = std::async(increment, 42);
  auto AsyncRetval2 = std::async(std::launch::async, increment, 42);
  useFuture(std::async(increment, 42));

  Foo FNoWarning;
  auto LaunderRetval = std::launder(&FNoWarning);
  std::launder(&FNoWarning)->f();
  delete std::launder(&FNoWarning);

  std::allocator<Foo> FANoWarning;
  auto AllocRetval1 = FANoWarning.allocate(1, nullptr);
  auto AllocRetval2 = FANoWarning.allocate(1);

  auto AllocRetval3 = std::allocator_traits<std::allocator<Foo>>::allocate(FANoWarning, 1);
  auto AllocRetval4 = std::allocator_traits<std::allocator<Foo>>::allocate(FANoWarning, 1, nullptr);

  std::scoped_allocator_adaptor<FooAlloc, FooAlloc> SAANoWarning;
  auto AllocRetval5 = SAANoWarning.allocate(1);
  auto AllocRetval6 = SAANoWarning.allocate(1, nullptr);

  std::vector<Foo> FVNoWarning;
  auto VectorEmptyRetval = FVNoWarning.empty();

  std::filesystem::path PNoWarning;
  auto PathEmptyRetval = PNoWarning.empty();

  std::pmr::memory_resource MRNoWarning;
  auto AllocRetval7 = MRNoWarning.allocate(1);

  std::pmr::polymorphic_allocator<Foo> PANoWarning;
  auto AllocRetval8 = PANoWarning.allocate(1);
}
