/*********************                                                        */
/*! \file unit-sort-inference.cpp
** \verbatim
** Top contributors (to current version):
**   Makai Mann
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Unit tests for sort inference
**
**
**/

#include "available_solvers.h"
#include "gtest/gtest.h"
#include "smt.h"
#include "sort_inference.h"

using namespace smt;
using namespace std;

namespace smt_tests {

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(UnitSortInferenceTests);
class UnitSortInferenceTests : public ::testing::Test,
                               public ::testing::WithParamInterface<SolverConfiguration>
{
 protected:
  void SetUp() override
  {
    s = create_solver(GetParam());
    s->set_opt("produce-models", "true");

    boolsort = s->make_sort(BOOL);
    bvsort4 = s->make_sort(BV, 4);
    bvsort5 = s->make_sort(BV, 5);
    arrsort = s->make_sort(ARRAY, bvsort4, bvsort4);
    funsort = s->make_sort(FUNCTION, { bvsort4, bvsort4, boolsort });

    b1 = s->make_symbol("b1", boolsort);
    b2 = s->make_symbol("b2", boolsort);
    p = s->make_symbol("p", bvsort4);
    q = s->make_symbol("q", bvsort4);
    w = s->make_symbol("w", bvsort5);
    arr = s->make_symbol("arr", arrsort);
    f = s->make_symbol("f", funsort);
  }
  SmtSolver s;
  Sort boolsort, bvsort4, bvsort5, arrsort, funsort;
  Term b1, b2, p, q, w, arr, f;
};

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(UnitArithmeticSortInferenceTests);
class UnitArithmeticSortInferenceTests : public UnitSortInferenceTests
{
 protected:
  void SetUp() override
  {
    UnitSortInferenceTests::SetUp();
    realsort = s->make_sort(REAL);
    intsort = s->make_sort(INT);

    x = s->make_symbol("x", realsort);
    y = s->make_symbol("y", realsort);
    z = s->make_symbol("z", realsort);

    xint = s->make_symbol("xint", intsort);
    yint = s->make_symbol("yint", intsort);
    zint = s->make_symbol("zint", intsort);
  }
  Sort realsort, intsort;
  Term x, y, z, xint, yint, zint;
};

TEST_P(UnitSortInferenceTests, HelperTests)
{
  EXPECT_TRUE(equal_sorts({ boolsort, boolsort }));
  EXPECT_TRUE(equal_sorts({ bvsort4, bvsort4 }));
  EXPECT_TRUE(equal_sorts({ arrsort, arrsort }));
  EXPECT_TRUE(equal_sorts({ funsort, funsort }));
  EXPECT_FALSE(equal_sorts({ boolsort, bvsort4 }));
  EXPECT_FALSE(equal_sorts({ bvsort4, bvsort5 }));

  EXPECT_TRUE(equal_sortkinds({ bvsort4, bvsort5 }));
  EXPECT_TRUE(equal_sortkinds({ funsort, funsort }));
  EXPECT_FALSE(equal_sortkinds({ funsort, bvsort4 }));

  EXPECT_FALSE(check_ite_sorts({ boolsort, bvsort4, bvsort5 }));

  // if solver aliases booleans and bitvectors of width 1, this test fails
  if (!solver_has_attribute(s->get_solver_enum(), BOOL_BV1_ALIASING))
  {
    EXPECT_TRUE(check_ite_sorts({ boolsort, bvsort4, bvsort4 }));
    EXPECT_TRUE(bool_sorts({ boolsort }));
  }

  EXPECT_TRUE(bv_sorts({ bvsort4 }));
  EXPECT_TRUE(array_sorts({ arrsort }));
  EXPECT_TRUE(function_sorts({ funsort }));
}

TEST_P(UnitSortInferenceTests, SortednessTests)
{
  /******** Booleans ********/
  EXPECT_TRUE(check_sortedness(Equal, (TermVec){ b1, b2 }));
  EXPECT_TRUE(check_sortedness(Distinct, (TermVec){ b1, b2 }));

  // wrong operator
  // if solver aliases booleans and bitvectors of width 1, this test fails
  if (!solver_has_attribute(s->get_solver_enum(), BOOL_BV1_ALIASING))
  {
    EXPECT_TRUE(check_sortedness(And, (TermVec){ b1, b2 }));
    EXPECT_TRUE(check_sortedness(Xor, (TermVec){ b1, b2 }));
    EXPECT_FALSE(check_sortedness(BVAnd, (TermVec){ b1, b2 }));
  }

  EXPECT_FALSE(check_sortedness(Ge, (TermVec){ b1, b2 }));

  // wrong number of arguments
  EXPECT_FALSE(check_sortedness(Xor, (TermVec){ b1 }));

  /******* Bitvectors *******/
  EXPECT_TRUE(check_sortedness(Equal, (TermVec){ p, q }));
  EXPECT_TRUE(check_sortedness(Distinct, (TermVec){ p, q }));
  EXPECT_TRUE(check_sortedness(BVAdd, (TermVec){ p, q }));
  EXPECT_TRUE(check_sortedness(BVAnd, (TermVec){ p, q }));
  EXPECT_TRUE(check_sortedness(BVAdd, (TermVec){ p, q }));
  EXPECT_TRUE(check_sortedness(BVUlt, (TermVec){ p, q }));
  EXPECT_TRUE(check_sortedness(BVNeg, (TermVec){ p }));
  // different bit-widths
  EXPECT_FALSE(check_sortedness(BVAdd, (TermVec){ p, w }));
  EXPECT_FALSE(check_sortedness(Distinct, (TermVec){ p, w }));

  /********* Arrays ********/
  EXPECT_TRUE(check_sortedness(Select, (TermVec){arr, p}));
  EXPECT_TRUE(check_sortedness(Store, (TermVec){arr, p, q}));
  EXPECT_TRUE(check_sortedness(Equal, (TermVec){ arr, s->make_term(Store, arr, p, q) }));
  // wrong bit-width
  EXPECT_FALSE(check_sortedness(Select, (TermVec){arr, w}));
  EXPECT_FALSE(check_sortedness(Store, (TermVec){arr, p, w}));
  EXPECT_FALSE(check_sortedness(Store, (TermVec){arr, w, p}));

  // BTOR doesn't support getting the sort of a function yet
  if (s->get_solver_enum() != BTOR)
  {
    /********* Functions ********/
    EXPECT_TRUE(check_sortedness(Apply, (TermVec){ f, p, q }));
    // wrong type
    EXPECT_FALSE(check_sortedness(Apply, (TermVec){ f, p, w }));
    EXPECT_FALSE(check_sortedness(Apply, (TermVec){ f, arr, q }));
    // wrong number of arguments
    EXPECT_FALSE(check_sortedness(Apply, (TermVec){f}));
    EXPECT_FALSE(check_sortedness(Apply, (TermVec){f, p}));
    EXPECT_FALSE(check_sortedness(Apply, (TermVec){f, arr}));
  }

  /************** Quantifiers (if supported) ******************/
  if (solver_has_attribute(s->get_solver_enum(), QUANTIFIERS))
  {
    Term param = s->make_param("param", bvsort4);
    EXPECT_TRUE(param->is_param());
    Term body = s->make_term(Equal, param, s->make_term(0, bvsort4));
    // if bool and bv1 aliased, then body won't necessarily have type bool
    // e.g. BTOR also returns the type as BV1
    if (!solver_has_attribute(s->get_solver_enum(), BOOL_BV1_ALIASING))
    {
      EXPECT_TRUE(check_sortedness(Exists, (TermVec){param, body}));
    }
    // not a parameter
    EXPECT_FALSE(check_sortedness(Exists, (TermVec){q, body}));
    // not a formula for the body
    EXPECT_FALSE(check_sortedness(Exists, (TermVec){param, s->make_term(BVAdd, param, q)}));

    // bind param
    Term forallparam = s->make_term(Forall, param, body);
    EXPECT_TRUE(param->is_param());  // should still be considered a parameter
                                     // after binding
  }
}

TEST_P(UnitSortInferenceTests, SortComputation)
{
  /******** Booleans ********/
  EXPECT_EQ(boolsort, compute_sort(Equal, s, (TermVec){ b1, b2 }));
  EXPECT_EQ(boolsort, compute_sort(Equal, s, (SortVec){ boolsort, boolsort }));
  EXPECT_EQ(boolsort, compute_sort(Distinct, s, (SortVec){ boolsort, boolsort }));

  /******* Bitvectors *******/
  EXPECT_EQ(boolsort, compute_sort(Equal, s, (TermVec){ p, q }));
  EXPECT_EQ(boolsort, compute_sort(Equal, s, (SortVec){ bvsort4, bvsort4 }));
  EXPECT_EQ(boolsort, compute_sort(BVUlt, s, (TermVec){ p, q }));
  EXPECT_EQ(boolsort, compute_sort(BVUlt, s, (SortVec){ bvsort4, bvsort4 }));
  EXPECT_EQ(bvsort4, compute_sort(BVAdd, s, (TermVec){ p, q }));
  EXPECT_EQ(bvsort4, compute_sort(BVAdd, s, (SortVec){ bvsort4, bvsort4 }));
  EXPECT_EQ(bvsort4, compute_sort(BVNeg, s, (TermVec){ p }));
  EXPECT_EQ(bvsort4, compute_sort(BVNeg, s, (SortVec){ bvsort4 }));

  Sort bvsort3 = s->make_sort(BV, 3);
  Sort bvsort9 = s->make_sort(BV, 9);
  Sort bvsort12 = s->make_sort(BV, 12);
  EXPECT_EQ(bvsort3, compute_sort(Op(Extract, 2, 0), s, (TermVec){ p }));
  EXPECT_EQ(bvsort3, compute_sort(Op(Extract, 2, 0), s, (SortVec){ bvsort4 }));
  EXPECT_EQ(bvsort9, compute_sort(Concat, s, (TermVec){ p, w }));
  EXPECT_EQ(bvsort12, compute_sort(Op(Repeat, 3), s, (TermVec){ p }));

  EXPECT_NE(bvsort9, compute_sort(Concat, s, (SortVec){ bvsort4, bvsort4 }));

  // /********* Arrays ********/
  EXPECT_EQ(bvsort4, compute_sort(Select, s, (TermVec){ arr, p }));
  EXPECT_EQ(bvsort4, compute_sort(Select, s, (SortVec){ arrsort, bvsort4 }));
  EXPECT_EQ(arrsort, compute_sort(Store, s, (SortVec){ arrsort, bvsort4, bvsort4 }));
  EXPECT_EQ(arrsort, compute_sort(Store, s, (TermVec){ arr, p, q }));

  // BTOR doesn't support getting the sort of a function yet
  if (s->get_solver_enum() != BTOR)
  {
    /********* Functions ********/
    EXPECT_EQ(boolsort, compute_sort(Apply, s, { f, p, q }));
    EXPECT_EQ(boolsort, compute_sort(Apply, s, { funsort, bvsort4, bvsort4 }));
  }
}

TEST_P(UnitArithmeticSortInferenceTests, ArithmeticSortedness)
{
  EXPECT_TRUE(check_sortedness(Gt, (TermVec){x, y}));
  EXPECT_TRUE(check_sortedness(Ge, (TermVec){xint, yint}));
  EXPECT_TRUE(check_sortedness(Lt, (TermVec){x, y}));
  EXPECT_TRUE(check_sortedness(Le, (TermVec){xint, yint}));

  EXPECT_TRUE(check_sortedness(Plus, (TermVec){x, y}));
  EXPECT_TRUE(check_sortedness(Plus, (TermVec){xint, yint}));
  EXPECT_TRUE(check_sortedness(Minus, (TermVec){x, y}));
  EXPECT_TRUE(check_sortedness(Minus, (TermVec){xint, yint}));
  EXPECT_TRUE(check_sortedness(Negate, (TermVec){xint}));

  EXPECT_TRUE(check_sortedness(To_Int, (TermVec){x}));
  EXPECT_TRUE(check_sortedness(To_Real, (TermVec){xint}));

  // wrong operators
  EXPECT_FALSE(check_sortedness(To_Real, (TermVec){x}));
  EXPECT_FALSE(check_sortedness(To_Int, (TermVec){xint}));
  EXPECT_FALSE(check_sortedness(BVUgt, (TermVec){x, y}));
  EXPECT_FALSE(check_sortedness(BVSgt, (TermVec){xint, yint}));
  EXPECT_FALSE(check_sortedness(BVUlt, (TermVec){x, y}));
  EXPECT_FALSE(check_sortedness(BVSge, (TermVec){xint, yint}));
  EXPECT_FALSE(check_sortedness(BVAdd, (TermVec){xint, yint}));

  // wrong number of arguments
  EXPECT_FALSE(check_sortedness(Negate, (TermVec){xint, yint}));
}

TEST_P(UnitArithmeticSortInferenceTests, ArithmeticSortComputation)
{
  EXPECT_EQ(boolsort, compute_sort(Equal, s, (TermVec){ x, y }));
  EXPECT_EQ(boolsort, compute_sort(Equal, s, (SortVec){ realsort, realsort }));

  EXPECT_EQ(boolsort, compute_sort(Ge, s, (TermVec){ x, y }));
  EXPECT_EQ(boolsort, compute_sort(Le, s, (SortVec){ realsort, realsort }));

  EXPECT_EQ(realsort, compute_sort(Plus, s, (TermVec){ x, y }));
  EXPECT_EQ(realsort, compute_sort(Minus, s, (SortVec){ realsort, realsort }));

  EXPECT_EQ(intsort, compute_sort(Plus, s, (TermVec){ xint, yint }));
  EXPECT_EQ(intsort, compute_sort(Minus, s, (SortVec){ intsort, intsort }));

  Sort aritharrsort = s->make_sort(ARRAY, intsort, realsort);
  Term aritharr = s->make_symbol("aritharr", aritharrsort);

  // /********* Arrays ********/
  EXPECT_EQ(realsort, compute_sort(Select, s, (TermVec){ aritharr, yint }));
  EXPECT_EQ(realsort, compute_sort(Select, s, (SortVec){ aritharrsort, intsort }));
  EXPECT_EQ(aritharrsort,
            compute_sort(Store, s, (SortVec){ aritharrsort, intsort, realsort }));
  EXPECT_EQ(aritharrsort, compute_sort(Store, s, (TermVec){ aritharr, xint, y }));
}

INSTANTIATE_TEST_SUITE_P(ParameterizedUnitSortInference,
                         UnitSortInferenceTests,
                         testing::ValuesIn(available_solver_configurations()));

INSTANTIATE_TEST_SUITE_P(ParameterizedUnitArithmeticSortInference,
                         UnitArithmeticSortInferenceTests,
                         testing::ValuesIn(filter_solver_configurations({ THEORY_INT, THEORY_REAL })));

}  // namespace smt_tests
