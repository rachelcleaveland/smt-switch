/*********************                                                        */
/*! \file term.h
** \verbatim
** Top contributors (to current version):
**   Makai Mann, Clark Barrett
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Abstract interface for SMT terms.
**
**
**/

#pragma once

#include <iostream>
#include <iterator>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "assert.h"
#include "ops.h"
#include "smt_defs.h"
#include "sort.h"

namespace smt {

class TermIter;

// Abstract class for term
class AbsTerm
{
 public:
  AbsTerm(){};
  virtual ~AbsTerm(){};
  /** Returns a hash for this term */
  virtual std::size_t hash() const = 0;
  /** Returns a unique id for this term */
  virtual std::size_t get_id() const = 0;
  /* Should return true iff the terms are identical */
  virtual bool compare(const Term& absterm) const = 0;
  // Term methods
  /* get the Op used to create this term */
  virtual Op get_op() const = 0;
  /* get the sort */
  virtual Sort get_sort() const = 0;
  /* to_string in smt2 format */
  virtual std::string to_string() = 0;
  /* returns true iff this term is a symbol */
  virtual bool is_symbol() const = 0;
  /* returns true iff this term is a parameter (to be bound by a quantifier) */
  virtual bool is_param() const = 0;
  /* returns true iff this term is a symbolic constant */
  virtual bool is_symbolic_const() const = 0;
  /* returns true iff this term is an interpreted constant */
  virtual bool is_value() const = 0;
  /** converts a constant that can be represented as an int to an int
   *  otherwise, throws an IncorrectUsageException
   */
  virtual uint64_t to_int() const = 0;
  /** begin iterator
   *  starts iteration through Term's children
   */
  virtual TermIter begin() = 0;
  /** end iterator
   *  ends iteration through Term's children
   */
  virtual TermIter end() = 0;

  // Methods used for strange edge-cases e.g. in the logging solver

  /** Print a value term in a specific form
   *  NOTE: this *only* exists for use in LoggingSolver
   *        it is to handle printing of values from solvers that alias
   *        sorts. For example, if Bool and (_ BitVec 1) are aliased,
   *        this can be used to print #b1 as true.
   *
   *  This method canNOT be used to convert arbitrarily, e.g.
   *  it cannot print a bitvector as an integer.
   *
   *  Thus, solvers that don't alias sorts can just use their to_string
   *  to implement this method
   *
   *  @param sk the SortKind to print the term as
   *  @param a string representation of the term
   *
   *  throws an exception if the term is not a value
   */
  virtual std::string print_value_as(SortKind sk) = 0;
};

/**
 * This is a TermValue.
 */
template <typename T>
class TermValue
{

  friend class RachelsSharedPtr<AbsTerm>;
  friend class RefCountGuard;

  /* ------------------------------------------------------------------------ */
 public:
  /* ------------------------------------------------------------------------ */

  using nv_iterator = TermValue**;
  using const_nv_iterator = TermValue const* const*;

  TermValue(T t) : d_term(t), d_rc(1) {}

  /**
   * Constructor for GenericTerm.
   */ 
  TermValue(Sort sort, Op op, std::vector<Term> t, std::string name) 
    : d_term(new T(sort,op,t,name)), d_rc(1) {}

  /**
   * Constructor for GenericTerm.
   */ 
  TermValue(Sort sort, Op op, std::vector<Term> t, std::string name, bool b) 
    : d_term(new T(sort,op,t,name,b)), d_rc(1) {}

/*
  template <class T>
  class iterator
  {
   public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T;

    iterator() : d_i(NULL) {}
    explicit iterator(const_nv_iterator i) : d_i(i) {}

    T operator*() const { return T(*d_i); }

    bool operator==(const iterator& i) const { return d_i == i.d_i; }

    bool operator!=(const iterator& i) const { return d_i != i.d_i; }

    iterator& operator++()
    {
      ++d_i;
      return *this;
    }

    iterator operator++(int) { return iterator(d_i++); }

    iterator& operator--()
    {
      --d_i;
      return *this;
    }

    iterator operator--(int) { return iterator(d_i--); }

    iterator& operator+=(difference_type p)
    {
      d_i += p;
      return *this;
    }

    iterator& operator-=(difference_type p)
    {
      d_i -= p;
      return *this;
    }

    iterator operator+(difference_type p) { return iterator(d_i + p); }

    iterator operator-(difference_type p) { return iterator(d_i - p); }

    difference_type operator-(iterator i) { return d_i - i.d_i; }

   private:
    const_nv_iterator d_i;

  }; /* class TermValue::iterator<T> 
*/
  /* ------------------------------ Header ---------------------------------- */
  /** Number of bits reserved for reference counting. */
  static constexpr uint32_t NBITS_REFCOUNT = 20;

