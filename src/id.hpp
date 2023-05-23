#pragma once

#include "expr.hpp"
#include "globals.hpp"

class ID {
public:
  string name;
  int line;
  int scope;
  bool isUsed;
  yytokentype type;

  ID(yytokentype _type, const string &_name) {
    this->line = yylineno;
    this->scope = current_scope;
    this->type = _type;
    this->name = _name;
    this->isUsed = false;
  }
  virtual ~ID() {}

  virtual string str() const {
    return "N: " + this->name + " L# " + to_string(this->line) + " S@ " +
           to_string(this->scope) + " U: " + to_string(this->isUsed) +
           " T: " + type2Str[this->type];
  }

  virtual vector<string> vstr() const {
    return {"N: " + this->name, "L# " + to_string(this->line),
            "S@ " + to_string(this->scope), "U: " + to_string(this->isUsed),
            "T: " + type2Str[this->type]};
  }

  friend ostream &operator<<(ostream &os, shared_ptr<ID> id) {
    os << id->str();
    return os;
  }
};

// ----------------------------------------------------------------------
class VarID : public ID {
public:
  shared_ptr<Expr> expr;
  bool isConst = false;
  bool isInitialized = false;

  bool isEnum = false;
  string enumName = ""; // if the variable is an enumVariant

  // for variables
  VarID(yytokentype _type, const string &_name, bool isInit = false)
      : ID(_type, _name) {
    this->isInitialized = isInit;
  }

  // for const variables
  VarID(yytokentype _type, const string &_name, shared_ptr<Expr> _expr)
      : ID(_type, _name) {
    this->__setExpr(_expr);
    this->isConst = true;
    // this->expr->isConst = true;
  }

  // for enum variants
  VarID(const string &_name, const string &_enumName) : ID(ENUM_TYPE, _name) {
    this->isEnum = true;
    this->enumName = _enumName;
  }

  // for const enum variants
  VarID(const string &_name, const string &_enumName, shared_ptr<Expr> _expr)
      : ID(ENUM_TYPE, _name) {
    this->isEnum = true;
    this->enumName = _enumName;
    this->__setExpr(_expr);
    this->isConst = true;
    // this->expr->isConst = true;
  }

  ~VarID() {}

  string str() const override {
    string parent = ID::str();
    string me = parent + " I: " + to_string(this->isInitialized) +
           " C: " + to_string(this->isConst);

    if(this->expr != nullptr) {
      me += " Expr: " + this->expr->repr();
    }
    if (this->isEnum) {
      me += " EnumType: " + this->enumName;
    } 
    return me;
  }

  vector<string> vstr() const override {
    vector<string> parent = ID::vstr();
    vector<string> me = parent;
    me.push_back("@VarID");
    me.push_back("I: " + to_string(this->isInitialized));
    me.push_back("C: " + to_string(this->isConst));
    if(this->expr != nullptr) {
      me.push_back("Expr: " + this->expr->repr());
    }
    if (this->isEnum) {
      me.push_back("EnumType: " + this->enumName);
    }
    return me;
  }

  // TODO: call doCast() when needed
  // checks are:
  // 1. if the variable is const, then it cannot be assigned again
  // 2. type of the variable and the expression must match or be convertible
  void setExpr(shared_ptr<Expr> _expr) {
    if (this->isConst) {
      error("cannot assign to const variable " + this->name);
    }
    this->__setExpr(_expr);
    // this->expr->isConst = this->isConst;
  }

  shared_ptr<Expr> getExpr() {
    if (!this->isInitialized) {
      error("variable " + this->name + " is not initialized");
    }
    return this->expr;
  }

private:
  void __setExpr(shared_ptr<Expr> _expr) {
    if (!canCast(this->type, _expr->type())) {
      error("cannot assign Variable " + this->name + " of type " +
            type2Str[this->type] + " to expression of type " +
            type2Str[_expr->type()]);
    }

    if (this->isEnum) {
      shared_ptr<EnumExpr> enumExpr = dynamic_pointer_cast<EnumExpr>(_expr);
      if (enumExpr == nullptr) {
        error("cannot assign non-enum expression to enum variable " +
              this->name);
      }
      if (this->enumName != enumExpr->enumName) {
        error("cannot assign enum expression of enum " + enumExpr->enumName +
              " to enum variable " + this->name + " of enum " + this->enumName);
      }
    }

    this->expr = _expr;
    this->isInitialized = true;
  }
};

// ----------------------------------------------------------------------
struct TypedList {
  vector<yytokentype> list;
  string reprsentation = "";
  TypedList() {}
  TypedList(yytokentype item, const string &str) { this->append(item, str); }
  TypedList *append(yytokentype item, const string &str) {
    list.push_back(item);
    reprsentation += str;
    return this;
  }
  string repr() const { return reprsentation; }
  int size() const { return list.size(); }
};

class FuncID : public ID {
public:
  TypedList *funcParamsTypes;

  FuncID(yytokentype _type, const string &_name, TypedList *_paramsTypes)
      : ID(_type, _name) {
    this->funcParamsTypes = _paramsTypes;
  }
  ~FuncID() {
    delete this->funcParamsTypes;
    this->funcParamsTypes = nullptr;
  }

  string str() const override {
    string parent = ID::str();
    string me = parent + " @FuncID";
    return me;
  }

  vector<string> vstr() const override {
    vector<string> parent = ID::vstr();
    vector<string> me = parent;
    me.push_back("@FuncID");
    return me;
  }

};

// ----------------------------------------------------------------------
struct StrList {
  vector<string> list;
  StrList() {}
  StrList(string item) { this->append(item); }
  StrList *append(string item) {
    list.push_back(item);
    return this;
  }
  int size() const { return list.size(); }
};

class EnumID : public ID {
public:
  StrList *enumVariants;

  EnumID(string _name, StrList *_variants) : ID(ENUM_TYPE, _name) {
    this->enumVariants = _variants;
  }
  ~EnumID() {
    delete this->enumVariants;
    this->enumVariants = nullptr;
  }

  int getVariant(const string &variant) {
    for (int i = 0; i < (int)this->enumVariants->size(); i++) {
      if (this->enumVariants->list[i] == variant) {
        return i;
      }
    }
    error("enum " + this->name + " has no variant " + variant);
    return -1;
  }

  string str() const override {
    string parent = ID::str();
    string me = parent + " @EnumID";
    return me;
  }
  
  vector<string> vstr() const override {
    vector<string> parent = ID::vstr();
    vector<string> me = parent;
    me.push_back("@EnumID");
    return me;
  }
};
