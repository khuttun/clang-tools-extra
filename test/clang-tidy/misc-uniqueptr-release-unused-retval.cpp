// RUN: %check_clang_tidy %s misc-uniqueptr-release-unused-retval %t

namespace std {
template <typename T>
struct unique_ptr {
  T *operator->();
  T *release();
};
} // namespace std

struct Foo {
  int release();
};

template <typename T>
void callRelease(T &t) { t.release(); }
// CHECK-MESSAGES: [[@LINE-1]]:26: warning: unused std::unique_ptr::release return value [misc-uniqueptr-release-unused-retval]

using FooPtr = std::unique_ptr<Foo>;

template <typename T>
struct Derived : public std::unique_ptr<T> {
};

void deleteThis(Foo *pointer) { delete pointer; }

void Warning() {
  std::unique_ptr<Foo> p1;
  p1.release();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused std::unique_ptr::release return value [misc-uniqueptr-release-unused-retval]
  { p1.release(); }
  // CHECK-MESSAGES: [[@LINE-1]]:5: warning: unused std::unique_ptr::release return value [misc-uniqueptr-release-unused-retval]
  callRelease(p1);
  FooPtr fp;
  fp.release();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused std::unique_ptr::release return value [misc-uniqueptr-release-unused-retval]
  Derived<Foo> dp;
  dp.release();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused std::unique_ptr::release return value [misc-uniqueptr-release-unused-retval]
  std::unique_ptr<std::unique_ptr<Foo>> pp;
  pp.release();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused std::unique_ptr::release return value [misc-uniqueptr-release-unused-retval]
  pp->release();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused std::unique_ptr::release return value [misc-uniqueptr-release-unused-retval]
}

void NoWarning() {
  std::unique_ptr<Foo> p2;
  auto q = p2.release();
  delete p2.release();
  deleteThis(p2.release());
  p2->release();
  p2.release()->release();
}
