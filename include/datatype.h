/*********************                                                        */
/*! \file datatype.h
** \verbatim
** Top contributors (to current version):
**   Kristopher Brown
** This file is part of the smt-switch project.
** Copyright (c) 2020 by the authors listed in the file AUTHORS
** in the top-level source directory) and their institutional affiliations.
** All rights reserved.  See the file LICENSE in the top-level source
** directory for licensing information.\endverbatim
**
** \brief Abstract interface for SMT datatypes.
**
**
**/

#pragma once

#include "assert.h"
#include "smt_defs.h"


namespace smt {

/**
 * This is a PtrValue.
 */
template <typename T>
class PtrValue
{

  friend class RachelsSharedPtr<T>;

 public:
  PtrValue() : d_ptr(nullptr), d_rc(1) {}

  PtrValue(T &t) : d_ptr(new T(t)), d_rc(1) {}

  PtrValue(T *t) : d_ptr(t), d_rc(1) {}

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
  ~PtrValue() {}

  void inc()
  {
    // FIXME multithreading
    if (__builtin_expect((d_rc < MAX_RC - 1), true))
    {
      ++d_rc;
    }
    else if (__builtin_expect((d_rc == MAX_RC - 1), false))
    {
      ++d_rc;
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

  void markForDeletion() {
    delete(d_ptr);
  }


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
   * Constructor taking a pointer to an AbsTerm.
   */
  RachelsSharedPtr(T *ptr) : d_nv(new PtrValue<T>(ptr)) {}

  /**
    * Constructor taking an AbsTerm. 
    */
  RachelsSharedPtr(T &ptr) : d_nv(new PtrValue<T>(ptr)) {}

  /**
   * Constructor used in casting function, taking a PtrValue.
   */
  RachelsSharedPtr(PtrValue<T>* tv) : d_nv(tv) {
    if (!d_nv) d_nv = new PtrValue<T>();
    d_nv->inc();
  }

  /**
    * Copy constructor.
    * @param ptr the ptr to make copy of
    */
  RachelsSharedPtr(const RachelsSharedPtr& ptr) : d_nv(ptr.d_nv) {
    d_nv->inc();
  }

  /**
    * Destructor. 
    */
  ~RachelsSharedPtr() {
    d_nv->dec();
  }

 /**
  * Assignment operator for ptrs, copies the relevant information from ptr
  * to this ptr.
  * @param ptr the ptr to copy
  * @return reference to this ptr
  */
  RachelsSharedPtr& operator=(const void * p) {
    assert (d_nv != NULL); // << "Expecting a non-NULL expression value!";
    assert (p == nullptr);

    d_nv->dec();
    d_nv = new PtrValue<T>();

    return *this;
  }

  RachelsSharedPtr& operator=(const RachelsSharedPtr& ptr) {
    assert (d_nv != NULL); // << "Expecting a non-NULL expression value!";
    assert (ptr.d_nv != NULL); // << "Expecting a non-NULL expression value on RHS!";
    if(__builtin_expect( ( d_nv != ptr.d_nv ), true )) {
      assert (d_nv->d_rc > 0); // << "ptr reference count would be negative";
      d_nv->dec();
      d_nv = ptr.d_nv;

      assert (d_nv->d_rc > 0); 
      d_nv->inc();

    }
    return *this;
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
    return d_nv->d_rc;
  }

  /**
   * From shared_ptr. Replaces the object managed by this shared pointer.
   */
  void reset(T *ptr) {
    d_nv->dec();
    d_nv = new PtrValue<T>(ptr);
  }

  /**
   * From shared_ptr. Removes this object as a manager of the pointer 
   * pointed to by d_nv->d_ptr.
   */
  void reset() {
    d_nv->dec();
    d_nv = new PtrValue<T>();
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



class AbsDatatypeDecl {

 public:
  AbsDatatypeDecl(){};
  virtual ~AbsDatatypeDecl(){};

};


class AbsDatatypeConstructorDecl {

 public:
  AbsDatatypeConstructorDecl(){};
  virtual ~AbsDatatypeConstructorDecl(){};
  virtual bool compare(const DatatypeConstructorDecl & d) const = 0;
};


class AbsDatatype {

 public:
  AbsDatatype(){};
  virtual ~AbsDatatype(){};
  virtual std::string get_name() const=0;
  virtual int get_num_selectors(std::string cons) const=0;
  virtual int get_num_constructors() const=0;
};
// Overloaded equivalence operators for two datatype constructor declarations
bool operator==(const DatatypeConstructorDecl & d1,
                const DatatypeConstructorDecl & d2);
bool operator!=(const DatatypeConstructorDecl & d1,
                const DatatypeConstructorDecl & d2);
}


namespace std
{
  // Specialize the hash function for data structures
  template<>
  struct hash<smt::Datatype>
  {
    size_t operator()(const smt::Datatype & t) const
    {
      return hash<void*>()((void*)(t.get()));
    }
  };
}