/*********************                                                        */
/*! \file yices2_sort.h
** \verbatim
** Top contributors (to current version):
**   Amalee Wilson
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Yices2 implementation of AbsSort
**
**
**/

#pragma once

#include "exceptions.h"
#include "sort.h"
#include "utils.h"

#include "yices.h"

namespace smt {

// forward declaration
class Yices2Solver;

class Yices2Sort : public AbsSort
{
 public:
  // Non-functions
  Yices2Sort(type_t y_type) : type(y_type), is_function(false){};

  // Functions
  Yices2Sort(type_t y_type, bool is_fun) : type(y_type), is_function(is_fun){};

  ~Yices2Sort() = default;
  std::size_t hash() const override;
  uint64_t get_width() const override;
  Sort get_indexsort() const override;
  Sort get_elemsort() const override;
  SortVec get_domain_sorts() const override;
  Sort get_codomain_sort() const override;
  std::string get_uninterpreted_name() const override;
  size_t get_arity() const override;
  SortVec get_uninterpreted_param_sorts() const override;
  Datatype get_datatype() const override;
  bool compare(const Sort & s) const override;
  SortKind get_sort_kind() const override;
  type_t get_ytype() { return type; };

 protected:
  type_t type;
  bool is_function;

  friend class Yices2Solver;
};

Sort make_shared_sort(type_t sort);
Sort make_shared_sort(type_t sort, bool b);

}  // namespace smt

