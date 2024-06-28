struct A {
    propconst int* i = nullptr;
};

struct B {
    B& operator=(B&) { return *this; }
};

struct C {
    B b;
};

int f()
{
    A a;
    A a2;
    a2 = a;

    C c;
    C c2;
    c2 = c;
    return 0;
}
