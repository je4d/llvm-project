struct A
{
    constexpr A(const int* ip) : ip { ip } {}
    constexpr ~A() { /*delete ip;*/ }
    constexpr const int& get() const { return *ip; }
    const int* ip;
};

constexpr int constexpr_alloc_0_ga = 0x44332211;
constexpr A ga{&constexpr_alloc_0_ga};

void f()
{
    volatile int a = ga.get();
}
