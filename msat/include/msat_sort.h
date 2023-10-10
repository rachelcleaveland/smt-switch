/*********************                                                        */
/*! \file msat_sort.h
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief MathSAT implementation of AbsSort
**
**
**/

#pragma once

#include "sort.h"

#include "mathsat.h"

namespace smt {
// forward declaration
class MsatSolver;

class MsatSort : public AbsSort
{
 public:
  MsatSort(msat_env e, msat_type t) : env(e), type(t), is_uf_type(false) {};
  MsatSort(msat_env e, msat_type t, msat_decl d) : env(e), type(t), uf_decl(d), is_uf_type(true) {};
  ~MsatSort() = default;
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
  bool compare(const Sort &) const override;
  SortKind get_sort_kind() const override;

  // getters for solver-specific objects
  // for interacting with third-party MathSAT-specific software
  msat_type get_msat_type() const { return type; }

 protected:
  msat_env env;
  msat_type type;
  msat_decl uf_decl;
  bool is_uf_type;

  friend class MsatSolver;
};

Sort make_shared_sort(msat_env e, msat_type t);
Sort make_shared_sort(msat_env e, msat_type t, msat_decl d);

}  // namespace smt

