int i = 0;

template <typename T>
struct ptr
{
  ptr(T* p) : p{p} {}

  T& get() { return *p; }
  const T& get() const { return *p; }
  T& leak() const { return *p; }

  propconst T* p;
};


void f(const ptr<int>& cip)
{
  cip.leak() = 42;
}

int main()
{
  ptr<int> ip(&i);
  ip.get() = 1;
  f(ip);
  return ip.get();
}
