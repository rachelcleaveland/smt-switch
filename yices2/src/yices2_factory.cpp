/*********************                                                        */
/*! \file yices2_factory.cpp
** \verbatim
** Top contributors (to current version):
**   Amalee Wilson
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Factory for creating a Yices2 SmtSolver
**
**
**/

#include "yices2_factory.h"

#include "yices2_solver.h"

#include "logging_solver.h"

namespace smt {

bool initialized = false;

SmtSolver make_shared_solver() {
  Yices2Solver *bs = new Yices2Solver();
  AbsSmtSolver *abss = dynamic_cast<AbsSmtSolver*>(bs);
  return RachelsSharedPtr<AbsSmtSolver>(abss);
}

/* Yices2SolverFactory implementation with logging */
SmtSolver Yices2SolverFactory::create(bool logging)
{
  // Must initialize only once, even if planning to
  // create multiple contexts.
  // Different instances of the solver will have
  // different contexts. Yices2 must be configured
  // with --enable-thread-safety in order for
  // multiple threads to manipulate different contexts.
  // https://github.com/SRI-CSL/yices2#support-for-thread-safety
  if (!initialized)
  {
    yices_init();
    initialized = true;
  }

  SmtSolver solver = make_shared_solver();
  if (logging)
  {
    solver = create_logging_solver(solver);
  }
  return solver;
}
/* end Yices2SolverFactory logging implementation */

}  // namespace smt
