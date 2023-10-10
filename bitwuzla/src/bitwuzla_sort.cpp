/*********************                                                        */
/*! \file bitwuzla_sort.cpp
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Bitwuzla implementation of AbsSort
**
**
**/

#include "bitwuzla_sort.h"

#include "assert.h"

using namespace std;

namespace smt {

Sort make_shared_sort(const BitwuzlaSort * s) {
  BzlaSort *bs = new BzlaSort(s);
  AbsSort *abss = dynamic_cast<AbsSort *>(bs);
  return RachelsSharedPtr<AbsSort>(abss);
}

BzlaSort::~BzlaSort()
{
  // TODO: figure out if sorts need to be destroyed
}

size_t BzlaSort::hash() const { return bitwuzla_sort_hash(sort); }

uint64_t BzlaSort::get_width() const { return bitwuzla_sort_bv_get_size(sort); }

Sort BzlaSort::get_indexsort() const
{
  return make_shared_sort(bitwuzla_sort_array_get_index(sort));
}

Sort BzlaSort::get_elemsort() const
{
  return make_shared_sort(bitwuzla_sort_array_get_element(sort));
}

SortVec BzlaSort::get_domain_sorts() const
{
  size_t arity;
  const BitwuzlaSort ** bsorts = bitwuzla_sort_fun_get_domain_sorts(sort, &arity);
  SortVec domain_sorts; domain_sorts.reserve(arity);

  for (size_t i = 0; i < arity; ++i)
  {
    // array is zero-terminated -- shouldn't hit the end
    assert(bsorts);
    domain_sorts.push_back(make_shared_sort(*bsorts));
    ++bsorts;
  }
  // should be at end of the array
  assert(bsorts);

  return domain_sorts;
}

Sort BzlaSort::get_codomain_sort() const
{
  return make_shared_sort(bitwuzla_sort_fun_get_codomain(sort));
}

std::string BzlaSort::get_uninterpreted_name() const
{
  throw IncorrectUsageException(
      "Bitwuzla does not support uninterpreted sorts.");
}

size_t BzlaSort::get_arity() const { return bitwuzla_sort_fun_get_arity(sort); }

SortVec BzlaSort::get_uninterpreted_param_sorts() const
{
  throw SmtException("Bitwuzla does not support uninterpreted sorts.");
}

Datatype BzlaSort::get_datatype() const
{
  throw IncorrectUsageException("Bitwuzla does not support datatypes.");
}

bool BzlaSort::compare(const Sort & s) const
{
  BzlaSort *bsort = dynamic_cast<BzlaSort*>(s.get());
  return bitwuzla_sort_is_equal(sort, bsort->sort);
}

SortKind BzlaSort::get_sort_kind() const
{
  if (bitwuzla_sort_is_bv(sort))
  {
    return BV;
  }
  else if (bitwuzla_sort_is_array(sort))
  {
    return ARRAY;
  }
  else if (bitwuzla_sort_is_fun(sort))
  {
    return FUNCTION;
  }
  else
  {
    throw SmtException("Got Bitwuzla sort of unknown SortKind.");
  }
}

}  // namespace smt
