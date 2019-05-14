//
// Copyright Jason Rice 2015-2016
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef NBDL_MPDEF_DIRECTIVE_HPP
#define NBDL_MPDEF_DIRECTIVE_HPP

#include "tree_node.hpp"

#include <boost/hana.hpp>
#include <boost/hana/experimental/types.hpp>

#define MPDEF_DIRECTIVE(NAME)                           \
namespace tag {                                         \
  struct NAME##_t {};                                   \
  constexpr auto NAME = boost::hana::type_c<NAME##_t>;  \
}                                                       \
template<typename... Tn> constexpr auto NAME(Tn... tn)  \
{ return mpdef::make_tree_node(tag::NAME, boost::hana::make_map(tn...)); }

#define MPDEF_DIRECTIVE_LIST(NAME)                                              \
namespace tag {                                                                 \
  struct NAME##_t {};                                                           \
  constexpr auto NAME = boost::hana::type_c<NAME##_t>;                          \
}                                                                               \
template<typename... Tn> constexpr auto NAME(Tn... tn)                             \
{ return mpdef::tree_node<decltype(tag::NAME), boost::hana::tuple<Tn...>>{{tn...}}; };
//{ return mpdef::tree_node<decltype(tag::NAME), mpdef::list<Tn...>>{}; };

#define MPDEF_DIRECTIVE_LEAF(NAME)                      \
namespace tag {                                         \
  struct NAME##_t {};                                   \
  constexpr auto NAME = boost::hana::type_c<NAME##_t>;  \
}                                                       \
template<typename T> constexpr auto NAME(T t)           \
{ return mpdef::tree_node<decltype(tag::NAME), T>{t}; };

#define MPDEF_DIRECTIVE_TYPE(NAME)                      \
namespace tag {                                         \
  struct NAME##_t {};                                   \
  constexpr auto NAME = boost::hana::type_c<NAME##_t>;  \
}                                                       \
template<typename T>                                    \
constexpr auto NAME = mpdef::make_tree_node(tag::NAME, boost::hana::type_c<T>);

#define MPDEF_DIRECTIVE_TYPE_LIST(NAME)                 \
namespace tag {                                         \
  struct NAME##_t {};                                   \
  constexpr auto NAME = boost::hana::type_c<NAME##_t>;  \
}                                                       \
template<typename ...T>                                 \
constexpr auto NAME = mpdef::make_tree_node(tag::NAME, boost::hana::experimental::types<T...>{});

#endif
