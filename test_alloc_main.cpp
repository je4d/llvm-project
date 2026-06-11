struct A
{
    constexpr A(const int* ip) : ip { ip } {}
    constexpr ~A() { /*delete ip;*/ }
    constexpr const int& get() const { return *ip; }
    const int* ip;
};

constexpr A ga{new int{0x44332211}};

int main()
{
    volatile int a = ga.get();
}
