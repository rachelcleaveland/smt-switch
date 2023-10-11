/*********************                                                        */
/*! \file cvc5_factory.cpp
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Factory for creating a cvc5 SmtSolver
**
**
**/

#include "cvc5_factory.h"

#include "cvc5_solver.h"
#include "logging_solver.h"

namespace smt {

template <typename T>
SmtSolver make_shared_solver() {
  T *cs = new T();
  AbsSmtSolver *abss = dynamic_cast<AbsSmtSolver*>(cs);
  return RachelsSharedPtr<AbsSmtSolver>(abss);
}

SmtSolver make_shared_solver(SmtSolver s) {
  LoggingSolver *cs = new LoggingSolver(s);
  AbsSmtSolver *abss = dynamic_cast<AbsSmtSolver*>(cs);
  return RachelsSharedPtr<AbsSmtSolver>(abss);
}

/* Cvc5SolverFactory implementation */
SmtSolver Cvc5SolverFactory::create(bool logging)
{
  SmtSolver solver = make_shared_solver<Cvc5Solver>();
  if (logging)
  {
    solver = make_shared_solver(solver);
  }
  return solver;
}

SmtSolver Cvc5SolverFactory::create_interpolating_solver()
{
  SmtSolver solver = make_shared_solver<cvc5InterpolatingSolver>();
  /*
   * Enable interpolant generation.
   * */
  solver->set_opt("produce-interpolants", "true");
  solver->set_opt("incremental", "false");
  return solver;
}
/* end Cvc5SolverFactory implementation */

}  // namespace smt
