// RUN: %clang_cc1 -fsyntax-only -verify %std_cxx98- %s
// RUN: %clang_cc1 -fsyntax-only -verify %std_cxx14- %s
// RUN: %clang_cc1 -fsyntax-only -verify %std_cxx17- %s
// RUN: %clang_cc1 -fsyntax-only -verify %std_cxx23- %s
// RUN: %clang_cc1 -fsyntax-only -verify %std_cxx26- %s

struct A {};
enum B { Dummy };
namespace C {}
struct D : A {};
struct E : A {};
struct F : D, E {};
struct G : virtual D {};
class H : A {}; // expected-note 2{{implicitly declared private here}}

int A::*pdi1;
int (::A::*pdi2);
int (A::*pfi)(int);
void (*A::*ppfie)() throw();
#if __cplusplus < 201703L
// expected-error@-2 {{exception specifications are not allowed beyond a single level of indirection}}
#endif

int B::*pbi;
#if __cplusplus <= 199711L // C++03 or earlier modes
// expected-warning@-2 {{use of enumeration in a nested name specifier is a C++11 extension}}
#endif
// expected-error@-4 {{'pbi' does not point into a class}}
int C::*pci; // expected-error {{'pci' does not point into a class}}
void A::*pdv; // expected-error {{'pdv' declared as a member pointer to void}}
int& A::*pdr; // expected-error {{'pdr' declared as a member pointer to a reference}}

void f() {
  // This requires tentative parsing.
  int (A::*pf)(int, int);

  // Implicit conversion to bool.
  bool b = pdi1;
  b = pfi;

  // Conversion from null pointer constant.
  pf = 0;
  pf = __null;

  // Conversion to member of derived.
  int D::*pdid = pdi1;
  pdid = pdi2;

  // Fail conversion due to ambiguity and virtuality.
  int F::*pdif = pdi1; // expected-error {{ambiguous conversion from pointer to member of base class 'A' to pointer to member of derived class 'F':}}
  int G::*pdig = pdi1; // expected-error {{conversion from pointer to member of class 'A' to pointer to member of class 'G' via virtual base 'D' is not allowed}}

  // Conversion to member of base.
  pdi1 = pdid; // expected-error {{assigning to 'int A::*' from incompatible type 'int D::*'}}
  
  // Comparisons
  int (A::*pf2)(int, int);
  int (D::*pf3)(int, int) = 0;
  bool b1 = (pf == pf2); (void)b1;
  bool b2 = (pf != pf2); (void)b2;
  bool b3 = (pf == pf3); (void)b3;
  bool b4 = (pf != 0); (void)b4;
}

struct TheBase
{
  void d();
};

struct HasMembers : TheBase
{
  int i;
  void f();

  void g();
  void g(int);
  static void g(double);
};

namespace Fake
{
  int i;
  void f();
}

void g() {
  HasMembers hm;

  int HasMembers::*pmi = &HasMembers::i;
  int *pni = &Fake::i;
  int *pmii = &hm.i;

  void (HasMembers::*pmf)() = &HasMembers::f;
  void (*pnf)() = &Fake::f;
  &hm.f; // expected-error {{cannot create a non-constant pointer to member function}}

  void (HasMembers::*pmgv)() = &HasMembers::g;
  void (HasMembers::*pmgi)(int) = &HasMembers::g;
  void (*pmgd)(double) = &HasMembers::g;

  void (HasMembers::*pmd)() = &HasMembers::d;
}

struct Incomplete;

void h() {
  HasMembers hm, *phm = &hm;

  int HasMembers::*pi = &HasMembers::i;
  hm.*pi = 0;
  int i = phm->*pi;
  (void)&(hm.*pi);
  (void)&(phm->*pi);
  (void)&((&hm)->*pi); 

  void (HasMembers::*pf)() = &HasMembers::f;
  (hm.*pf)();
  (phm->*pf)();

  (void)(hm->*pi); // expected-error {{left hand operand to ->* must be a pointer to class compatible with the right hand operand, but is 'HasMembers'}}
  (void)(phm.*pi); // expected-error {{left hand operand to .* must be a class compatible with the right hand operand, but is 'HasMembers *'}}
  (void)(i.*pi); // expected-error {{left hand operand to .* must be a class compatible with the right hand operand, but is 'int'}}
  int *ptr;
  (void)(ptr->*pi); // expected-error {{left hand operand to ->* must be a pointer to class compatible with the right hand operand, but is 'int *'}}

  int A::*pai = 0;
  D d, *pd = &d;
  (void)(d.*pai);
  (void)(pd->*pai);
  F f, *ptrf = &f;
  (void)(f.*pai); // expected-error {{ambiguous conversion from derived class 'F' to base class 'A'}}
  (void)(ptrf->*pai); // expected-error {{ambiguous conversion from derived class 'F' to base class 'A'}}
  H h, *ptrh = &h;
  (void)(h.*pai); // expected-error {{cannot cast 'H' to its private base class 'A'}}
  (void)(ptrh->*pai); // expected-error {{cannot cast 'H' to its private base class 'A'}}

  (void)(hm.*i); // expected-error {{pointer-to-member}}
  (void)(phm->*i); // expected-error {{pointer-to-member}}

  // Okay
  Incomplete *inc;
  int Incomplete::*pii = 0;
  (void)(inc->*pii);
}

struct OverloadsPtrMem
{
  int operator ->*(const char *);
};

void i() {
  OverloadsPtrMem m;
  int foo = m->*"Awesome!";
}

namespace pr5985 {
  struct c {
    void h();
    void f() {
      void (c::*p)();
      p = &h; // expected-error {{must explicitly qualify}}
      p = &this->h; // expected-error {{cannot create a non-constant pointer to member function}}
      p = &(*this).h; // expected-error {{cannot create a non-constant pointer to member function}}
    }
  };
}

namespace pr6783 {
  struct Base {};
  struct X; // expected-note {{forward declaration}}

  int test1(int Base::* p2m, X* object)
  {
    return object->*p2m; // expected-error {{left hand operand to ->*}}
  }
}

namespace PR7176 {
  namespace base
  {
    struct Process
    { };
    struct Continuous : Process
    {
      bool cond();
    };
  }

  typedef bool( base::Process::*Condition )();

  void m()
  { (void)(Condition) &base::Continuous::cond; }
}

namespace rdar8358512 {
  // We can't call this with an overload set because we're not allowed
  // to look into overload sets unless the parameter has some kind of
  // function type.
  template <class F> void bind(F f); // expected-note 12 {{candidate template ignored}}
  template <class F, class T> void bindmem(F (T::*f)()); // expected-note 4 {{candidate template ignored}}
  template <class F> void bindfn(F (*f)()); // expected-note 4 {{candidate template ignored}}

