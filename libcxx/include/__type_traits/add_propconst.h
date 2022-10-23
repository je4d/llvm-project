//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___TYPE_TRAITS_ADD_PROPCONST_H
#define _LIBCPP___TYPE_TRAITS_ADD_PROPCONST_H

#include <__config>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCPP_STD_VER >= 23
_LIBCPP_BEGIN_NAMESPACE_STD

template <class _Tp> struct _LIBCPP_TEMPLATE_VIS add_propconst {
  typedef _LIBCPP_NODEBUG propconst _Tp type;
};

template <class _Tp> using add_propconst_t = typename add_propconst<_Tp>::type;

_LIBCPP_END_NAMESPACE_STD
#endif

#endif // _LIBCPP___TYPE_TRAITS_ADD_PROPCONST_H
