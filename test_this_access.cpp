struct X
{
  propconst int* ip;


  void f()
  {
    int* ip2 = this->ip;
  }

  void f() const
  {
    const int* ip2 = this->ip;
    (void)ip2;
  }
};

static_assert(__cplusplus == 0);