  /* ------------------- This header fits into 96 bits ---------------------- */

  uint32_t getRefCount() const { return d_rc; }

  //template <typename T>
  //inline iterator<T> begin() const;
  //template <typename T>
  //inline iterator<T> end() const;

  /* ------------------------------------------------------------------------ */
 private:
  /* ------------------------------------------------------------------------ */

  /**
   * RAII guard that increases the reference count if the reference count to be
   * > 0.  Otherwise, this does nothing. This does not just increment the
   * reference count to avoid maxing out the d_rc field. This is only for low
   * level functions.
   */
  class RefCountGuard
  {
   public:
    RefCountGuard(const TermValue* nv) : d_nv(const_cast<TermValue*>(nv))
    {
      d_increased = (d_nv->d_rc == 0);
      if (d_increased)
      {
        d_nv->d_rc = 1;
      }
    }
    ~RefCountGuard()
    {
      // dec() without marking for deletion: we don't want to garbage
      // collect this TermValue if ours is the last reference to it.
      // E.g., this can happen when debugging code calls the print
      // routines below.  As RefCountGuards are scoped on the stack,
      // this should be fine---but not in multithreaded contexts!
      if (d_increased)
      {
        --d_nv->d_rc;
      }
    }

   private:
    TermValue* d_nv;
    bool d_increased;
  }; /* TermValue::RefCountGuard */

  /** Maximum reference count possible.  Used for sticky
   *  reference-counting.  Should be (1 << num_bits(d_rc)) - 1 */
  static constexpr uint32_t MAX_RC =
      (static_cast<uint32_t>(1) << NBITS_REFCOUNT) - 1;


  /** Uninitializing constructor for NodeBuilder's use.  */
  TermValue()
  { /* do not initialize! */
  }
  /** Private constructor for the null value. */
  TermValue(int);

  /**
   * Destructor. 
   */
  ~TermValue() {
    dec();
    if (d_rc == 0) {
      delete(d_term);
    }
  }

  void inc()
  {
    assert(!isBeingDeleted());
        //<< "TermValue is currently being deleted "
        //   "and increment is being called on it. Don't Do That!";
    // FIXME multithreading
    if (__builtin_expect((d_rc < MAX_RC - 1), true))
    {
      ++d_rc;
    }
    else if (__builtin_expect((d_rc == MAX_RC - 1), false))
    {
      ++d_rc;
      markRefCountMaxedOut();
    }
  }

  void dec()
  {
    // FIXME multithreading
    if (__builtin_expect((d_rc < MAX_RC), true))
    {
      --d_rc;
      if (__builtin_expect((d_rc == 0), false))
      {
        markForDeletion();
      }
    }
  }

  void markRefCountMaxedOut();
  void markForDeletion();

  bool isBeingDeleted() const;

  /** Returns true if the reference count is maximized. */
  inline bool HasMaximizedReferenceCount() { return d_rc == MAX_RC; }

  nv_iterator nv_begin();
  nv_iterator nv_end();

  const_nv_iterator nv_begin() const;
  const_nv_iterator nv_end() const;

  /** The expression */
  T *d_term;

  /** The expression's reference count. */
  uint32_t d_rc : NBITS_REFCOUNT;

}; /* class TermValue */



template <typename T>
class RachelsSharedPtr {

  /** The referenced TermValue */
  TermValue<T>* d_nv;

public:

 /** 
  * Default constructor, makes a null expression.
  */
 RachelsSharedPtr() : d_nv(nullptr) {}

 /**
  * Constructor taking an AbsTerm. 
  */
 RachelsSharedPtr(T term) : d_nv(new TermValue<T>(term)) {}

 /**
  * Copy constructor.
  * @param term the term to make copy of
  */
 RachelsSharedPtr(const RachelsSharedPtr& term) : d_nv(term.d_nv) {}

  /**
   * Constructor for GenericTerm
   */
  RachelsSharedPtr(Sort sort, Op op, std::vector<Term> t, std::string name)
   : d_nv(new TermValue<T>(sort,op,t,name)) {}

  /**
   * Constructor for GenericTerm
   */
  RachelsSharedPtr(Sort sort, Op op, std::vector<Term> t, std::string name, bool b)
   : d_nv(new TermValue<T>(sort,op,t,name,b)) {}

  /**
    * Destructor. 
    */
  ~RachelsSharedPtr() {
    d_nv->~TermValue();
  }

