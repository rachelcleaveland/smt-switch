/*********************                                                        */
/*! \file boolector_sort.cpp
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Boolector implementation of AbsSort
**
**
**/

#include <sstream>

#include "boolector_sort.h"

namespace smt {

/* BoolectorSolver implementation */

Sort make_shared_sort(SortKind sk, Btor * b, BoolectorSort s) {
  BoolectorSortBase *bsb = new BoolectorSortBase(sk,b,s);
  AbsSort *abss = dynamic_cast<AbsSort *>(bsb);
  return RachelsSharedPtr<AbsSort>(abss);
}

Sort make_shared_sort(Btor * b, BoolectorSort s, uint64_t w) {
  BoolectorBVSort *bsb = new BoolectorBVSort(b,s,w);
  AbsSort *abss = dynamic_cast<AbsSort *>(bsb);
  return RachelsSharedPtr<AbsSort>(abss);
}

Sort make_shared_sort(Btor * b, BoolectorSort s, Sort is, Sort es) {
  BoolectorArraySort *bsb = new BoolectorArraySort(b,s,is,es);
  AbsSort *abss = dynamic_cast<AbsSort *>(bsb);
  return RachelsSharedPtr<AbsSort>(abss);
}

Sort make_shared_sort(Btor * b, BoolectorSort s, SortVec sorts, Sort sort) {
  BoolectorUFSort *bufs = new BoolectorUFSort(b,s,sorts,sort);
  AbsSort *abss = dynamic_cast<AbsSort *>(bufs);
  return RachelsSharedPtr<AbsSort>(abss);
}

BoolectorSortBase::~BoolectorSortBase() { boolector_release_sort(btor, sort); }

std::size_t BoolectorSortBase::hash() const
{
  // TODO: come up with better hash function
  std::size_t hash = sk;

  if(sk == BV)
  {
    hash ^= get_width();
  }
  else if(sk == ARRAY)
  {
    hash ^= get_indexsort()->hash();
    hash ^= get_elemsort()->hash();
  }
  return hash;
}


// by default the following get_* methods don't work
// overloaded in derived classes
uint64_t BoolectorSortBase::get_width() const
{
  throw IncorrectUsageException("Only defined for a bit-vector sort.");
};

Sort BoolectorSortBase::get_indexsort() const
{
  throw IncorrectUsageException("Only defined for an array sort.");
};

Sort BoolectorSortBase::get_elemsort() const
{
  throw IncorrectUsageException("Only defined for an array sort.");
};

SortVec BoolectorSortBase::get_domain_sorts() const
{
  throw IncorrectUsageException("Only defined for a function sort.");
};

Sort BoolectorSortBase::get_codomain_sort() const
{
  throw IncorrectUsageException("Only defined for a function sort.");
};

std::string BoolectorSortBase::get_uninterpreted_name() const
{
  throw IncorrectUsageException(
      "Boolector doesn't support uninterpreted sorts.");
}

size_t BoolectorSortBase::get_arity() const
{
  throw NotImplementedException(
      "Boolector does not support uninterpreted sorts");
}

SortVec BoolectorSortBase::get_uninterpreted_param_sorts() const
{
  throw NotImplementedException(
      "Boolector does not support uninterpreted sorts.");
}

Datatype BoolectorSortBase::get_datatype() const
{
  throw NotImplementedException("get_datatype");
};

bool BoolectorSortBase::compare(const Sort & s) const
{
  BoolectorSortBase *bs =
      dynamic_cast<BoolectorSortBase*>(s.get());
  return (sort == bs->sort);
}
/* end BoolectorSolver implementation */

}  // namespace smt
