/*********************                                                        */
/*! \file term.cpp
** \verbatim
** Top contributors (to current version):
**   Makai Mann, Clark Barrett
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Abstract interface for SMT terms.
**
**
**/

#include "term.h"

namespace smt {

std::ostream & operator<<(std::ostream & output, const Term t)
{
  output << t->to_string();
  return output;
}

/* TermIterBase implementation */
const Term TermIterBase::operator*()
{
  RachelsSharedPtr<AbsTerm> s;
  return s;
}

bool TermIterBase::operator==(const TermIterBase & other) const
{
  return (typeid(*this) == typeid(other)) && equal(other);
}
/* end TermIterBase implementation */

/* TermIter implementation */
TermIter & TermIter::operator=(const TermIter & other)
{
  delete iter_;
  iter_ = other.iter_->clone();
  return *this;
}

TermIter & TermIter::operator++()
{
  ++(*iter_);
  return *this;
}

TermIter TermIter::operator++(int)
{
  TermIter it = *this;
  ++(*iter_);
  return it;
}

bool TermIter::operator==(const TermIter & other) const
{
  return (iter_ == other.iter_) || (*iter_ == *other.iter_);
}

bool TermIter::operator!=(const TermIter & other) const
{
  return !(*this == other);
}
/* end TermIter implementation */
}  // namespace smt
