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
bool empty(const T &);

template <typename ForwardIt, typename T>
ForwardIt remove(ForwardIt, ForwardIt, const T &);

template <typename ForwardIt, typename UnaryPredicate>
ForwardIt remove_if(ForwardIt, ForwardIt, UnaryPredicate);

template <typename ForwardIt>
ForwardIt unique(ForwardIt, ForwardIt);

// the check should be able to match std lib calls even if the functions are
// declared inside inline namespaces
inline namespace v1 {

template <typename T>
T *launder(T *);

} // namespace v1

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

template <typename T>
struct default_delete {};

template <typename T, typename Deleter = default_delete<T>>
struct unique_ptr {
  using pointer = T *;
  pointer release() noexcept;
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

void warning() {
  std::async(increment, 42);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::async(std::launch::async, increment, 42);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  Foo F;
  std::launder(&F);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::unique_ptr<Foo> UP;
  UP.release();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::unique_ptr<std::vector<int>> UP2;
  UP2.release();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::allocator<Foo> FA;
  FA.allocate(1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]
  FA.allocate(1, nullptr);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::allocator_traits<std::allocator<Foo>>::allocate(FA, 1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]
  std::allocator_traits<std::allocator<Foo>>::allocate(FA, 1, nullptr);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::scoped_allocator_adaptor<FooAlloc, FooAlloc> SAA;
  SAA.allocate(1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]
  SAA.allocate(1, nullptr);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::pmr::memory_resource MR;
  MR.allocate(1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::pmr::polymorphic_allocator<Foo> PA;
  PA.allocate(1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::vector<Foo> FV;
  FV.empty();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::filesystem::path P;
  P.empty();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::empty(FV);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::remove(nullptr, nullptr, 1);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::remove_if(nullptr, nullptr, nullptr);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  std::unique(nullptr, nullptr);
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: the value returned by this function should be used [bugprone-unused-return-value]

  // test discarding return values inside different kinds of statements

  auto Lambda = [&FV] { std::empty(FV); };
  // CHECK-MESSAGES: [[@LINE-1]]:25: warning: the value returned by this function should be used [bugprone-unused-return-value]

  if (true)
    std::empty(FV);
  // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]
  else if (true)
    std::empty(FV);
  // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]
  else
    std::empty(FV);
  // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]

  while (true)
    std::empty(FV);
  // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]

  do
    std::empty(FV);
  // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]
  while (true);

  for (;;)
    std::empty(FV);
  // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]

  for (std::empty(FV);;)
    // CHECK-MESSAGES: [[@LINE-1]]:8: warning: the value returned by this function should be used [bugprone-unused-return-value]
    ;

  for (;; std::empty(FV))
    // CHECK-MESSAGES: [[@LINE-1]]:11: warning: the value returned by this function should be used [bugprone-unused-return-value]
    ;

  for (auto C : "foo")
    std::empty(FV);
  // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]

  switch (1) {
  case 1:
    std::empty(FV);
    // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]
    break;
  default:
    std::empty(FV);
    // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]
    break;
  }

  try {
    std::empty(FV);
    // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]
  } catch (...) {
    std::empty(FV);
    // CHECK-MESSAGES: [[@LINE-1]]:5: warning: the value returned by this function should be used [bugprone-unused-return-value]
  }
}

void noWarning() {
  auto AsyncRetval1 = std::async(increment, 42);
  auto AsyncRetval2 = std::async(std::launch::async, increment, 42);

  Foo FNoWarning;
  auto LaunderRetval = std::launder(&FNoWarning);

  std::unique_ptr<Foo> UPNoWarning;
  auto ReleaseRetval = UPNoWarning.release();

  std::allocator<Foo> FANoWarning;
  auto AllocRetval1 = FANoWarning.allocate(1, nullptr);
  auto AllocRetval2 = FANoWarning.allocate(1);

  auto AllocRetval3 = std::allocator_traits<std::allocator<Foo>>::allocate(FANoWarning, 1);
  auto AllocRetval4 = std::allocator_traits<std::allocator<Foo>>::allocate(FANoWarning, 1, nullptr);

  std::scoped_allocator_adaptor<FooAlloc, FooAlloc> SAANoWarning;
  auto AllocRetval5 = SAANoWarning.allocate(1);
  auto AllocRetval6 = SAANoWarning.allocate(1, nullptr);

  std::pmr::memory_resource MRNoWarning;
  auto AllocRetval7 = MRNoWarning.allocate(1);

  std::pmr::polymorphic_allocator<Foo> PANoWarning;
  auto AllocRetval8 = PANoWarning.allocate(1);

  std::vector<Foo> FVNoWarning;
  auto VectorEmptyRetval = FVNoWarning.empty();

  std::filesystem::path PNoWarning;
  auto PathEmptyRetval = PNoWarning.empty();

  auto EmptyRetval = std::empty(FVNoWarning);

  auto RemoveRetval = std::remove(nullptr, nullptr, 1);

  auto RemoveIfRetval = std::remove_if(nullptr, nullptr, nullptr);

  auto UniqueRetval = std::unique(nullptr, nullptr);

  // test using the return value in different kinds of expressions
  useFuture(std::async(increment, 42));
  std::launder(&FNoWarning)->f();
  delete std::launder(&FNoWarning);

  if (std::empty(FVNoWarning))
    ;
  for (; std::empty(FVNoWarning);)
    ;
  while (std::empty(FVNoWarning))
    ;
  do
    ;
  while (std::empty(FVNoWarning));
  switch (std::empty(FVNoWarning))
    ;

  // cast to void should allow ignoring the return value
  (void)std::async(increment, 42);

  // test discarding return value of functions that are not configured to be checked
  increment(1);

  // test that the check is disabled inside GNU statement expressions
  ({ std::async(increment, 42); });
  auto StmtExprRetval = ({ std::async(increment, 42); });
}
