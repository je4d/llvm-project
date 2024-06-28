
//constexpr propconst int* foo() { return nullptr; }
//int* bar = foo();

int* ip;
struct W { propconst int*& get() { return ip; } };
int* ip2 = W{}.get();

#if 1
template <typename T>
struct wrap
{
  wrap(T val) : val{val} {}
  T& get() { return val; }
  const T& get() const { return val; }
private:
  T val;
};

/*
template <typename T>
struct wrap2_holder
{
  T val;
  using reference = T&;
  using const_reference = const T&;
  const_reference get() const { return val; }
};

template <typename T>
struct wrap2 : private wrap2_holder<T>
{
  wrap2(T val) : wrap2_holder<T>{val} {}
  T& get() { return this->val; }
  const T& get() const { return this->val; }

  using Elem = wrap2_holder<T>;

  typename Elem::const_reference elem() const {
    return static_cast<Elem const&>(*this).get();
  }
};
*/

int main()
{

  /*{
    int i = 5;
    wrap<propconst int*> w{&i};
    const auto& cw = w;
    int* ip = w.get();
    const int* const cipc = cw.get();
  }*/

  /*
  {
    int i = 5;
    wrap2<propconst int*> w{&i};
    const auto& cw = w;
    w.get();
    cw.get();
    w.elem();
    cw.elem();
  }
  */
}
#endif
