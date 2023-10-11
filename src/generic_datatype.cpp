#include "generic_datatype.h"

#include <algorithm>
#include <exception>
#include <memory>
#include <unordered_map>

#include "assert.h"

using namespace std;

namespace smt {

DatatypeDecl make_shared_datatype_decl(const std::string & s) {
  GenericDatatypeDecl *gd = new GenericDatatypeDecl(s);
  AbsDatatypeDecl *absd = dynamic_cast<AbsDatatypeDecl*>(gd);
  return RachelsSharedPtr<AbsDatatypeDecl>(absd);
}

Datatype make_shared_datatype(DatatypeDecl dt) {
  GenericDatatype *gd = new GenericDatatype(dt);
  AbsDatatype *absd = dynamic_cast<AbsDatatype*>(gd);
  return RachelsSharedPtr<AbsDatatype>(absd);
}

DatatypeConstructorDecl make_shared_datatype_constructor(const std::string s) {
  GenericDatatypeConstructorDecl *gd = new GenericDatatypeConstructorDecl(s);
  AbsDatatypeConstructorDecl *absd = dynamic_cast<AbsDatatypeConstructorDecl*>(gd);
  return RachelsSharedPtr<AbsDatatypeConstructorDecl>(absd);
}


GenericDatatypeDecl::GenericDatatypeDecl(const std::string name) : dt_name(name)
{
}

std::string GenericDatatypeDecl::get_name() { return dt_name; }

GenericDatatypeConstructorDecl::GenericDatatypeConstructorDecl(
    const std::string & name)
    : cons_name(name)
{
}

void GenericDatatypeConstructorDecl::add_new_selector(
    const SelectorComponents & newSelector)
{
  for (unsigned int i = 0; i < selector_vector.size(); ++i)
  {
    // Checks if the selector has already been added
    if (selector_vector[i].name == (newSelector).name)
    {
      throw "Can't add selector. It already exists in this datatype!";
    }
  }
  selector_vector.push_back(newSelector);
}

std::vector<SelectorComponents>
GenericDatatypeConstructorDecl::get_selector_vector()
{
  return selector_vector;
}

std::string GenericDatatypeConstructorDecl::get_name() const
{
  return cons_name;
}

int GenericDatatypeConstructorDecl::get_selector_count() const
{
  return selector_vector.size();
}
bool GenericDatatypeConstructorDecl::compare(
    const DatatypeConstructorDecl & d) const
{
  // Compares based off constructor's name
  GenericDatatypeConstructorDecl *d_ = dynamic_cast<GenericDatatypeConstructorDecl*>(d.get());
  return cons_name
         == d_->get_name();
}

std::string GenericDatatypeConstructorDecl::get_dt_name() const
{
  GenericDatatypeDecl *dt_decl_ = dynamic_cast<GenericDatatypeDecl*>(dt_decl.get());
  return dt_decl_->get_name();
}

void GenericDatatypeConstructorDecl::update_stored_dt(
    const DatatypeDecl & datatype_decl)
{
  dt_decl = datatype_decl;
}

GenericDatatype::GenericDatatype(const DatatypeDecl & dt_declaration)
    : dt_decl(dt_declaration)
{
}

void GenericDatatype::add_constructor(
    const DatatypeConstructorDecl & dt_cons_decl)
{
  // Checks if dt_cons_decl is already associated with the datatype
  if (std::find(cons_decl_vector.begin(), cons_decl_vector.end(), dt_cons_decl)
      != cons_decl_vector.end())
  {
    throw "Can't add constructor. It already has been added!";
  }
  GenericDatatypeConstructorDecl *gdt_cons =
      dynamic_cast<GenericDatatypeConstructorDecl*>(dt_cons_decl.get());
  // Links the constructor to the datatype_decl of the datatype
  gdt_cons->update_stored_dt(dt_decl);
  // Links the datatype to the new constructor
  cons_decl_vector.push_back(dt_cons_decl);
}

void GenericDatatype::add_selector(const DatatypeConstructorDecl & dt_cons_decl,
                                   const SelectorComponents & newSelector)
{
  // Boolean used to keep track of if a successful match was found.
  bool success = false;
  for (unsigned int i = 0; i < cons_decl_vector.size(); ++i)
  {
    // If the constructor is associated with the datatype
    if (cons_decl_vector[i] == dt_cons_decl)
    {
      // Adds the selector to the correct constructor
      GenericDatatypeConstructorDecl *c_ = 
        dynamic_cast<GenericDatatypeConstructorDecl*>(cons_decl_vector[i].get());
      c_->add_new_selector(newSelector);
      success = true;
      break;
    }
  }
  if (!success)
  {
    throw InternalSolverException(
        "Can't add selector. The constructor is not a member of the datatype!");
  }
}
std::vector<DatatypeConstructorDecl> GenericDatatype::get_cons_vector()
{
  return cons_decl_vector;
}

std::string GenericDatatype::get_name() const
{
  GenericDatatypeDecl *dt_decl_ = dynamic_cast<GenericDatatypeDecl*>(dt_decl.get());
  return dt_decl_->get_name();
}

int GenericDatatype::get_num_constructors() const
{
  return cons_decl_vector.size();
}

int GenericDatatype::get_num_selectors(std::string cons) const
{
  // Used to keep track of the number of selectors in the constructor
  int num_selectors = 0;
  bool found = false;
  for (unsigned int i = 0; i < cons_decl_vector.size(); ++i)
  // Searches for a matching constructor
  {
    GenericDatatypeConstructorDecl *c_ = 
      dynamic_cast<GenericDatatypeConstructorDecl*>(cons_decl_vector[i].get());
    if (c_->get_name() == cons)
    {
      found = true;
      // Calls the constructor's get_selector_count() function
      GenericDatatypeConstructorDecl *cdv_ = 
        dynamic_cast<GenericDatatypeConstructorDecl*>(cons_decl_vector[i].get());
      num_selectors = cdv_->get_selector_count();
      break;
    }
  }
  if (!found)
  {
    throw InternalSolverException("Constructor not found");
  }
  return num_selectors;
}

/*
This function goes through every selector in the datatype and if
finalized is set to false, it replaces the previously stored sort
with new_sort
 */
void GenericDatatype::change_sort_of_selector(const Sort new_sort)
{
  // For every constructor
  for (unsigned int i = 0; i < cons_decl_vector.size(); ++i)
  {
    GenericDatatypeConstructorDecl *cons_cast = 
      dynamic_cast<GenericDatatypeConstructorDecl*>(cons_decl_vector[i].get());
    // For every selector
    for (unsigned int f = 0; f < get_num_selectors(cons_cast->get_name()); ++f)
    {
      if (cons_cast->selector_vector[f].finalized == false)
      {
        // Updates the selector's members
        cons_cast->selector_vector[f].sort = new_sort;
        cons_cast->selector_vector[f].finalized = true;
      }
    }
  }
}
}  // namespace smt
