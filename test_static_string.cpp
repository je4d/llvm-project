struct A
{
    int* i = new int{0};
};

void f()
{
    static A a;
}
