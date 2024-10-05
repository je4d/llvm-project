//int* i = nullptr;
//constexpr int* foo() { return i; } // note_constexpr_ltor_non_const_int
//constexpr int* j = foo();

// see also:
// note_constexpr_ltor_non_integral 
// note_constexpr_ltor_non_constexpr

//struct B {}
//
//#include <typeinfo>

static bool TRUE = true;

void foi(int) {}

struct B
{
    int i = 43;
};

static int g_i;
struct A
{
    constexpr A(unsigned int magic, B* alloc, bool log = true)
        : magic{magic}
        , value{&g_i}
        , expr{&foi}
        , dynalloc{ alloc }
        , dynalloc2{ alloc }
        , u{ .dynalloc3 = alloc }
        //, u{ .j = 32 }
//        , type{&typeid(int)}
        , log{log}
    { if (log) if consteval { __constexpr_print("evaluating A::A()"); } }

    A(A&&) = delete;

    constexpr ~A()
    { if (log) if consteval { __constexpr_print("evaluating A:~A()"); } /*delete dynalloc;*/ }
    int start{0x11223344};
    unsigned int magic;
    int* value;
    void(*expr)(int);
    propconst B* dynalloc;
    const B* dynalloc2;
    union {
        int j = 123;
        B* dynalloc3;
    } u;
//    const std::type_info* type;
    bool log;
    int end{0x55667788};
    typedef float float4 __attribute__((ext_vector_type(4)));
    float4 floats{};
};

struct C : A { using A::A;};

    // note_constexpr_double_delete: delete of pointer that has already been deleted
/*
constexpr int f()
{
    A a;
    return *a.i;
}
*/

constexpr C constexpr_a{0xaabbccdd, new B[2]{}};  // err_constexpr_var_requires_const_init
                     // note_constexpr_dynamic_alloc: pointer to heap-allocated object is not a constant expression
                     //
                     // A pointer to a heap-allocated object should be a constant expression if any other variable in the same location would be a constant expression



// err_constexpr_var_requires_const_init: constexpr variable 'constexpr_a' must be initialized by a constant expression
// note_constexpr_memory_leak:            allocation performed here was not deallocated

// constexpr int F = f();

void myfunc()
{
    //A a{0x1234, false};
    //(void) &a;
    auto* b = &constexpr_a + 1;
}
