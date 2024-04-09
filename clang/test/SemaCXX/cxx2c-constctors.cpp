// RUN: %clang_cc1 -std=c++26 -verify -Wall -Wshadow %s

struct NCOnly {
    NCOnly() = default;
    NCOnly(const NCOnly&) = default;
};

struct COnly {
    COnly() = default; // expected-note 2{{candidate constructor not viable: requires 0 arguments, but 1 was provided}}
    const COnly(const COnly&) = default;
};

struct Overloaded {
    constexpr Overloaded() = default; // expected-note 2 {{candidate constructor not viable: requires 0 arguments, but 1 was provided}}
    constexpr Overloaded(Overloaded&) : used_const_ctor{false} {} // expected-note 2 {{candidate constructor not viable: 1st argument ('const Overloaded') would lose const qualifier}}
    constexpr const Overloaded(const Overloaded&)  : used_const_ctor{true} {}
    bool used_const_ctor{};
};

namespace TestConstruction {
  template <typename T, typename U>
  constexpr bool constCtorWasUsed()
  {
    U arg;
    T var{arg}; // expected-error {{no matching constructor for initialization of 'Overloaded'}}
    return var.used_const_ctor;
  }

  void f() {
    NCOnly nco_nc;
    const NCOnly nco_c;
    NCOnly nco_ncc_from_nc = nco_nc;
    NCOnly nco_ncc_from_c = nco_c;
    const NCOnly nco_cc_from_nc = nco_nc;
    const NCOnly nco_cc_from_c = nco_c;

    COnly co_nc;
    const COnly co_c;
    COnly co_ncc_from_nc = co_nc; // expected-error {{no matching constructor for initialization of 'COnly'}}
    COnly co_ncc_from_c = co_c;  // expected-error {{no matching constructor for initialization of 'COnly'}}
    const COnly co_cc_from_nc = co_nc;
    const COnly co_cc_from_c = co_c;

    Overloaded o_nc;
    const Overloaded o_c;
    Overloaded o_nc_from_nc{o_nc};
    Overloaded o_nc_from_c{o_c}; // expected-error {{no matching constructor for initialization of 'Overloaded'}}
    const Overloaded o_c_from_nc{o_nc};
    const Overloaded o_c_from_c{o_c};

    static_assert(constCtorWasUsed<Overloaded, Overloaded>() == false);
    // expected-error@+2 {{static assertion expression is not an integral constant expression}}
    // expected-note@+1 {{in instantiation of function template specialization 'TestConstruction::constCtorWasUsed<Overloaded, const Overloaded>' requested here}}
    static_assert(constCtorWasUsed<Overloaded, const Overloaded>() == false);
    static_assert(constCtorWasUsed<const Overloaded, Overloaded>() == false);
    static_assert(constCtorWasUsed<const Overloaded, const Overloaded>() == true);
  }
}
