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

  //Term assertion1 = s->make_term(Or, a, s->make_term(And, b, s->make_term(Not, b)));
  Term assertion1 = s->make_term(Equal, a, a);
  s->assert_formula(assertion1);

  Term simplified1 = s->simplify(assertion1);
  std::cout << simplified1->to_string() << std::endl;
  assert(simplified1->to_string() == "true");

  Term assertion2 = s->make_term(Not, s->make_term(Equal, a, a));
  s->assert_formula(assertion2);

  Term simplified2 = s->simplify(assertion2);
  std::cout << simplified2->to_string() << std::endl;
  assert(simplified2->to_string() == "false");

  s->pop();

  return 0;
}