 /**
  * Assignment operator for terms, copies the relevant information from term
  * to this term.
  * @param term the term to copy
  * @return reference to this term
  */
 RachelsSharedPtr& operator=(const RachelsSharedPtr& term) {
  assert (d_nv != NULL); // << "Expecting a non-NULL expression value!";
  assert (term.d_nv != NULL); // << "Expecting a non-NULL expression value on RHS!";
  if(__builtin_expect( ( d_nv != term.d_nv ), true )) {
    assert (d_nv->d_rc > 0); // << "term reference count would be negative";
    d_nv->dec();
    d_nv = term.d_nv;

    assert (d_nv->d_rc > 0); // << "term assigned from term with rc == 0";
    d_nv->inc();

  }
  return *this;
 }

 /**
  * Casts the TermValue pointed to by d_nv.
  */
 template <typename T_n>
 RachelsSharedPtr& cast_shared_pointer() {
  d_nv->d_term = static_cast<T_n>(d_nv->d_term);
 }

  /**
   * Dereferencing operators.
   */
  T *operator->() const {
    return d_nv->d_term;
  }

  T &operator*() const {
    return *d_nv->d_term;
  }

  T *get() const {
    return d_nv->d_term;
  }

  /**
   * Returns the i-th element.
   * @param i the index of the child
   * @return the i-th term
   */
  T &operator[](int i) const {
    return d_nv->d_term[i];
  }


};/* class RachelsSharedPtr */

/*
RachelsSharedPtr& RachelsSharedPtr::
operator=(const RachelsSharedPtr& e) {
  assert (d_nv != NULL); // << "Expecting a non-NULL expression value!";
  assert (e.d_nv != NULL); // << "Expecting a non-NULL expression value on RHS!";
  if(__builtin_expect( ( d_nv != e.d_nv ), true )) {
    assert (d_nv->d_rc > 0); // << "term reference count would be negative";
    d_nv->dec();
    d_nv = e.d_nv;

    assert (d_nv->d_rc > 0); // << "term assigned from term with rc == 0";
    d_nv->inc();

  }
  return *this;
}*/

inline bool operator==(const Term & t1, const Term & t2)
{
  return t1->compare(t2);
}
inline bool operator!=(const Term & t1, const Term & t2)
{
  return !t1->compare(t2);
}
inline bool operator<(const Term & t1, const Term & t2)
{
  return t1->get_id() < t2->get_id();
}
inline bool operator>(const Term & t1, const Term & t2)
{
  return t1->get_id() > t2->get_id();
}
inline bool operator<=(const Term & t1, const Term & t2)
{
  return t1->get_id() <= t2->get_id();
}
inline bool operator>=(const Term & t1, const Term & t2)
{
  return t1->get_id() >= t2->get_id();
}

std::ostream& operator<<(std::ostream& output, const Term t);

// term iterators
// impelementation based on
// https://www.codeproject.com/Articles/92671/How-to-write-abstract-iterators-in-Cplusplus
class TermIterBase
{
 public:
  TermIterBase() {}
  virtual ~TermIterBase() {}
  virtual void operator++() {}
  const virtual Term operator*();
  virtual TermIterBase * clone() const = 0;
  bool operator==(const TermIterBase& other) const;

 protected:
  virtual bool equal(const TermIterBase & other) const = 0;
};

class TermIter
{
 public:
  // typedefs for marking as an input iterator
  // based on iterator_traits: https://en.cppreference.com/w/cpp/iterator/iterator_traits
  // used by the compiler for statements such as: TermVec children(term->begin(), term->end())
  typedef Term value_type;
  typedef std::ptrdiff_t difference_type;
  typedef Term * pointer;
  typedef Term & reference;
  typedef std::input_iterator_tag iterator_category;

  TermIter() : iter_(0) {}
  TermIter(TermIterBase* tib) : iter_(tib) {}
  ~TermIter() { delete iter_; }
  TermIter(const TermIter& other) : iter_(other.iter_->clone()) {}
  TermIter& operator=(const TermIter& other);
  TermIter& operator++();
  TermIter operator++(int junk);
  Term operator*() const { return *(*iter_); }
  bool operator==(const TermIter& other) const;
  bool operator!=(const TermIter& other) const;

 protected:
  TermIterBase* iter_;
};

// useful typedefs for data structures
using TermVec = std::vector<Term>;
using TermList = std::list<Term>;
using UnorderedTermSet = std::unordered_set<Term>;
using UnorderedTermMap = std::unordered_map<Term, Term>;
// range-based iteration
inline TermIter begin(Term & t) { return t->begin(); }
inline TermIter end(Term & t) { return t->end(); }

}  // namespace smt

namespace std
{
  // Specialize the hash function for data structures
  template<>
  struct hash<smt::Term>
  {
    size_t operator()(const smt::Term & t) const
    {
      return t->hash();
    }
  };
}

