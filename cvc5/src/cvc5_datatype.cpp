#include "cvc5_datatype.h"
namespace smt {

DatatypeDecl make_shared_datatype_decl(cvc5::DatatypeDecl d) {
  Cvc5DatatypeDecl *cvct = new Cvc5DatatypeDecl(d);
  AbsDatatypeDecl *abst = dynamic_cast<AbsDatatypeDecl *>(cvct);
  return RachelsSharedPtr<AbsDatatypeDecl>(abst);
}

DatatypeConstructorDecl make_shared_datatype_constructor(cvc5::DatatypeConstructorDecl d) {
  Cvc5DatatypeConstructorDecl *cvct = new Cvc5DatatypeConstructorDecl(d);
  AbsDatatypeConstructorDecl *abst = dynamic_cast<AbsDatatypeConstructorDecl *>(cvct);
  return RachelsSharedPtr<AbsDatatypeConstructorDecl>(abst);
}

Datatype make_shared_datatype(cvc5::Datatype d) {
  Cvc5Datatype *cvct = new Cvc5Datatype(d);
  AbsDatatype *abst = dynamic_cast<AbsDatatype *>(cvct);
  return RachelsSharedPtr<AbsDatatype>(abst);
}


bool Cvc5DatatypeConstructorDecl::compare(
    const DatatypeConstructorDecl & d) const
{
  Cvc5DatatypeConstructorDecl *cd =
      dynamic_cast<Cvc5DatatypeConstructorDecl*>(d.get());
  return datatypeconstructordecl.toString()
         == cd->datatypeconstructordecl.toString();
}

}  // namespace smt
