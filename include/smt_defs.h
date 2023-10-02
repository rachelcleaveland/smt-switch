/*********************                                                        */
/*! \file smt_defs.h
** \verbatim
** Top contributors (to current version):
**   Makai Mann, Clark Barrett
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Type definitions for pointers to main abstract objects.
**
**
**/

#pragma once

#include <memory>

namespace smt {

struct Op;
template <typename T>
class RachelsSharedPtr;

class AbsSort;
using Sort = std::shared_ptr<AbsSort>; // RachelsSharedPtr<AbsSort>; //

class AbsTerm;
using Term = RachelsSharedPtr<AbsTerm>;
//using Term = std::shared_ptr<AbsTerm>;

class AbsSmtSolver;
using SmtSolver = std::shared_ptr<AbsSmtSolver>;

// Datatype theory related
class AbsDatatypeDecl;
using DatatypeDecl = std::shared_ptr<AbsDatatypeDecl>;

class AbsDatatypeConstructorDecl;
using DatatypeConstructorDecl = std::shared_ptr<AbsDatatypeConstructorDecl>;

class AbsDatatype;
using Datatype = std::shared_ptr<AbsDatatype>;

}  // namespace smt

