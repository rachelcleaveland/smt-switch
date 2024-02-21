/*********************                                                        */
/*! \file cvc5-str.cpp
** \verbatim
** Top contributors (to current version):
**   Nestan Tsiskaridze
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
** 
** \brief
** Tests for strings in the cvc5 backend.
**/

#include <iostream>
#include <memory>
#include <vector>

#include "assert.h"
#include "cvc5_factory.h"
#include "smt.h"

using namespace smt;
using namespace std;

int main()
{
  SmtSolver s = Cvc5SolverFactory::create(false);
  s->set_opt("produce-models", "true");
  s->set_logic("S");
  Sort strsort = s->make_sort(STRING);
  Sort intsort = s->make_sort(INT);
  Sort regsort = s->make_sort(REGEXP);

  Term a = s->make_symbol("a", strsort);
  Term five_as = s->make_term("aaaaa", false, strsort);
  Term zero = s->make_term(0, intsort);
  Term five = s->make_term(5, intsort);

  //StrToRe
  Term s_to_re = s->make_term(StrToRe, a);
  //ReStar
  Term a_star = s->make_term(ReStar, s_to_re);
  s->assert_formula(
	s->make_term(StrInRe,five_as,a_star));
  s->assert_formula(
	s->make_term(StrInRe,a,a_star));

  Result r = s->check_sat();
  assert(r.is_sat());

  cout << "Model Values:" << endl;
  cout << "a = " << s->get_value(a) << endl;
  return 0;
}
