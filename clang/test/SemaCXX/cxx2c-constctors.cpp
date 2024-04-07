struct NCOnly {
    NCOnly() = default;
    NCOnly(const NCOnly&) = default;
};

struct COnly {
    COnly() = default;
    COnly(const COnly&) const = default;
};

struct Overloaded {
    constexpr Overloaded() = default;
    constexpr Overloaded(Overloaded&) : used_const_ctor{false} {}
    constexpr Overloaded(const Overloaded&) const  : used_const_ctor{true} {}
    bool used_const_ctor;
};

struct A {
    A(const A&) const = default;
};

void f() {
    NCOnly nco_nc;
    const NCOnly nco_c;
    NCOnly nco_ncc = nco_nc;
    NCOnly nco_ncc = nco_c;
    const NCOnly nco_cc = nco_nc;
    const NCOnly nco_cc = nco_c;

    COnly co_nc;
    const COnly co_c;
    COnly co_ncc = co_nc; // error:
    COnly co_ncc = co_c;  // error:
    const COnly co_cc = co_nc;
    const COnly co_cc = co_c;

    Overloaded o_nc;
    const Overloaded o_c;
    static_assert(Overloaded{o_nc}.used_const_ctor == false);
    static_assert(Overloaded{o_c}.used_const_ctor == true); // error
    static_assert(const Overloaded{o_nc}.used_const_ctor == false);
    static_assert(const Overloaded{o_c}.used_const_ctor == true);
}
