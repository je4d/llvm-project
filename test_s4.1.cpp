struct A
{
  //int const* i_pc_ptr;
  int propconst* i_pc_ptr;
  int propconst& i_pc_ref;

  void fp()
  {
    using decltype_decl = decltype(i_pc_ptr);
    using decltype_decl_exp = int propconst*;
    static_assert(__is_same(decltype_decl, decltype_decl_exp));
    using decltype_expr = decltype((i_pc_ptr));
    using decltype_expr_exp = int*&;
    static_assert(__is_same(decltype_expr, decltype_expr_exp));
  };

  void fp() const
  {
    using decltype_decl = decltype(i_pc_ptr);
    using decltype_decl_exp = int propconst*;
    static_assert(__is_same(decltype_decl, decltype_decl_exp));
    using decltype_expr = decltype((i_pc_ptr));
    using decltype_expr_exp = int const* const&;
    static_assert(__is_same(decltype_expr, decltype_expr_exp));
  };

  void fr()
  {
    using decltype_decl = decltype(i_pc_ref);
    using decltype_decl_exp = int propconst&;
    static_assert(__is_same(decltype_decl, decltype_decl_exp));
    using decltype_expr = decltype((i_pc_ref));
    using decltype_expr_exp = int&;
    static_assert(__is_same(decltype_expr, decltype_expr_exp));
  };

  void fr() const
  {
    using decltype_decl = decltype(i_pc_ref);
    using decltype_decl_exp = int propconst&;
    static_assert(__is_same(decltype_decl, decltype_decl_exp));
    using decltype_expr = decltype((i_pc_ref));
    using decltype_expr_exp = int const&;
    static_assert(__is_same(decltype_expr, decltype_expr_exp));
  };
};
