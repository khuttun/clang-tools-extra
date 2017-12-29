// RUN: %check_clang_tidy %s bugprone-unused-return-value %t \
// RUN: -config='{CheckOptions: \
// RUN:  [{key: bugprone-unused-return-value.FunctionsRegex, \
// RUN:    value: "^::fun$|^::ns::Template<.*>::Inner::memFun$|^::ns::Type::staticFun$"}]}' \
// RUN: --

namespace std {

template <typename T>
T *launder(T *);

} // namespace std

namespace ns {

template <typename T>
struct Template {
  struct Inner {
    bool memFun();
  };
};

using IntType = Template<int>;

struct Derived : public Template<bool>::Inner {};

struct Retval {
  int *P;
  Retval() { P = new int; }
  ~Retval() { delete P; }
};

struct Type {
  Retval memFun();
  static Retval staticFun();
};

} // namespace ns

int fun();

void warning() {
  fun();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  (fun());
  // CHECK-MESSAGES: [[@LINE-1]]:4: warning: unused return value [bugprone-unused-return-value]

  static_cast<short>(fun());
  // CHECK-MESSAGES: [[@LINE-1]]:22: warning: unused return value [bugprone-unused-return-value]

  ns::Template<bool>::Inner ObjA1;
  ObjA1.memFun();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  ns::IntType::Inner ObjA2;
  ObjA2.memFun();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  ns::Derived ObjA3;
  ObjA3.memFun();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]

  ns::Type::staticFun();
  // CHECK-MESSAGES: [[@LINE-1]]:3: warning: unused return value [bugprone-unused-return-value]
}

void noWarning() {
  auto R1 = fun();

  ns::Template<bool>::Inner ObjB1;
  auto R2 = ObjB1.memFun();

  auto R3 = ns::Type::staticFun();

  // test discarding return value of functions that are not configured to be checked
  int I = 1;
  std::launder(&I);

  ns::Type ObjB2;
  ObjB2.memFun();
}
