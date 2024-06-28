// RUN: %clang_cc1 -verify -std=c++2b -Wall -Wshadow %s

template <typename T>
struct Ptr
// xexpected-error {{cannot initialize a member subobject of type 'propconst int *' with an lvalue of type 'const int *const'}}
{
  propconst T* ptr;
};

void f()
{
  int i;
  Ptr<int> pi{&i};
  const Ptr<int> pi2 = pi;
  Ptr<int> pi3 = pi2; // xexpected-note {{in implicit copy constructor for 'Ptr<int>' first required here}}
}
