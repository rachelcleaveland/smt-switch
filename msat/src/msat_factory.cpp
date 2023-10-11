/*********************                                                        */
/*! \file msat_factory.cpp
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Factory for creating a MathSAT SmtSolver
**
**
**/

#include "msat_factory.h"
#include "msat_solver.h"

#include "logging_solver.h"

namespace smt {

template <typename T>
SmtSolver make_shared_solver() {
  T *bs = new T();
  AbsSmtSolver *abss = dynamic_cast<AbsSmtSolver*>(bs);
  return RachelsSharedPtr<AbsSmtSolver>(abss);
}

/* MsatSolverFactory implementation */

SmtSolver MsatSolverFactory::create(bool logging)
{
  //MsatSolver * ms = new MsatSolver();
  //SmtSolver solver(ms);
  SmtSolver solver = make_shared_solver<MsatSolver>();
  if (logging)
  {
    solver = create_logging_solver(solver);
  }
  return solver;
}

SmtSolver MsatSolverFactory::create_interpolating_solver()
{
  SmtSolver s = make_shared_solver<MsatInterpolatingSolver>();
  return s;
}

/* end MsatSolverFactory implementation */

}  // namespace smt