  struct A {
    void nonstat();
    void nonstat(int);

    void mixed();
    static void mixed(int);

    static void stat();
    static void stat(int);
    
    template <typename T> struct Test0 {
      void test() {
        bind(&nonstat); // expected-error {{no matching function for call}}
        bind(&A::nonstat); // expected-error {{no matching function for call}}

        bind(&mixed); // expected-error {{no matching function for call}}
        bind(&A::mixed); // expected-error {{no matching function for call}}

        bind(&stat); // expected-error {{no matching function for call}}
        bind(&A::stat); // expected-error {{no matching function for call}}
      }
    };

    template <typename T> struct Test1 {
      void test() {
        bindmem(&nonstat); // expected-error {{no matching function for call}}
        bindmem(&A::nonstat);

        bindmem(&mixed); // expected-error {{no matching function for call}}
        bindmem(&A::mixed);

        bindmem(&stat); // expected-error {{no matching function for call}}
        bindmem(&A::stat); // expected-error {{no matching function for call}}
      }
    };

    template <typename T> struct Test2 {
      void test() {
        bindfn(&nonstat); // expected-error {{no matching function for call}}
        bindfn(&A::nonstat); // expected-error {{no matching function for call}}

        bindfn(&mixed); // expected-error {{no matching function for call}}
        bindfn(&A::mixed); // expected-error {{no matching function for call}}

        bindfn(&stat);
        bindfn(&A::stat);
      }
    };
  };

  template <class T> class B {
    void nonstat();
    void nonstat(int);

    void mixed();
    static void mixed(int);

    static void stat();
    static void stat(int);

    // None of these can be diagnosed yet, because the arguments are
    // still dependent.
    void test0a() {
      bind(&nonstat);
      bind(&B::nonstat);

      bind(&mixed);
      bind(&B::mixed);

      bind(&stat);
      bind(&B::stat);
    }

    void test0b() {
      bind(&nonstat); // expected-error {{no matching function for call}}
      bind(&B::nonstat); // expected-error {{no matching function for call}}

      bind(&mixed); // expected-error {{no matching function for call}}
      bind(&B::mixed); // expected-error {{no matching function for call}}

      bind(&stat); // expected-error {{no matching function for call}}
      bind(&B::stat); // expected-error {{no matching function for call}}
    }
  };

  template void B<int>::test0b(); // expected-note {{in instantiation}}
}

namespace PR9973 {
  template<class R, class T> struct dm
  {
    typedef R T::*F;
    F f_;
    template<class U> int & call(U u)
    { return u->*f_; } // expected-error{{reference to non-static member function must be called; did you mean to call it with no arguments?}} expected-error {{non-const lvalue reference to type 'int' cannot bind to a temporary of type 'int'}}

    template<class U> int operator()(U u)
    { call(u); } // expected-note{{in instantiation of}}
  };

  template<class R, class T> 
  dm<R, T> mem_fn(R T::*) ;

  struct test
  { int nullary_v(); };

  void f()
  {
    test* t;
    mem_fn(&test::nullary_v)(t); // expected-note{{in instantiation of}}
  }
}

namespace test8 {
  struct A { int foo; };
  int test1() {
    // Verify that we perform (and check) an lvalue conversion on the operands here.
    return (*((A**) 0)) // expected-warning {{indirection of non-volatile null pointer will be deleted}} expected-note {{consider}}
             ->**(int A::**) 0; // expected-warning {{indirection of non-volatile null pointer will be deleted}} expected-note {{consider}}
  }

  int test2() {
    // Verify that we perform (and check) an lvalue conversion on the operands here.
    // TODO: the .* should itself warn about being a dereference of null.
    return (*((A*) 0))
             .**(int A::**) 0; // expected-warning {{indirection of non-volatile null pointer will be deleted}} expected-note {{consider}}
  }
}

namespace conv {
  struct PB {};                                           // Base
  struct PAB {};                                          // Ambiguous base
  struct PAB1 : PAB {};
  struct PAB2 : PAB {};
  struct PVB {};                                          // Virtual base
  struct PAVB {};                                         // Ambiguous base 2
  struct PAVB2 : PAVB {};
  struct PPB {};                                          // Private base
  struct P : PB, PAB1, PAB2, virtual PVB, PAVB, virtual PAVB2, private PPB {};
  // expected-warning@-1 {{direct base 'PAVB' is inaccessible due to ambiguity:\n    struct conv::P -> PAVB\n    struct conv::P -> PAVB2 -> PAVB}}
#if __cplusplus >= 202400L
  // expected-note@-3 7{{declared private here}}
#endif

  struct CB {};                                           // Base
  struct CVB {};                                          // Ambiguous base
  struct CAB {};
  struct CAB1 : CAB {};
  struct CAB2 : CAB {};                                   // Virtual base
  struct CAVB {};                                         // Ambiguous base 2
  struct CAVB2 : CAVB {};
  struct CPB {};                                          // Private base
  struct C : CB, CAB1, CAB2, virtual CVB, CAVB, virtual CAVB2, private CPB {};
  // expected-warning@-1 {{direct base 'CAVB' is inaccessible due to ambiguity:\n    struct conv::C -> CAVB\n    struct conv::C -> CAVB2 -> CAVB}}
#if __cplusplus >= 202400L
  // expected-note@-3 2{{declared private here}}
#else
  // expected-note@-5 {{declared private here}}
#endif
  struct CD : C {};                                       // Derived
  struct CAD1 : C {};
  struct CAD2 : C {};
  struct CAD : CAD1, CAD2 {};                             // Ambiguous derived
  struct CVD : virtual C {};                              // Virtual derived
  struct CAVD2 : C {};
  struct CAVD : C, virtual CAVD2 {};                      // Virt+Ambiguous base
  // expected-warning@-1 {{direct base 'C' is inaccessible due to ambiguity:\n    struct conv::CAVD -> C\n    struct conv::CAVD -> CAVD -> CAVD2}}
  struct CPD : private C {};                              // Private derived
#if __cplusplus >= 202400L
  // expected-note@-2 6{{declared private here}}
#else
  // expected-note@-4 3{{declared private here}}
#endif

  static P C::* c_to_p = 0;
  static P const C::* c_to_cp = 0;

