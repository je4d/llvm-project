// RUN: %clang_cc1 -std=c++26 -verify -Wall -Wshadow %s

template <typename T>
struct Ptr
{
  propconst T* ptr;
};

void f(int& i)
{
  Ptr<int> pi{&i};
  (void) pi;
  const Ptr<int> cpi{&i};
  (void) cpi;
}

void g(const Ptr<int>& cpi)
{
  Ptr<int> pi = cpi; // expected-error {{no viable conversion from 'const Ptr<int>' to 'propconst int *'}}
  (void) pi;
}

void h(const int& i)
{
  const Ptr<int> cpi{&i};
  (void) cpi;
  Ptr<int> pi{&i}; // expected-error {{cannot initialize a member subobject of type 'propconst int *' with an rvalue of type 'const int *'}}
  (void) pi;
}
