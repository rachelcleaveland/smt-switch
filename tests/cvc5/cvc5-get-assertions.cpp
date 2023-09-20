/*********************                                                        */
/*! \file cvc5_test.cpp
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief
**
**
**/

#include <iostream>

#include "cvc5_factory.h"
#include "smt.h"
#include "assert.h"

// after full installation
// #include "smt-switch/cvc5_factory.h"
// #include "smt-switch/smt.h"

using namespace std;
using namespace smt;

int main()
{
  SmtSolver s = Cvc5SolverFactory::create(false);

  Term a = s->make_symbol("a", s->make_sort(BOOL));
  Term b = s->make_symbol("b", s->make_sort(BOOL));
  Term c = s->make_symbol("c", s->make_sort(BOOL));

  s->push();

  Term assertion1 = s->make_term(Or, a, b);
  s->assert_formula(assertion1);

  TermVec vec;
  s->get_assertions(vec);
  assert(vec.size() == 1);
  assert(vec[0]->to_string() == "(or a b)");

  Term assertion2 = s->make_term(And, s->make_term(Not,c), s->make_term(Or,a,b));
  s->assert_formula(assertion2);
  vec.clear();
  s->get_assertions(vec);
  assert(vec.size() == 2);
  assert(vec[0]->to_string() == "(or a b)");
  assert(vec[1]->to_string() == "(and (not c) (or a b))");

  s->pop();

  Term assertion3 = s->make_term(And,s->make_term(And,a,b),c);
  s->assert_formula(assertion3);
  vec.clear();
  s->get_assertions(vec);
  assert(vec.size() == 1);
  assert(vec[0]->to_string() == "(and (and a b) c)");

  return 0;
}