  void f() {
    {
      // No pointee conversion, no class conversion, initialization syntax
      P C::*plain(c_to_p);
      P const C::*cvq_add(c_to_p);
      P C::*cvq_rem(c_to_cp); // expected-error{{cannot initialize a variable of type 'P conv::C::*' with an lvalue of type 'const P conv::C::*'}}
    }
    {
      // No pointee conversion, no class conversion, assignment syntax
      P C::*plain; plain = c_to_p;
      P const C::*cvq_add; cvq_add = c_to_p;
      P C::*cvq_rem; cvq_rem = c_to_cp; // expected-error{{assigning to 'P conv::C::*' from incompatible type 'const P conv::C::*'}}
    }
    {
      // No pointee conversion, no class conversion, static_cast
      (void) static_cast<P C::*>(c_to_p);
      (void) static_cast<P const C::*>(c_to_p);
      (void) static_cast<P C::*>(c_to_cp); // expected-error{{static_cast from 'const P conv::C::*' to 'P conv::C::*' casts away qualifiers}}
    }
    {
      // No pointee conversion, no class conversion, function-style cast
      typedef P C::*CToP; (void) CToP(c_to_p);
      typedef P const C::*CToConstP; (void) CToConstP(c_to_p);
      typedef P C::*CToP; (void) CToP(c_to_cp);
    }
    {
      // No pointee conversion, no class conversion, C-style cast
      (void) (P C::*) c_to_p;
      (void) (P const C::*) c_to_p;
      (void) (P C::*) c_to_cp;
    }
    {
      // No pointee conversion, class downcast, initialization syntax
      P CD::*plain(c_to_p);
      P CAD::*ambig_class(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAD'}}
      P CVD::*virt_class(c_to_p); // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVD' via virtual base 'conv::C' is not allowed}}
      P CAVD::*virtambig_class(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAVD'}}
      P CPD::*priv_class(c_to_p); // expected-error{{cannot cast private base class 'conv::C' to 'conv::CPD'}}
      P const CD::*cvq_add(c_to_p);
      P CD::*cvq_rem(c_to_cp); // expected-error{{cannot initialize a variable of type 'P conv::CD::*' with an lvalue of type 'const P conv::C::*'}}
    }
    {
      // No pointee conversion, class downcast, assignment syntax
      P CD::*plain; plain = c_to_p;
      P CAD::*ambig_class; ambig_class = c_to_p; // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAD'}}
      P CVD::*virt_class; virt_class = c_to_p; // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVD' via virtual base 'conv::C' is not allowed}}
      P CAVD::*virtambig_class; virtambig_class = c_to_p; // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAVD'}}
      P CPD::*priv_class; priv_class = c_to_p; // expected-error{{cannot cast private base class 'conv::C' to 'conv::CPD'}}
      P const CD::*cvq_add; cvq_add = c_to_p;
      P CD::*cvq_rem; cvq_rem = c_to_cp; // expected-error{{assigning to 'P conv::CD::*' from incompatible type 'const P conv::C::*'}}
    }
    {
      // No pointee conversion, class downcast, static_cast
      (void) static_cast<P CD::*>(c_to_p);
      (void) static_cast<P CAD::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAD'}}
      (void) static_cast<P CVD::*>(c_to_p); // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVD' via virtual base 'conv::C' is not allowed}}
      (void) static_cast<P CAVD::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAVD'}}
      (void) static_cast<P CPD::*>(c_to_p); // expected-error{{cannot cast private base class 'conv::C' to 'conv::CPD'}}
      (void) static_cast<P const CD::*>(c_to_p);
      (void) static_cast<P CD::*>(c_to_cp); // expected-error{{static_cast from 'const P conv::C::*' to 'P conv::CD::*' casts away qualifiers}}
    }
    {
      // No pointee conversion, class downcast, function-style cast
      typedef P CD::*CdToP; (void) CdToP(c_to_p);
      typedef P CAD::*CadToP; (void) CadToP(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAD'}}
      typedef P CVD::*CvdToP; (void) CvdToP(c_to_p); // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVD' via virtual base 'conv::C' is not allowed}}
      typedef P CAVD::*CavdToP; (void) CavdToP(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAVD'}}
      typedef P CPD::*CpdToP; (void) CpdToP(c_to_p);
      typedef P const CD::*CdToConstP; (void) CdToConstP(c_to_p);
      typedef P CD::*CdToP; (void) CdToP(c_to_cp);
    }
    {
      // No pointee conversion, class downcast, C-style cast
      (void) (P CD::*) c_to_p;
      (void) (P CAD::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAD'}}
      (void) (P CVD::*) c_to_p; // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVD' via virtual base 'conv::C' is not allowed}}
      (void) (P CAVD::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAVD'}}
      (void) (P CPD::*) c_to_p;
      (void) (P const CD::*) c_to_p;
      (void) (P CD::*) c_to_cp;
    }
    {
      // No pointee conversion, class upcast, initialization syntax
      P CB::*plain(c_to_p); // expected-error{{cannot initialize a variable of type 'P conv::CB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      P CAB::*ambig_class(c_to_p); // expected-error{{cannot initialize a variable of type 'P conv::CAB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CAB' vs 'conv::C')}}
      P CVB::*virt_class(c_to_p); // expected-error{{cannot initialize a variable of type 'P conv::CVB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CVB' vs 'conv::C')}}
      P CAVB::*virtambig_class(c_to_p); // expected-error{{cannot initialize a variable of type 'P conv::CAVB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CAVB' vs 'conv::C')}}
      P CPB::*priv_class(c_to_p); // expected-error{{cannot initialize a variable of type 'P conv::CPB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CPB' vs 'conv::C')}}
      P const CB::*cvq_add(c_to_p); // expected-error{{cannot initialize a variable of type 'const P conv::CB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      P CB::*cvq_rem(c_to_cp); // expected-error{{cannot initialize a variable of type 'P conv::CB::*' with an lvalue of type 'const P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
    }
    {
      // No pointee conversion, class upcast, assignment syntax
      P CB::*plain; plain = c_to_p; // expected-error{{assigning to 'P conv::CB::*' from incompatible type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      P CAB::*ambig_class; ambig_class = c_to_p; // expected-error{{assigning to 'P conv::CAB::*' from incompatible type 'P conv::C::*': different classes ('conv::CAB' vs 'conv::C')}}
      P CVB::*virt_class; virt_class = c_to_p; // expected-error{{assigning to 'P conv::CVB::*' from incompatible type 'P conv::C::*': different classes ('conv::CVB' vs 'conv::C')}}
      P CAVB::*virtambig_class; virtambig_class = c_to_p; // expected-error{{assigning to 'P conv::CAVB::*' from incompatible type 'P conv::C::*': different classes ('conv::CAVB' vs 'conv::C')}}
      P CPB::*priv_class; priv_class = c_to_p; // expected-error{{assigning to 'P conv::CPB::*' from incompatible type 'P conv::C::*': different classes ('conv::CPB' vs 'conv::C')}}
      P const CB::*cvq_add; cvq_add = c_to_p; // expected-error{{assigning to 'const P conv::CB::*' from incompatible type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      P CB::*cvq_rem; cvq_rem = c_to_cp; // expected-error{{assigning to 'P conv::CB::*' from incompatible type 'const P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
    }
    {
      // No pointee conversion, class upcast, static_cast
      (void) static_cast<P CB::*>(c_to_p);
      (void) static_cast<P CAB::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAB'}}
      (void) static_cast<P CVB::*>(c_to_p); // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVB' via virtual base 'conv::CVB' is not allowed}}
      (void) static_cast<P CAVB::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAVB'}}
      (void) static_cast<P CPB::*>(c_to_p); // expected-error{{cannot cast 'conv::C' to its private base class 'conv::CPB'}}
      (void) static_cast<P const CB::*>(c_to_p);
      (void) static_cast<P CB::*>(c_to_cp); // expected-error{{static_cast from 'const P conv::C::*' to 'P conv::CB::*' casts away qualifiers}}
    }
    {
      // No pointee conversion, class upcast, function-style cast
      typedef P CB::*CbToP; (void) CbToP(c_to_p);
      typedef P CAB::*CabToP; (void) CabToP(c_to_p); // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAB'}}
      typedef P CVB::*CvbToP; (void) CvbToP(c_to_p); // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVB' via virtual base 'conv::CVB' is not allowed}}
      typedef P CAVB::*CavbToP; (void) CavbToP(c_to_p); // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAVB'}}
      typedef P CPB::*CpbToP; (void) CpbToP(c_to_p);
      typedef P const CB::*CbToConstP; (void) CbToConstP(c_to_p);
      typedef P CB::*CbToP; (void) CbToP(c_to_cp);
    }
    {
      // No pointee conversion, class upcast, C-style cast
      (void) (P CB::*) c_to_p;
      (void) (P CAB::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAB'}}
      (void) (P CVB::*) c_to_p; // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVB' via virtual base 'conv::CVB' is not allowed}}
      (void) (P CAVB::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAVB'}}
      (void) (P CPB::*) c_to_p;
      (void) (P const CB::*) c_to_p;
      (void) (P CB::*) c_to_cp;
    }
    {
      // Pointee upcast, no class conversion, initialization syntax
#if __cplusplus >= 202400L
      PB C::*plain(c_to_p);
      PAB C::*ambig_pointee(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAB' (base class)}}
      PVB C::*virt_pointee(c_to_p); // expected-error{{conversion from pointer to member of type 'P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      PAVB C::*virtambig_pointee(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      PPB C::*priv_pointee(c_to_p); // expected-error{{cannot cast 'P' to its private base class 'PPB'}}
      PB const C::*cvq_add(c_to_p);
      PB C::*cvq_rem(c_to_cp); // expected-error{{cannot initialize a variable of type 'PB conv::C::*' with an lvalue of type 'const P conv::C::*'}}
#else
      PB C::*plain(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::C::*' with an lvalue of type 'P conv::C::*'}}
      PAB C::*ambig_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PAB conv::C::*' with an lvalue of type 'P conv::C::*'}}
      PVB C::*virt_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PVB conv::C::*' with an lvalue of type 'P conv::C::*'}}
      PAVB C::*virtambig_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PAVB conv::C::*' with an lvalue of type 'P conv::C::*'}}
      PPB C::*priv_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PPB conv::C::*' with an lvalue of type 'P conv::C::*'}}
      PB const C::*cvq_add(c_to_p); // expected-error{{cannot initialize a variable of type 'const PB conv::C::*' with an lvalue of type 'P conv::C::*'}}
      PB C::*cvq_rem(c_to_cp); // expected-error{{cannot initialize a variable of type 'PB conv::C::*' with an lvalue of type 'const P conv::C::*'}}
#endif
    }
    {
      // Pointee upcast, no class conversion, assignment syntax
#if __cplusplus >= 202400L
      PB C::*plain; plain = c_to_p;
      PAB C::*ambig_pointee; ambig_pointee = c_to_p; // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAB' (base class)}}
      PVB C::*virt_pointee; virt_pointee = c_to_p; // expected-error{{conversion from pointer to member of type 'P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      PAVB C::*virtambig_pointee; virtambig_pointee = c_to_p; // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      PPB C::*priv_pointee; priv_pointee = c_to_p; // expected-error{{cannot cast 'P' to its private base class 'PPB'}}
      PB const C::*cvq_add; cvq_add = c_to_p;
      PB C::*cvq_rem; cvq_rem = c_to_cp; // expected-error{{assigning to 'PB conv::C::*' from incompatible type 'const P conv::C::*'}}
#else
      PB C::*plain; plain = c_to_p; // expected-error{{assigning to 'PB conv::C::*' from incompatible type 'P conv::C::*'}}
      PAB C::*ambig_pointee; ambig_pointee = c_to_p; // expected-error{{assigning to 'PAB conv::C::*' from incompatible type 'P conv::C::*'}}
      PVB C::*virt_pointee; virt_pointee = c_to_p; // expected-error{{assigning to 'PVB conv::C::*' from incompatible type 'P conv::C::*'}}
      PAVB C::*virtambig_pointee; virtambig_pointee = c_to_p; // expected-error{{assigning to 'PAVB conv::C::*' from incompatible type 'P conv::C::*'}}
      PPB C::*priv_pointee; priv_pointee = c_to_p; // expected-error{{assigning to 'PPB conv::C::*' from incompatible type 'P conv::C::*'}}
      PB const C::*cvq_add; cvq_add = c_to_p; // expected-error{{assigning to 'const PB conv::C::*' from incompatible type 'P conv::C::*'}}
      PB C::*cvq_rem; cvq_rem = c_to_cp; // expected-error{{assigning to 'PB conv::C::*' from incompatible type 'const P conv::C::*'}}
#endif
    }
    {
      // Pointee upcast, no class conversion, static_cast
#if __cplusplus >= 202400L
      (void) static_cast<PB C::*>(c_to_p);
      (void) static_cast<PAB C::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAB' (base class)}}
      (void) static_cast<PVB C::*>(c_to_p); // expected-error{{conversion from pointer to member of type 'P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      (void) static_cast<PAVB C::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      (void) static_cast<PPB C::*>(c_to_p); // expected-error{{cannot cast 'P' to its private base class 'PPB'}}
      (void) static_cast<PB const C::*>(c_to_p);
      (void) static_cast<PB C::*>(c_to_cp); // expected-error{{static_cast from 'const P conv::C::*' to 'PB conv::C::*' casts away qualifiers}}
#else
      (void) static_cast<PB C::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::C::*' is not allowed}}
      (void) static_cast<PAB C::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PAB conv::C::*' is not allowed}}
      (void) static_cast<PVB C::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PVB conv::C::*' is not allowed}}
      (void) static_cast<PAVB C::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PAVB conv::C::*' is not allowed}}
      (void) static_cast<PPB C::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PPB conv::C::*' is not allowed}}
      (void) static_cast<PB const C::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'const PB conv::C::*' is not allowed}}
      (void) static_cast<PB C::*>(c_to_cp); // expected-error{{static_cast from 'const P conv::C::*' to 'PB conv::C::*' is not allowed}}
#endif
    }
    {
      // Pointee upcast, no class conversion, function-style cast
#if __cplusplus >= 202400L
      typedef PB C::*CToPb; (void) CToPb(c_to_p);
      typedef PAB C::*CToPab; (void) CToPab(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAB' (base class)}}
      typedef PVB C::*CToPvb; (void) CToPvb(c_to_p); // expected-error{{conversion from pointer to member of type 'P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      typedef PAVB C::*CToPavb; (void) CToPavb(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      typedef PPB C::*CToPpb; (void) CToPpb(c_to_p);
      typedef PB const C::*CToConstPb; (void) CToConstPb(c_to_p);
      typedef PB C::*CToPb; (void) CToPb(c_to_cp);
#else
      typedef PB C::*CToPb; (void) CToPb(c_to_p);
      typedef PAB C::*CToPab; (void) CToPab(c_to_p);
      typedef PVB C::*CToPvb; (void) CToPvb(c_to_p);
      typedef PAVB C::*CToPavb; (void) CToPavb(c_to_p);
      typedef PPB C::*CToPpb; (void) CToPpb(c_to_p);
      typedef PB const C::*CToConstPb; (void) CToConstPb(c_to_p);
      typedef PB C::*CToPb; (void) CToPb(c_to_cp);
#endif
    }
    {
      // Pointee upcast, no class conversion, C-style cast
#if __cplusplus >= 202400L
      (void) (PB C::*) c_to_p;
      (void) (PAB C::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAB' (base class)}}
      (void) (PVB C::*) c_to_p; // expected-error{{conversion from pointer to member of type 'P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      (void) (PAVB C::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      (void) (PPB C::*) c_to_p;
      (void) (PB const C::*) c_to_p;
      (void) (PB C::*) c_to_cp;
#else
      (void) (PB C::*) c_to_p;
      (void) (PAB C::*) c_to_p;
      (void) (PVB C::*) c_to_p;
      (void) (PAVB C::*) c_to_p;
      (void) (PPB C::*) c_to_p;
      (void) (PB const C::*) c_to_p;
      (void) (PB C::*) c_to_cp;
#endif
    }
    {
      // Pointee upcast, class downcast, initialization syntax
#if __cplusplus >= 202400L
      PB CD::*plain(c_to_p);
      PAB CD::*ambig_pointee(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAB' (base class)}}
      PVB CD::*virt_pointee(c_to_p); // expected-error{{conversion from pointer to member of type 'P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      PAVB CD::*virtambig_pointee(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      PPB CD::*priv_pointee(c_to_p); // expected-error{{cannot cast 'P' to its private base class 'PPB'}}
      PB CAD::*ambig_class(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAD'}}
      PB CVD::*virt_class(c_to_p); // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVD' via virtual base 'conv::C' is not allowed}}
      PB CAVD::*virtambig_class(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAVD'}}
      PB CPD::*priv_class(c_to_p); // expected-error{{cannot cast private base class 'conv::C' to 'conv::CPD'}}
      PB const CD::*cvq_add(c_to_p);
      PB CD::*cvq_rem(c_to_cp); // expected-error{{cannot initialize a variable of type 'PB conv::CD::*' with an lvalue of type 'const P conv::C::*'}}
#else
      PB CD::*plain(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CD::*' with an lvalue of type 'P conv::C::*'}}
      PAB CD::*ambig_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PAB conv::CD::*' with an lvalue of type 'P conv::C::*'}}
      PVB CD::*virt_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PVB conv::CD::*' with an lvalue of type 'P conv::C::*'}}
      PAVB CD::*virtambig_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PAVB conv::CD::*' with an lvalue of type 'P conv::C::*'}}
      PPB CD::*priv_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PPB conv::CD::*' with an lvalue of type 'P conv::C::*'}}
      PB CAD::*ambig_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CAD::*' with an lvalue of type 'P conv::C::*'}}
      PB CVD::*virt_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CVD::*' with an lvalue of type 'P conv::C::*'}}
      PB CAVD::*virtambig_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CAVD::*' with an lvalue of type 'P conv::C::*'}}
      PB CPD::*priv_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CPD::*' with an lvalue of type 'P conv::C::*'}}
      PB const CD::*cvq_add(c_to_p); // expected-error{{cannot initialize a variable of type 'const PB conv::CD::*' with an lvalue of type 'P conv::C::*'}}
      PB CD::*cvq_rem(c_to_cp); // expected-error{{cannot initialize a variable of type 'PB conv::CD::*' with an lvalue of type 'const P conv::C::*'}}
#endif
    }
    {
      // Pointee upcast, class downcast, assignment syntax
#if __cplusplus >= 202400L
      PB CD::*plain; plain = c_to_p;
      PAB CD::*ambig_pointee; ambig_pointee = c_to_p; // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAB' (base class)}}
      PVB CD::*virt_pointee; virt_pointee = c_to_p; // expected-error{{conversion from pointer to member of type 'P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      PAVB CD::*virtambig_pointee; virtambig_pointee = c_to_p; // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      PPB CD::*priv_pointee; priv_pointee = c_to_p; // expected-error{{cannot cast 'P' to its private base class 'PPB'}}
      PB CAD::*ambig_class; ambig_class = c_to_p; // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAD'}}
      PB CVD::*virt_class; virt_class = c_to_p; // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVD' via virtual base 'conv::C' is not allowed}}
      PB CAVD::*virtambig_class; virtambig_class = c_to_p; // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAVD'}}
      PB CPD::*priv_class; priv_class = c_to_p; // expected-error{{cannot cast private base class 'conv::C' to 'conv::CPD'}}
      PB const CD::*cvq_add; cvq_add = c_to_p;
      PB CD::*cvq_rem; cvq_rem = c_to_cp; // expected-error{{assigning to 'PB conv::CD::*' from incompatible type 'const P conv::C::*'}}
#else
      PB CD::*plain; plain = c_to_p; // expected-error{{assigning to 'PB conv::CD::*' from incompatible type 'P conv::C::*'}}
      PAB CD::*ambig_pointee; ambig_pointee = c_to_p; // expected-error{{assigning to 'PAB conv::CD::*' from incompatible type 'P conv::C::*'}}
      PVB CD::*virt_pointee; virt_pointee = c_to_p; // expected-error{{assigning to 'PVB conv::CD::*' from incompatible type 'P conv::C::*'}}
      PAVB CD::*virtambig_pointee; virtambig_pointee = c_to_p; // expected-error{{assigning to 'PAVB conv::CD::*' from incompatible type 'P conv::C::*'}}
      PPB CD::*priv_pointee; priv_pointee = c_to_p; // expected-error{{assigning to 'PPB conv::CD::*' from incompatible type 'P conv::C::*'}}
      PB CAD::*ambig_class; ambig_class = c_to_p; // expected-error{{assigning to 'PB conv::CAD::*' from incompatible type 'P conv::C::*'}}
      PB CVD::*virt_class; virt_class = c_to_p; // expected-error{{assigning to 'PB conv::CVD::*' from incompatible type 'P conv::C::*'}}
      PB CAVD::*virtambig_class; virtambig_class = c_to_p; // expected-error{{assigning to 'PB conv::CAVD::*' from incompatible type 'P conv::C::*'}}
      PB CPD::*priv_class; priv_class = c_to_p; // expected-error{{assigning to 'PB conv::CPD::*' from incompatible type 'P conv::C::*'}}
      PB const CD::*cvq_add; cvq_add = c_to_p; // expected-error{{assigning to 'const PB conv::CD::*' from incompatible type 'P conv::C::*'}}
      PB CD::*cvq_rem; cvq_rem = c_to_cp; // expected-error{{assigning to 'PB conv::CD::*' from incompatible type 'const P conv::C::*'}}
#endif
    }
    {
      // Pointee upcast, class downcast, static_cast
#if __cplusplus >= 202400L
      (void) static_cast<PB CD::*>(c_to_p);
      (void) static_cast<PAB CD::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAB' (base class)}}
      (void) static_cast<PVB CD::*>(c_to_p); // expected-error{{conversion from pointer to member of type 'P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      (void) static_cast<PAVB CD::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      (void) static_cast<PPB CD::*>(c_to_p); // expected-error{{cannot cast 'P' to its private base class 'PPB'}}
      (void) static_cast<PB CAD::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAD'}}
      (void) static_cast<PB CVD::*>(c_to_p); // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVD' via virtual base 'conv::C' is not allowed}}
      (void) static_cast<PB CAVD::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAVD'}}
      (void) static_cast<PB CPD::*>(c_to_p); // expected-error{{cannot cast private base class 'conv::C' to 'conv::CPD'}}
      (void) static_cast<PB const CD::*>(c_to_p);
      (void) static_cast<PB CD::*>(c_to_cp); // expected-error{{static_cast from 'const P conv::C::*' to 'PB conv::CD::*' casts away qualifiers}}
#else
      (void) static_cast<PB CD::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::CD::*' is not allowed}}
      (void) static_cast<PAB CD::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PAB conv::CD::*' is not allowed}}
      (void) static_cast<PVB CD::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PVB conv::CD::*' is not allowed}}
      (void) static_cast<PAVB CD::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PAVB conv::CD::*' is not allowed}}
      (void) static_cast<PPB CD::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PPB conv::CD::*' is not allowed}}
      (void) static_cast<PB CAD::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::CAD::*' is not allowed}}
      (void) static_cast<PB CVD::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::CVD::*' is not allowed}}
      (void) static_cast<PB CAVD::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::CAVD::*' is not allowed}}
      (void) static_cast<PB CPD::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::CPD::*' is not allowed}}
      (void) static_cast<PB const CD::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'const PB conv::CD::*' is not allowed}}
      (void) static_cast<PB CD::*>(c_to_cp); // expected-error{{static_cast from 'const P conv::C::*' to 'PB conv::CD::*' is not allowed}}
#endif
    }
    {
      // Pointee upcast, class downcast, function-style cast
#if __cplusplus >= 202400L
      typedef PB CD::*CdToPb; (void) CdToPb(c_to_p);
      typedef PAB CD::*CdToPab; (void) CdToPab(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAB' (base class)}}
      typedef PVB CD::*CdToPvb; (void) CdToPvb(c_to_p); // expected-error{{conversion from pointer to member of type 'P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      typedef PAVB CD::*CdToPavb; (void) CdToPavb(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      typedef PPB CD::*CdToPpb; (void) CdToPpb(c_to_p);
      typedef PB CAD::*CadToPb; (void) CadToPb(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAD'}}
      typedef PB CVD::*CvdToPb; (void) CvdToPb(c_to_p); // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVD' via virtual base 'conv::C' is not allowed}}
      typedef PB CAVD::*CavdToPb; (void) CavdToPb(c_to_p); // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAVD'}}
      typedef PB CPD::*CpdToPb; (void) CpdToPb(c_to_p);
      typedef PB const CD::*CdToConstPb; (void) CdToConstPb(c_to_p);
      typedef PB CD::*CdToPb; (void) CdToPb(c_to_cp);
#else
      typedef PB CD::*CdToPb; (void) CdToPb(c_to_p);
      typedef PAB CD::*CdToPab; (void) CdToPab(c_to_p);
      typedef PVB CD::*CdToPvb; (void) CdToPvb(c_to_p);
      typedef PAVB CD::*CdToPavb; (void) CdToPavb(c_to_p);
      typedef PPB CD::*CdToPpb; (void) CdToPpb(c_to_p);
      typedef PB CAD::*CadToPb; (void) CadToPb(c_to_p);
      typedef PB CVD::*CvdToPb; (void) CvdToPb(c_to_p);
      typedef PB CAVD::*CavdToPb; (void) CavdToPb(c_to_p);
      typedef PB CPD::*CpdToPb; (void) CpdToPb(c_to_p);
      typedef PB const CD::*CdToConstPb; (void) CdToConstPb(c_to_p);
      typedef PB CD::*CdToPb; (void) CdToPb(c_to_cp);
#endif
    }
    {
      // Pointee upcast, class downcast, C-style cast
#if __cplusplus >= 202400L
      (void) (PB CD::*) c_to_p;
      (void) (PAB CD::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAB' (base class)}}
      (void) (PVB CD::*) c_to_p; // expected-error{{conversion from pointer to member of type 'P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      (void) (PAVB CD::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of type 'P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      (void) (PPB CD::*) c_to_p;
      (void) (PB CAD::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAD'}}
      (void) (PB CVD::*) c_to_p; // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVD' via virtual base 'conv::C' is not allowed}}
      (void) (PB CAVD::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of base class 'conv::C' to pointer to member of derived class 'conv::CAVD'}}
      (void) (PB CPD::*) c_to_p;
      (void) (PB const CD::*) c_to_p;
      (void) (PB CD::*) c_to_cp;
#else
      (void) (PB CD::*) c_to_p;
      (void) (PAB CD::*) c_to_p;
      (void) (PVB CD::*) c_to_p;
      (void) (PAVB CD::*) c_to_p;
      (void) (PPB CD::*) c_to_p;
      (void) (PB CAD::*) c_to_p;
      (void) (PB CVD::*) c_to_p;
      (void) (PB CAVD::*) c_to_p;
      (void) (PB CPD::*) c_to_p;
      (void) (PB const CD::*) c_to_p;
      (void) (PB CD::*) c_to_cp;
#endif
    }
    {
      // Pointee upcast, class upcast, initialization syntax
#if __cplusplus >= 202400L
      PB CB::*plain(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PAB CB::*ambig_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PAB conv::CB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PVB CB::*virt_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PVB conv::CB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PAVB CB::*virtambig_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PAVB conv::CB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PPB CB::*priv_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PPB conv::CB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PB CAB::*ambig_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CAB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CAB' vs 'conv::C')}}
      PB CVB::*virt_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CVB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CVB' vs 'conv::C')}}
      PB CAVB::*virtambig_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CAVB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CAVB' vs 'conv::C')}}
      PB CPB::*priv_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CPB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CPB' vs 'conv::C')}}
      PB const CB::*cvq_add(c_to_p); // expected-error{{cannot initialize a variable of type 'const PB conv::CB::*' with an lvalue of type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PB CB::*cvq_rem(c_to_cp); // expected-error{{cannot initialize a variable of type 'PB conv::CB::*' with an lvalue of type 'const P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
#else
      PB CB::*plain(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CB::*' with an lvalue of type 'P conv::C::*'}}
      PAB CB::*ambig_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PAB conv::CB::*' with an lvalue of type 'P conv::C::*'}}
      PVB CB::*virt_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PVB conv::CB::*' with an lvalue of type 'P conv::C::*'}}
      PAVB CB::*virtambig_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PAVB conv::CB::*' with an lvalue of type 'P conv::C::*'}}
      PPB CB::*priv_pointee(c_to_p); // expected-error{{cannot initialize a variable of type 'PPB conv::CB::*' with an lvalue of type 'P conv::C::*'}}
      PB CAB::*ambig_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CAB::*' with an lvalue of type 'P conv::C::*'}}
      PB CVB::*virt_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CVB::*' with an lvalue of type 'P conv::C::*'}}
      PB CAVB::*virtambig_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CAVB::*' with an lvalue of type 'P conv::C::*'}}
      PB CPB::*priv_class(c_to_p); // expected-error{{cannot initialize a variable of type 'PB conv::CPB::*' with an lvalue of type 'P conv::C::*'}}
      PB const CB::*cvq_add(c_to_p); // expected-error{{cannot initialize a variable of type 'const PB conv::CB::*' with an lvalue of type 'P conv::C::*'}}
      PB CB::*cvq_rem(c_to_cp); // expected-error{{cannot initialize a variable of type 'PB conv::CB::*' with an lvalue of type 'const P conv::C::*'}}
#endif
    }
    {
      // Pointee upcast, class upcast, assignment syntax
#if __cplusplus >= 202400L
      PB CB::*plain; plain = c_to_p; // expected-error{{assigning to 'PB conv::CB::*' from incompatible type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PAB CB::*ambig_pointee; ambig_pointee = c_to_p; // expected-error{{assigning to 'PAB conv::CB::*' from incompatible type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PVB CB::*virt_pointee; virt_pointee = c_to_p; // expected-error{{assigning to 'PVB conv::CB::*' from incompatible type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PAVB CB::*virtambig_pointee; virtambig_pointee = c_to_p; // expected-error{{assigning to 'PAVB conv::CB::*' from incompatible type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PPB CB::*priv_pointee; priv_pointee = c_to_p; // expected-error{{assigning to 'PPB conv::CB::*' from incompatible type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PB CAB::*ambig_class; ambig_class = c_to_p; // expected-error{{assigning to 'PB conv::CAB::*' from incompatible type 'P conv::C::*': different classes ('conv::CAB' vs 'conv::C')}}
      PB CVB::*virt_class; virt_class = c_to_p; // expected-error{{assigning to 'PB conv::CVB::*' from incompatible type 'P conv::C::*': different classes ('conv::CVB' vs 'conv::C')}}
      PB CAVB::*virtambig_class; virtambig_class = c_to_p; // expected-error{{assigning to 'PB conv::CAVB::*' from incompatible type 'P conv::C::*': different classes ('conv::CAVB' vs 'conv::C')}}
      PB CPB::*priv_class; priv_class = c_to_p; // expected-error{{assigning to 'PB conv::CPB::*' from incompatible type 'P conv::C::*': different classes ('conv::CPB' vs 'conv::C')}}
      PB const CB::*cvq_add; cvq_add = c_to_p; // expected-error{{assigning to 'const PB conv::CB::*' from incompatible type 'P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
      PB CB::*cvq_rem; cvq_rem = c_to_cp; // expected-error{{assigning to 'PB conv::CB::*' from incompatible type 'const P conv::C::*': different classes ('conv::CB' vs 'conv::C')}}
#else
      PB CB::*plain; plain = c_to_p; // expected-error{{assigning to 'PB conv::CB::*' from incompatible type 'P conv::C::*'}}
      PAB CB::*ambig_pointee; ambig_pointee = c_to_p; // expected-error{{assigning to 'PAB conv::CB::*' from incompatible type 'P conv::C::*'}}
      PVB CB::*virt_pointee; virt_pointee = c_to_p; // expected-error{{assigning to 'PVB conv::CB::*' from incompatible type 'P conv::C::*'}}
      PAVB CB::*virtambig_pointee; virtambig_pointee = c_to_p; // expected-error{{assigning to 'PAVB conv::CB::*' from incompatible type 'P conv::C::*'}}
      PPB CB::*priv_pointee; priv_pointee = c_to_p; // expected-error{{assigning to 'PPB conv::CB::*' from incompatible type 'P conv::C::*'}}
      PB CAB::*ambig_class; ambig_class = c_to_p; // expected-error{{assigning to 'PB conv::CAB::*' from incompatible type 'P conv::C::*'}}
      PB CVB::*virt_class; virt_class = c_to_p; // expected-error{{assigning to 'PB conv::CVB::*' from incompatible type 'P conv::C::*'}}
      PB CAVB::*virtambig_class; virtambig_class = c_to_p; // expected-error{{assigning to 'PB conv::CAVB::*' from incompatible type 'P conv::C::*'}}
      PB CPB::*priv_class; priv_class = c_to_p; // expected-error{{assigning to 'PB conv::CPB::*' from incompatible type 'P conv::C::*'}}
      PB const CB::*cvq_add; cvq_add = c_to_p; // expected-error{{assigning to 'const PB conv::CB::*' from incompatible type 'P conv::C::*'}}
      PB CB::*cvq_rem; cvq_rem = c_to_cp; // expected-error{{assigning to 'PB conv::CB::*' from incompatible type 'const P conv::C::*'}}
#endif
    }
    {
      // Pointee upcast, class upcast, static_cast
#if __cplusplus >= 202400L
      (void) static_cast<PB CB::*>(c_to_p);
      (void) static_cast<PAB CB::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'conv::P' (derived class) to pointer to member of type 'PAB' (base class)}}
      (void) static_cast<PVB CB::*>(c_to_p); // expected-error{{conversion from pointer to member of type 'conv::P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      (void) static_cast<PAVB CB::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'conv::P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      (void) static_cast<PPB CB::*>(c_to_p); // expected-error{{cannot cast 'conv::P' to its private base class 'PPB'}}
      (void) static_cast<PB CAB::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAB'}}
      (void) static_cast<PB CVB::*>(c_to_p); // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVB' via virtual base 'conv::CVB' is not allowed}}
      (void) static_cast<PB CAVB::*>(c_to_p); // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAVB'}}
      (void) static_cast<PB CPB::*>(c_to_p); // expected-error{{cannot cast 'conv::C' to its private base class 'conv::CPB'}}
      (void) static_cast<PB const CB::*>(c_to_p);
      (void) static_cast<PB CB::*>(c_to_cp); // expected-error{{static_cast from 'const P conv::C::*' to 'PB conv::CB::*' casts away qualifiers}}
#else
      (void) static_cast<PB CB::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::CB::*' is not allowed}}
      (void) static_cast<PAB CB::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PAB conv::CB::*' is not allowed}}
      (void) static_cast<PVB CB::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PVB conv::CB::*' is not allowed}}
      (void) static_cast<PAVB CB::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PAVB conv::CB::*' is not allowed}}
      (void) static_cast<PPB CB::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PPB conv::CB::*' is not allowed}}
      (void) static_cast<PB CAB::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::CAB::*' is not allowed}}
      (void) static_cast<PB CVB::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::CVB::*' is not allowed}}
      (void) static_cast<PB CAVB::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::CAVB::*' is not allowed}}
      (void) static_cast<PB CPB::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'PB conv::CPB::*' is not allowed}}
      (void) static_cast<PB const CB::*>(c_to_p); // expected-error{{static_cast from 'P conv::C::*' to 'const PB conv::CB::*' is not allowed}}
      (void) static_cast<PB CB::*>(c_to_cp); // expected-error{{static_cast from 'const P conv::C::*' to 'PB conv::CB::*' is not allowed}}
#endif
    }
    {
      // Pointee upcast, class upcast, function-style cast
#if __cplusplus >= 202400L
      typedef PB CB::*CbToPb; (void) CbToPb(c_to_p);
      typedef PAB CB::*CbToPab; (void) CbToPab(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'conv::P' (derived class) to pointer to member of type 'PAB' (base class)}}
      typedef PVB CB::*CbToPvb; (void) CbToPvb(c_to_p); // expected-error{{conversion from pointer to member of type 'conv::P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      typedef PAVB CB::*CbToPavb; (void) CbToPavb(c_to_p); // expected-error{{ambiguous conversion from pointer to member of type 'conv::P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      typedef PPB CB::*CbToPpb; (void) CbToPpb(c_to_p);
      typedef PB CAB::*CabToPb; (void) CabToPb(c_to_p); // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAB'}}
      typedef PB CVB::*CvbToPb; (void) CvbToPb(c_to_p); // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVB' via virtual base 'conv::CVB' is not allowed}}
      typedef PB CAVB::*CavbToPb; (void) CavbToPb(c_to_p); // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAVB'}}
      typedef PB CPB::*CpbToPb; (void) CpbToPb(c_to_p);
      typedef PB const CB::*CbToConstPb; (void) CbToConstPb(c_to_p);
      typedef PB CB::*CbToPb; (void) CbToPb(c_to_cp);
#else
      typedef PB CB::*CbToPb; (void) CbToPb(c_to_p);
      typedef PAB CB::*CbToPab; (void) CbToPab(c_to_p);
      typedef PVB CB::*CbToPvb; (void) CbToPvb(c_to_p);
      typedef PAVB CB::*CbToPavb; (void) CbToPavb(c_to_p);
      typedef PPB CB::*CbToPpb; (void) CbToPpb(c_to_p);
      typedef PB CAB::*CabToPb; (void) CabToPb(c_to_p);
      typedef PB CVB::*CvbToPb; (void) CvbToPb(c_to_p);
      typedef PB CAVB::*CavbToPb; (void) CavbToPb(c_to_p);
      typedef PB CPB::*CpbToPb; (void) CpbToPb(c_to_p);
      typedef PB const CB::*CbToConstPb; (void) CbToConstPb(c_to_p);
      typedef PB CB::*CbToPb; (void) CbToPb(c_to_cp);
#endif
    }
    {
      // Pointee upcast, class upcast, C-style cast
#if __cplusplus >= 202400L
      (void) (PB CB::*) c_to_p;
      (void) (PAB CB::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of type 'conv::P' (derived class) to pointer to member of type 'PAB' (base class)}}
      (void) (PVB CB::*) c_to_p; // expected-error{{conversion from pointer to member of type 'conv::P' to pointer to member of type 'PVB' via virtual base 'conv::PVB' is not allowed}}
      (void) (PAVB CB::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of type 'conv::P' (derived class) to pointer to member of type 'PAVB' (base class)}}
      (void) (PPB CB::*) c_to_p;
      (void) (PB CAB::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAB'}}
      (void) (PB CVB::*) c_to_p; // expected-error{{conversion from pointer to member of class 'conv::C' to pointer to member of class 'conv::CVB' via virtual base 'conv::CVB' is not allowed}}
      (void) (PB CAVB::*) c_to_p; // expected-error{{ambiguous conversion from pointer to member of derived class 'conv::C' to pointer to member of base class 'conv::CAVB'}}
      (void) (PB CPB::*) c_to_p;
      (void) (PB const CB::*) c_to_p;
      (void) (PB CB::*) c_to_cp;
#else
      (void) (PB CB::*) c_to_p;
      (void) (PAB CB::*) c_to_p;
      (void) (PVB CB::*) c_to_p;
      (void) (PAVB CB::*) c_to_p;
      (void) (PPB CB::*) c_to_p;
      (void) (PB CAB::*) c_to_p;
      (void) (PB CVB::*) c_to_p;
      (void) (PB CAVB::*) c_to_p;
      (void) (PB CPB::*) c_to_p;
      (void) (PB const CB::*) c_to_p;
      (void) (PB CB::*) c_to_cp;
#endif
    }
  }
}
