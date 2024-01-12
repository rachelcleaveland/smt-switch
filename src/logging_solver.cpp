/*********************                                                        */
/*! \file logging_solver.cpp
** \verbatim
** Top contributors (to current version):
**   Makai Mann, Clark Barrett
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Class that wraps another SmtSolver and tracks the term DAG by
**        wrapping sorts and terms and performs hash-consing.
**
**/

#include "assert.h"

#include "logging_solver.h"
#include "logging_sort.h"
#include "logging_term.h"
#include "sort_inference.h"

#include "utils.h"

using namespace std;

namespace smt {

/* These are the only sortkinds that are supported for get_value
   Terms returned by get_value were not created through the
   smt-switch API, so the LoggingSolver needs to recover some
   information. Most SortKinds are not an issue because they
   have no Op or children
 */
const unordered_set<SortKind> supported_sortkinds_for_get_value(
    { BOOL, BV, INT, STRING, REAL, ARRAY });

/* LoggingSolver */

/**
 * Create a new shared pointer. 
 */
Term make_shared_term(Term t, Sort s, Op op, TermVec tv, size_t id) { 
  LoggingTerm *lt = new LoggingTerm(t,s,op,tv,id);
  return RachelsSharedPtr<AbsTerm>(lt);
}

Term make_shared_term(Term t, Sort s, Op o, TermVec tv, std::string name, bool b, size_t id) {
  LoggingTerm *lt = new LoggingTerm(t,s,o,tv,name,b,id);
  return RachelsSharedPtr<AbsTerm>(lt);
}

SmtSolver create_logging_solver(SmtSolver solver) {
  LoggingSolver *ls = new LoggingSolver(solver);
  AbsSmtSolver *abss = dynamic_cast<AbsSmtSolver*>(ls);
  return RachelsSharedPtr<AbsSmtSolver>(abss);
}

// implementations

LoggingSolver::LoggingSolver(SmtSolver s)
    : AbsSmtSolver(s->get_solver_enum()),
      wrapped_solver(s),
      hashtable(new TermHashTable()),
      assumption_cache(new UnorderedTermMap()),
      next_term_id(0)
{
}

LoggingSolver::~LoggingSolver() {}

Sort LoggingSolver::make_sort(const string name, uint64_t arity) const
{
  Sort wrapped_sort = wrapped_solver->make_sort(name, arity);
  return make_uninterpreted_logging_sort(wrapped_sort, name, arity);
}

Sort LoggingSolver::make_sort(const SortKind sk) const
{
  Sort sort = wrapped_solver->make_sort(sk);
  return make_logging_sort(sk, sort);
}

Sort LoggingSolver::make_sort(const SortKind sk, uint64_t size) const
{
  Sort sort = wrapped_solver->make_sort(sk, size);
  return make_logging_sort(sk, sort, size);
}

Sort LoggingSolver::make_sort(const SortKind sk, const Sort & sort1) const
{
  LoggingSort *ls1 = dynamic_cast<LoggingSort*>(sort1.get());
  Sort sort = wrapped_solver->make_sort(sk, ls1->wrapped_sort);
  return make_logging_sort(sk, sort, sort1);
}

Sort LoggingSolver::make_sort(const SortKind sk,
                              const Sort & sort1,
                              const Sort & sort2) const
{
  LoggingSort *ls1 = dynamic_cast<LoggingSort*>(sort1.get());
  LoggingSort *ls2 = dynamic_cast<LoggingSort*>(sort2.get());
  Sort sort =
      wrapped_solver->make_sort(sk, ls1->wrapped_sort, ls2->wrapped_sort);
  return make_logging_sort(sk, sort, sort1, sort2);
}

Sort LoggingSolver::make_sort(const SortKind sk,
                              const Sort & sort1,
                              const Sort & sort2,
                              const Sort & sort3) const
{
  LoggingSort *ls1 = dynamic_cast<LoggingSort*>(sort1.get());

  LoggingSort *ls2 = dynamic_cast<LoggingSort*>(sort2.get());
  LoggingSort *ls3 = dynamic_cast<LoggingSort*>(sort3.get());
  Sort sort = wrapped_solver->make_sort(
      sk, ls1->wrapped_sort, ls2->wrapped_sort, ls3->wrapped_sort);
  return make_logging_sort(sk, sort, sort1, sort2, sort3);
}

Sort LoggingSolver::make_sort(SortKind sk, const SortVec & sorts) const
{
  // convert to sorts stored by LoggingSorts
  SortVec sub_sorts;
  for (auto s : sorts)
  {
    sub_sorts.push_back(dynamic_cast<LoggingSort*>(s.get())->wrapped_sort);
  }
  Sort sort = wrapped_solver->make_sort(sk, sub_sorts);
  return make_logging_sort(sk, sort, sorts);
}

Sort LoggingSolver::make_sort(const Sort & sort_con,
                              const SortVec & sorts) const
{
  Sort sub_sort_con = dynamic_cast<LoggingSort*>(sort_con.get())->wrapped_sort;

  // convert to sorts stored by LoggingSorts
  SortVec sub_sorts;
  for (auto s : sorts)
  {
    sub_sorts.push_back(dynamic_cast<LoggingSort*>(s.get())->wrapped_sort);
  }

  Sort ressort = wrapped_solver->make_sort(sub_sort_con, sub_sorts);
  return make_uninterpreted_logging_sort(ressort,
                                         sort_con->get_uninterpreted_name(),
                                         sorts);
}

Sort LoggingSolver::make_sort(const DatatypeDecl & d) const {
  throw NotImplementedException("LoggingSolver::make_sort");
};
DatatypeDecl LoggingSolver::make_datatype_decl(const std::string & s)  {
    throw NotImplementedException("LoggingSolver::make_datatype_decl");
}
DatatypeConstructorDecl LoggingSolver::make_datatype_constructor_decl(
    const std::string s)
{
  throw NotImplementedException(
      "LoggingSolver::make_datatype_constructor_decl");
};
void LoggingSolver::add_constructor(DatatypeDecl & dt, const DatatypeConstructorDecl & con) const {
  throw NotImplementedException("LoggingSolver::add_constructor");
};
void LoggingSolver::add_selector(DatatypeConstructorDecl & dt, const std::string & name, const Sort & s) const {
  throw NotImplementedException("LoggingSolver::add_selector");
};
void LoggingSolver::add_selector_self(DatatypeConstructorDecl & dt, const std::string & name) const {
  throw NotImplementedException("LoggingSolver::add_selector_self");
};

Term LoggingSolver::get_constructor(const Sort & s, std::string name) const  {
  throw NotImplementedException("LoggingSolver::get_constructor");
};
Term LoggingSolver::get_tester(const Sort & s, std::string name) const  {
  throw NotImplementedException("LoggingSolver::get_testeer");
};

Term LoggingSolver::get_selector(const Sort & s, std::string con, std::string name) const  {
  throw NotImplementedException("LoggingSolver::get_selector");
};


Term LoggingSolver::make_term(bool b) const
{
  Term wrapped_res = wrapped_solver->make_term(b);
  Sort boolsort = make_logging_sort(BOOL, wrapped_res->get_sort());
  Term res = make_shared_term(
      wrapped_res, boolsort, Op(), TermVec{}, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroys the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::make_term(int64_t i, const Sort & sort) const
{
  LoggingSort *lsort = dynamic_cast<LoggingSort*>(sort.get());
  Term wrapped_res = wrapped_solver->make_term(i, lsort->wrapped_sort);
  Term res = make_shared_term(
      wrapped_res, sort, Op(), TermVec{}, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::make_term(const std::string& s, bool useEscSequences, const Sort & sort) const
{
  LoggingSort *lsort = dynamic_cast<LoggingSort*>(sort.get());
  Term wrapped_res = wrapped_solver->make_term(s, useEscSequences, lsort->wrapped_sort);
  Term res = make_shared_term(
      wrapped_res, sort, Op(), TermVec{}, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::make_term(const std::wstring& s, const Sort & sort) const{
  LoggingSort *lsort = dynamic_cast<LoggingSort*>(sort.get());
  Term wrapped_res = wrapped_solver->make_term(s, lsort->wrapped_sort);
  Term res = make_shared_term(
      wrapped_res, sort, Op(), TermVec{}, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::make_term(const string name,
                              const Sort & sort,
                              uint64_t base) const
{
  LoggingSort *lsort = dynamic_cast<LoggingSort*>(sort.get());
  Term wrapped_res = wrapped_solver->make_term(name, lsort->wrapped_sort, base);
  Term res = make_shared_term(
      wrapped_res, sort, Op(), TermVec{}, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::make_term(const Term & val, const Sort & sort) const
{
  LoggingTerm *lval = dynamic_cast<LoggingTerm *>(val.get());
  LoggingSort *lsort = dynamic_cast<LoggingSort*>(sort.get());
  Term wrapped_res =
      wrapped_solver->make_term(lval->wrapped_term, lsort->wrapped_sort);
  // this make_term is for constant arrays
  if (sort->get_sort_kind() != ARRAY)
  {
    throw IncorrectUsageException(
        "make_term(Term, Sort) is for creating constant arrays.\nExpecting "
        "array sort but got: "
        + sort->to_string());
  }
  // the constant value must be the child
  Term res = make_shared_term(
      wrapped_res, sort, Op(), TermVec{ val }, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::make_symbol(const string name, const Sort & sort)
{
  LoggingSort *lsort = dynamic_cast<LoggingSort*>(sort.get());
  Term wrapped_sym = wrapped_solver->make_symbol(name, lsort->wrapped_sort);
  // bool true means it's a symbol
  Term res = make_shared_term(
      wrapped_sym, sort, Op(), TermVec{}, name, true, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  symbol_table[name] = res;

  return res;
}

Term LoggingSolver::get_symbol(const std::string & name)
{
  auto it = symbol_table.find(name);
  if (it == symbol_table.end())
  {
    throw IncorrectUsageException("Symbol named " + name + " does not exist.");
  }
  return it->second;
}

Term LoggingSolver::make_param(const string name, const Sort & sort)
{
  LoggingSort *lsort = dynamic_cast<LoggingSort*>(sort.get());
  Term wrapped_param = wrapped_solver->make_param(name, lsort->wrapped_sort);
  // bool false means it's not a symbol
  Term res = make_shared_term(
      wrapped_param, sort, Op(), TermVec{}, name, false, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::make_term(const Op op, const Term & t) const
{
  LoggingTerm *lt = dynamic_cast<LoggingTerm *>(t.get());
  Term wrapped_res = wrapped_solver->make_term(op, lt->wrapped_term);
  SortVec sorts = { t->get_sort() };
  Sort res_logging_sort = compute_sort(op, this, sorts);

  // check that child is already in hash table
  assert(hashtable->contains(t));

  Term res = make_shared_term(
      wrapped_res, res_logging_sort, op, TermVec{ t }, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::make_term(const Op op,
                              const Term & t1,
                              const Term & t2) const
{
  LoggingTerm *lt1 = dynamic_cast<LoggingTerm *>(t1.get());
  LoggingTerm *lt2 = dynamic_cast<LoggingTerm *>(t2.get());
  Term wrapped_res =
      wrapped_solver->make_term(op, lt1->wrapped_term, lt2->wrapped_term);
  Sort res_logging_sort =
      compute_sort(op, this, { t1->get_sort(), t2->get_sort() });

  // check that children are already in hash table
  assert(hashtable->contains(t1));
  assert(hashtable->contains(t2));

  Term res = make_shared_term(
      wrapped_res, res_logging_sort, op, TermVec({ t1, t2 }), next_term_id);
  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::make_term(const Op op,
                              const Term & t1,
                              const Term & t2,
                              const Term & t3) const
{
  LoggingTerm *lt1 = dynamic_cast<LoggingTerm *>(t1.get());
  LoggingTerm *lt2 = dynamic_cast<LoggingTerm *>(t2.get());
  LoggingTerm *lt3 = dynamic_cast<LoggingTerm *>(t3.get());
  Term wrapped_res = wrapped_solver->make_term(
      op, lt1->wrapped_term, lt2->wrapped_term, lt3->wrapped_term);
  Sort res_logging_sort = compute_sort(
      op, this, { t1->get_sort(), t2->get_sort(), t3->get_sort() });

  // check that children are already in hash table
  assert(hashtable->contains(t1));
  assert(hashtable->contains(t2));
  assert(hashtable->contains(t3));

  Term res = make_shared_term(
      wrapped_res, res_logging_sort, op, TermVec{ t1, t2, t3 }, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::make_term(const Op op, const TermVec & terms) const
{
  TermVec lterms;
  for (auto tt : terms)
  {
    LoggingTerm *ltt = dynamic_cast<LoggingTerm *>(tt.get());
    lterms.push_back(ltt->wrapped_term);

    // check that children are already in the hash table
    assert(hashtable->contains(tt));
  }
  Term wrapped_res = wrapped_solver->make_term(op, lterms);
  // Note: for convenience there's a version of compute_sort that takes terms
  // since these are already in a vector, just let it unpack the sorts
  Sort res_logging_sort = compute_sort(op, this, terms);
  Term res = make_shared_term(
      wrapped_res, res_logging_sort, op, terms, next_term_id);

  // check hash table
  // lookup modifies term in place and returns true if it's a known term
  // i.e. returns existing term and destroying the unnecessary new one
  if (!hashtable->lookup(res))
  {
    // this is the first time this term was created
    hashtable->insert(res);
    next_term_id++;
  }

  return res;
}

Term LoggingSolver::get_value(const Term & t) const
{
  Term res;
  SortKind sk = t->get_sort()->get_sort_kind();
  if (supported_sortkinds_for_get_value.find(sk)
      == supported_sortkinds_for_get_value.end())
  {
    throw NotImplementedException(
        "LoggingSolver does not support get_value for " + smt::to_string(sk));
  }

  LoggingTerm *lt = dynamic_cast<LoggingTerm *>(t.get());
  if (t->get_sort()->get_sort_kind() != ARRAY)
  {
    Term wrapped_val = wrapped_solver->get_value(lt->wrapped_term);
    res = make_shared_term(
        wrapped_val, t->get_sort(), Op(), TermVec{}, next_term_id);

    // check hash table
    // lookup modifies term in place and returns true if it's a known term
    // i.e. returns existing term and destroying the unnecessary new one
    if (!hashtable->lookup(res))
    {
      // this is the first time this term was created
      hashtable->insert(res);
      next_term_id++;
    }
  }
  else
  {
    Term out_const_base;
    UnorderedTermMap pairs = get_array_values(t, out_const_base);
    if (!out_const_base)
    {
      throw InternalSolverException(
          "Wrapped solver did not provide constant base. Please use "
          "get_array_values instead of get_value of an array");
    }
    res = make_term(out_const_base, t->get_sort());
    for (auto elem : pairs)
    {
      res = make_term(Store, res, elem.first, elem.second);
    }
  }

  return res;
}

void LoggingSolver::get_assertions(TermVec & out) 
{
  wrapped_solver->get_assertions(out);
}

void LoggingSolver::get_unsat_assumptions(UnorderedTermSet & out)
{
  UnorderedTermSet underlying_core;
  wrapped_solver->get_unsat_assumptions(underlying_core);
  for (auto c : underlying_core)
  {
    // assumption: these should be (possible negated) Boolean literals
    // that were used in check_sat_assuming
    // assumption_cache stored a mapping so we can recover the logging term
    if (assumption_cache->find(c) == assumption_cache->end())
    {
      throw InternalSolverException(
          "Got an element in the unsat core that was not cached from "
          "check_sat_assuming in LoggingSolver.");
    }
    Term log_c = assumption_cache->at(c);
    out.insert(log_c);
  }
}

UnorderedTermMap LoggingSolver::get_array_values(const Term & arr,
                                                 Term & out_const_base) const
{
  Sort arrsort = arr->get_sort();
  Sort idxsort = arrsort->get_indexsort();
  Sort elemsort = arrsort->get_elemsort();
  LoggingTerm *larr = dynamic_cast<LoggingTerm *>(arr.get());
  UnorderedTermMap assignments;
  Term wrapped_out_const_base;
  UnorderedTermMap wrapped_assignments = wrapped_solver->get_array_values(
      larr->wrapped_term, wrapped_out_const_base);
  if (wrapped_out_const_base)
  {
    if (wrapped_out_const_base->get_sort()->get_sort_kind() == ARRAY)
    {
      throw NotImplementedException(
          "const base for multidimensional array not implemented in "
          "LoggingSolver");
    }
    out_const_base = make_shared_term(
        wrapped_out_const_base, elemsort, Op(), TermVec{}, next_term_id);
    // check hash table
    // lookup modifies term in place and returns true if it's a known term
    // i.e. returns existing term and destroys the unnecessary new one
    if (!hashtable->lookup(out_const_base))
    {
      // this is the first time this term was created
      hashtable->insert(out_const_base);
      next_term_id++;
    }
  }

  Term idx;
  Term val;
  for (auto elem : wrapped_assignments)
  {
    // expecting values in assignment map
    Assert(elem.first->is_value());
    Assert(elem.second->is_value());

    idx = make_shared_term(
        elem.first, idxsort, Op(), TermVec{}, next_term_id);
    if (!hashtable->lookup(idx))
    {
      // this is the first time this term was created
      hashtable->insert(idx);
      next_term_id++;
    }

    val = make_shared_term(
        elem.second, elemsort, Op(), TermVec{}, next_term_id);
    if (!hashtable->lookup(val))
    {
      // this is the first time this term was created
      hashtable->insert(val);
      next_term_id++;
    }

    assignments[idx] = val;
  }

  return assignments;
}

void LoggingSolver::reset()
{
  wrapped_solver->reset();
  hashtable->clear();
}

// dispatched to underlying solver

void LoggingSolver::set_opt(const std::string option, const std::string value)
{
  wrapped_solver->set_opt(option, value);
}

void LoggingSolver::set_logic(const std::string logic)
{
  wrapped_solver->set_logic(logic);
}

void LoggingSolver::assert_formula(const Term & t)
{
  LoggingTerm *lt = dynamic_cast<LoggingTerm*>(t.get());
  wrapped_solver->assert_formula(lt->wrapped_term);
}

Result LoggingSolver::check_sat() { return wrapped_solver->check_sat(); }

Result LoggingSolver::check_sat_assuming(const TermVec & assumptions)
{
  // only needs to remember the latest set of assumptions
  assumption_cache->clear();
  TermVec lassumps;
  LoggingTerm *la;
  for (auto a : assumptions)
  {
    la = dynamic_cast<LoggingTerm *>(a.get());
    lassumps.push_back(la->wrapped_term);
    // store a mapping from the wrapped term to the logging term
    (*assumption_cache)[la->wrapped_term] = a;
  }
  return wrapped_solver->check_sat_assuming(lassumps);
}

Result LoggingSolver::check_sat_assuming_list(const TermList & assumptions)
{
  // only needs to remember the latest set of assumptions
  assumption_cache->clear();
  TermList lassumps;
  LoggingTerm *la;
  for (auto a : assumptions)
  {
    la = dynamic_cast<LoggingTerm *>(a.get());
    lassumps.push_back(la->wrapped_term);
    // store a mapping from the wrapped term to the logging term
    (*assumption_cache)[la->wrapped_term] = a;
  }
  return wrapped_solver->check_sat_assuming_list(lassumps);
}

Result LoggingSolver::check_sat_assuming_set(
    const UnorderedTermSet & assumptions)
{
  // only needs to remember the latest set of assumptions
  assumption_cache->clear();
  UnorderedTermSet lassumps;
  LoggingTerm *la;
  for (auto a : assumptions)
  {
    la = dynamic_cast<LoggingTerm *>(a.get());
    lassumps.insert(la->wrapped_term);
    // store a mapping from the wrapped term to the logging term
    (*assumption_cache)[la->wrapped_term] = a;
  }
  return wrapped_solver->check_sat_assuming_set(lassumps);
}

void LoggingSolver::push(uint64_t num) { wrapped_solver->push(num); }

void LoggingSolver::pop(uint64_t num) { wrapped_solver->pop(num); }

uint64_t LoggingSolver::get_context_level() const
{
  return wrapped_solver->get_context_level();
}

void LoggingSolver::reset_assertions() { wrapped_solver->reset_assertions(); }

}  // namespace smt
