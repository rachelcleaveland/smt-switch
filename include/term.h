/*********************                                                        */
/*! \file ptr.h
** \verbatim
** Top contributors (to current version):
**   Makai Mann, Clark Barrett
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Abstract interface for SMT ptrs.
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

// Abstract class for ptr
class AbsTerm
{
 public:
  AbsTerm(){};
  virtual ~AbsTerm(){};
  /** Returns a hash for this ptr */
  virtual std::size_t hash() const = 0;
  /** Returns a unique id for this ptr */
  virtual std::size_t get_id() const = 0;
  /* Should return true iff the ptrs are identical */
  virtual bool compare(const Term& absptr) const = 0;
  // Term methods
  /* get the Op used to create this ptr */
  virtual Op get_op() const = 0;
  /* get the sort */
  virtual Sort get_sort() const = 0;
  /* to_string in smt2 format */
  virtual std::string to_string() = 0;
  /* returns true iff this ptr is a symbol */
  virtual bool is_symbol() const = 0;
  /* returns true iff this ptr is a parameter (to be bound by a quantifier) */
  virtual bool is_param() const = 0;
  /* returns true iff this ptr is a symbolic constant */
  virtual bool is_symbolic_const() const = 0;
  /* returns true iff this ptr is an interpreted constant */
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

  /** Print a value ptr in a specific form
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
   *  @param sk the SortKind to print the ptr as
   *  @param a string representation of the ptr
   *
   *  throws an exception if the ptr is not a value
   */
  virtual std::string print_value_as(SortKind sk) = 0;
};


template <typename T>
class TestClass
{
  public:
  TestClass(T *t) : test(t) {}

  private:
  T *test;

};

/**
 * This is a PtrValue.
 */
template <typename T>
class PtrValue
{

  friend class RachelsSharedPtr<T>;

 public:
  PtrValue() : d_ptr(nullptr), d_rc(1) {}

  PtrValue(T t) : d_ptr(new T(t)), d_rc(1) {}

  /**
   * Constructor for GenericTerm.
   */ 
  PtrValue(Sort sort, Op op, std::vector<Term> t, std::string name) 
    : d_ptr(new T(sort,op,t,name)), d_rc(1) {}

  /**
   * Constructor for GenericTerm.
   */ 
  PtrValue(Sort sort, Op op, std::vector<Term> t, std::string name, bool b) 
    : d_ptr(new T(sort,op,t,name,b)), d_rc(1) {}

  /**
   * Constructor for LoggingTerm.
   */ 
  PtrValue(std::vector<Term>  t, Sort s, Op op, std::vector<Term> tv, size_t id)
   : d_ptr(new T(t[0],s,op,tv,id)), d_rc(1) {}

  /**
   * Constructor for LoggingTerm.
   */ 
  PtrValue(std::vector<Term>  t, Sort s, Op op, std::vector<Term> tv, std::string n, bool b, size_t id)
   : d_ptr(new T(t[0],s,op,tv,n,b,id)), d_rc(1) {}

  /* ------------------------------ Header ---------------------------------- */
  /** Number of bits reserved for reference counting. */
  static constexpr uint32_t NBITS_REFCOUNT = 20;

  /* ------------------- This header fits into 96 bits ---------------------- */

  uint32_t getRefCount() const { return d_rc; }

 private:

  /** Maximum reference count possible.  Used for sticky
   *  reference-counting.  Should be (1 << num_bits(d_rc)) - 1 */
  static constexpr uint32_t MAX_RC =
      (static_cast<uint32_t>(1) << NBITS_REFCOUNT) - 1;

  /** Private constructor for the null value. */
  PtrValue(int);

  /**
   * Destructor. 
   */
  //~PtrValue() {
  //  dec();
  //}

