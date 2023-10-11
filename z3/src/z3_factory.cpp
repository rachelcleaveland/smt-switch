/*********************                                                        */
/*! \file z3_factory.cpp
** \verbatim
** Top contributors (to current version):
**   Lindsey Stuntz
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Factory for creating a Z3 SmtSolver
**
**
**/

#include "z3_factory.h"

#include "logging_solver.h"
#include "z3_solver.h"

namespace smt {

SmtSolver make_shared_solver() {
  Z3Solver *bs = new Z3Solver();
  AbsSmtSolver *abss = dynamic_cast<AbsSmtSolver*>(bs);
  return RachelsSharedPtr<AbsSmtSolver>(abss);
}

/* Z3SolverFactory implementation with logging */
SmtSolver Z3SolverFactory::create(bool logging)
{
  SmtSolver solver = make_shared_solver();
  if (logging)
  {
    solver = create_logging_solver(solver);
  }
  return solver;
}
/* end Z3SolverFactory implementation */

}  // namespace smt