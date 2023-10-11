/*********************                                                        */
/*! \file test-generic-sort.cpp
** \verbatim
** Top contributors (to current version):
**   Yoni Zohar
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
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <array>
#include <iostream>
#include <memory>
#include <vector>
#include <sstream>
#include "assert.h"

// note: this file depends on the CMake build infrastructure
// specifically defined macros
// it cannot be compiled outside of the build
#include "cvc5_factory.h"
#include "generic_sort.h"
#include "gtest/gtest.h"
#include "smt.h"
#include "test-utils.h"

using namespace smt;
using namespace std;

int main()
{
  GenericSort s1(INT);
  GenericSort s2(INT);
  std::cout << "testing basic properties of sorts" << std::endl;
  assert(s1.hash() == s2.hash());
  assert(s1.to_string() == s2.to_string());
  assert(s2.to_string() == s1.to_string());
  assert((s1.get_sort_kind()) == (s2.get_sort_kind()));
  assert(s1.get_sort_kind() == INT);

  Sort int1 = make_generic_sort(INT);
  Sort int2 = make_generic_sort(INT);
  assert(int1 == int2);
  Sort bv4 = make_generic_sort(BV, 4);
  Sort bv5 = make_generic_sort(BV, 5);
  assert(bv4 != bv5);
  assert(bv4 != int1);
  Sort inttobv4 = make_generic_sort(FUNCTION, int1, bv4);
  Sort inttobv4_second = make_generic_sort(FUNCTION, int2, bv4);
  assert(inttobv4 == inttobv4_second);
  Sort arr = make_generic_sort(ARRAY, int1, bv4);
  assert(arr != inttobv4);
  assert(arr->get_indexsort() == int1);
  assert(arr->get_elemsort() == bv4);
  assert(bv4->get_width() == 4);

  Sort us1 = make_uninterpreted_generic_sort("sort1", 0);
  Sort us2 = make_uninterpreted_generic_sort("sort1", 0);
  assert(us1 == us2);
  Sort us3 = make_uninterpreted_generic_sort("sort3", 0);
  assert(us1 != us3);
  assert(us1->get_uninterpreted_name() == "sort1");
  assert(us1->get_arity() == 0);

  // Creates a new datatype with one constructor
  DatatypeDecl new_dt_decl = make_shared_datatype_decl("testSort1");
  Datatype new_dt = make_shared_datatype(new_dt_decl);
  GenericDatatype *new_dt_ = dynamic_cast<GenericDatatype*>(new_dt.get());
  DatatypeConstructorDecl new_dt_cons_decl =
      make_shared_datatype_constructor("Cons");
  new_dt_->add_constructor(new_dt_cons_decl);
  shared_ptr<GenericDatatypeSort> dt_sort =
      make_shared<GenericDatatypeSort>(new_dt);

  // Creates a different datatype with one constructor
  DatatypeDecl new2_dt_decl = make_shared_datatype_decl("testSort2");
  Datatype new2_dt = make_shared_datatype(new2_dt_decl);
  GenericDatatype *new2_dt_ = dynamic_cast<GenericDatatype*>(new2_dt.get());
  DatatypeConstructorDecl new2_dt_cons_decl = make_shared_datatype_constructor("test2");
  new2_dt_->add_constructor(new2_dt_cons_decl);
  shared_ptr<GenericDatatypeSort> dt_sort2 =
      make_shared<GenericDatatypeSort>(new2_dt);
  // Asserts that the sorts are distinct from one another and that the
  // copy operator works with said sorts
  assert(dt_sort != dt_sort2);
  auto copy = dt_sort;
  assert(dt_sort == copy);
  // Compares string names of the sorts
  assert(dt_sort->to_string() != dt_sort2->to_string());
  // Checks for valid sortKinds
  assert((dt_sort->get_sort_kind()) == (dt_sort2->get_sort_kind()));
  assert(dt_sort->get_sort_kind() == DATATYPE);
  assert(dt_sort2->get_sort_kind() == DATATYPE);

  return 0;
}