  void inc()
  {
    assert(!isBeingDeleted());
        //<< "PtrValue is currently being deleted "
        //   "and increment is being called on it. Don't Do That!";
    // FIXME multithreading
    if (__builtin_expect((d_rc < MAX_RC - 1), true))
    {
      ++d_rc;
    }
    else if (__builtin_expect((d_rc == MAX_RC - 1), false))
    {
      ++d_rc;
      //markRefCountMaxedOut();
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

  //void markRefCountMaxedOut();
  void markForDeletion() {
    //delete(d_ptr);
  }

  bool isBeingDeleted() const {
    return false;
  }

  /** Returns true if the reference count is maximized. */
  inline bool HasMaximizedReferenceCount() { return d_rc == MAX_RC; }

  /** The expression */
  T *d_ptr;

  /** The expression's reference count. */
  uint32_t d_rc : NBITS_REFCOUNT;

}; /* class PtrValue */



template <typename T>
class RachelsSharedPtr {

  friend class PtrValue<T>;

  /** The referenced PtrValue */
  PtrValue<T> *d_nv;

public:

  bool nullPtr() const {
    return !(d_nv->d_ptr);
  }

  /** 
    * Default constructor, makes a null expression.
    */
  RachelsSharedPtr() : d_nv(new PtrValue<T>()) {}

  /**
    * Constructor taking an AbsTerm. 
    */
  RachelsSharedPtr(T ptr) : d_nv(new PtrValue<T>(ptr)) {
    //std::cout << "Calling constructor for (T ptr) in RachelsSharedPointer" << std::endl;
    //std::cout << "  d_nv->d_ptr = " << reinterpret_cast<void *>(d_nv->d_ptr) << std::endl;
    //std::cout << "  d_rc = " << d_nv->d_rc << std::endl; 
  }

  /**
   * Constructor used in casting function, taking a PtrValue.
   */
  RachelsSharedPtr(PtrValue<T>* tv) : d_nv(tv) {
    //std::cout << "Calling constructor for (PtrValue<T>* tv) in RachelsSharedPointer" << std::endl;
    //std::cout << "  d_nv->d_ptr = " << reinterpret_cast<void *>(d_nv->d_ptr) << std::endl;
    //std::cout << "  d_rc = " << d_nv->d_rc << std::endl; 

    d_nv->inc();
  }

  /**
    * Copy constructor.
    * @param ptr the ptr to make copy of
    */
  RachelsSharedPtr(const RachelsSharedPtr& ptr) : d_nv(ptr.d_nv) {
    //std::cout << "Copy constructor called" << std::endl;
    d_nv->inc();
  }

  /**
   * Constructor for GenericTerm.
   */
  RachelsSharedPtr(Sort sort, Op op, std::vector<Term> t, std::string name)
   : d_nv(new PtrValue<T>(sort,op,t,name)) {}

  /**
   * Constructor for GenericTerm.
   */
  RachelsSharedPtr(Sort sort, Op op, std::vector<Term> t, std::string name, bool b)
   : d_nv(new PtrValue<T>(sort,op,t,name,b)) {}

  /**
   * Constructor for LoggingTerm.
   */
  RachelsSharedPtr(std::vector<Term>  t, Sort s, Op op, std::vector<Term> tv, size_t id)
   : d_nv(new PtrValue<T>(t,s,op,tv,id)) {}

  /**
   * Constructor for LoggingTerm.
   */
  RachelsSharedPtr(std::vector<Term>  t, Sort s, Op op, std::vector<Term> tv, std::string n, bool b, size_t id)
   : d_nv(new PtrValue<T>(t,s,op,tv,n,b,id)) {}

  /**
    * Destructor. 
    */
  ~RachelsSharedPtr() {
    d_nv->dec(); //->~PtrValue();

    //if (d_nv->d_rc == 0) {
    //  delete(d_nv);
    //}
  }

 /**
  * Assignment operator for ptrs, copies the relevant information from ptr
  * to this ptr.
  * @param ptr the ptr to copy
  * @return reference to this ptr
  */
  RachelsSharedPtr& operator=(const RachelsSharedPtr& ptr) {
    assert (d_nv != NULL); // << "Expecting a non-NULL expression value!";
    assert (ptr.d_nv != NULL); // << "Expecting a non-NULL expression value on RHS!";
    if(__builtin_expect( ( d_nv != ptr.d_nv ), true )) {
      assert (d_nv->d_rc > 0); // << "ptr reference count would be negative";
      d_nv->dec();
      d_nv = ptr.d_nv;

      //std::cout << "ptr.d_rc = " << ptr.d_nv->d_rc << std::endl;

      assert (d_nv->d_rc > 0); // << "ptr assigned from ptr with rc == 0";
      d_nv->inc();

    }
    return *this;
  }

 /**
  * Casts the PtrValue pointed to by d_nv.
  */
  template <typename T_n>
  RachelsSharedPtr<T_n> cast_shared_pointer() const {
    PtrValue<T_n>* nv = (PtrValue<T_n>*)(d_nv);
    return RachelsSharedPtr<T_n>(nv);
    //d_nv->d_ptr = static_cast<T_n*>(d_nv->d_ptr);
  }

  /**
   * Dereferencing operators.
   */
  T *operator->() const {
    return d_nv->d_ptr;
  }

  T &operator*() const {
    return *d_nv->d_ptr;
  }

  T *get() const {
    return d_nv->d_ptr;
  }

  /**
   * From shared_ptr. If *this owns a pointer, return the 
   * number of owners, otherwise zero.
   */
  long use_count() const {
    return 0;
  }

  /**
   * Logical operators
   */
  operator bool() const {
    return d_nv->d_ptr != 0;
  }

  bool operator!() const {
    return !d_nv->d_ptr;
  }

  /**
   * Returns the i-th element.
   * @param i the index of the child
   * @return the i-th ptr
   */
  T &operator[](int i) const {
    return d_nv->d_ptr[i];
  }


};/* class RachelsSharedPtr */

/**
 * Casts a Term to a specific type of shared pointer.
 */
template <typename T>
RachelsSharedPtr<T> cast_ptr(Term t) {
  return t.cast_shared_pointer<T>();
  //return std::static_pointer_cast<Cvc5Term>(t);

}

/*
RachelsSharedPtr& RachelsSharedPtr::
operator=(const RachelsSharedPtr& e) {
  assert (d_nv != NULL); // << "Expecting a non-NULL expression value!";
  assert (e.d_nv != NULL); // << "Expecting a non-NULL expression value on RHS!";
  if(__builtin_expect( ( d_nv != e.d_nv ), true )) {
    assert (d_nv->d_rc > 0); // << "ptr reference count would be negative";
    d_nv->dec();
    d_nv = e.d_nv;

    assert (d_nv->d_rc > 0); // << "ptr assigned from ptr with rc == 0";
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
  if (t1.nullPtr()) {
    return !t2.nullPtr();
  }
  if (t2.nullPtr()) {
    return !t1.nullPtr();
  }
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

// ptr iterators
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
  // used by the compiler for statements such as: TermVec children(ptr->begin(), ptr->end())
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

