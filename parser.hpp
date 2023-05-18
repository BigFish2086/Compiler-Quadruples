#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "y.tab.h"
using namespace std;

using Value = std::variant<int, float, std::string, bool>;

extern int yylex();
extern int yylineno;
extern char *yytext;

int current_scope = 0;

// Output files.
string fout;
ofstream symlog;
ofstream output;

// ----------------------------------------------------------------------
// Global TODO:
// - add more type conversions
// - also print the appropriate quadruples when converting types
// - handle the error better i.e change all those throw runtime_error
// - handle the assignment of enum types

// type `key` can be treated as any of its `values`
map<int, vector<int>> typeCast = {{INTEGER, {INTEGER, FLOAT, BOOL}},
                                  {FLOAT, {INTEGER, FLOAT, BOOL}},
                                  {STRING, {STRING}},
                                  {BOOL, {INTEGER, FLOAT, BOOL}}};

map<int, string> type2Str = {
    {INTEGER, "INTEGER"}, {FLOAT, "FLOAT"}, {STRING, "STRING"}, {BOOL, "BOOL"}};

bool canCast(const int &from, const int &to) {
  if (from == to) {
    return true;
  }
  auto it = typeCast.find(from);
  if (it == typeCast.end()) {
    return false;
  }
  auto it2 = std::find(it->second.begin(), it->second.end(), to);
  return it2 != it->second.end();
}

// TODO:
// - to do the actual type conversion
// - to print the appropriate quadruples when converting types
void doCast(const int &from, const int &to){};

// ----------------------------------------------------------------------
struct adder {
  Value operator()(const int &a, const int &b) const { return a + b; }
  Value operator()(const int &a, const float &b) const { return a + b; }
  Value operator()(const float &a, const int &b) const { return a + b; }
  Value operator()(const float &a, const float &b) const { return a + b; }

  Value operator()(const std::string &a, const std::string &b) const {
    return a + b;
  }

  Value operator()(auto a, auto b) const {
    throw std::runtime_error("invalid types for add");
  }
};

struct subtractor {
  Value operator()(const int &a, const int &b) const { return a - b; }
  Value operator()(const int &a, const float &b) const { return a - b; }
  Value operator()(const float &a, const int &b) const { return a - b; }
  Value operator()(const float &a, const float &b) const { return a - b; }

  Value operator()(auto a, auto b) const {
    throw std::runtime_error("invalid types for add");
  }
};

struct multplier {
  Value operator()(const int &a, const int &b) const { return a * b; }
  Value operator()(const int &a, const float &b) const { return a * b; }
  Value operator()(const float &a, const int &b) const { return a * b; }
  Value operator()(const float &a, const float &b) const { return a * b; }

  Value operator()(auto a, auto b) const {
    throw std::runtime_error("invalid types for mul");
  }
};

struct divider {
  Value operator()(const int &a, const int &b) const { return a / b; }
  Value operator()(const int &a, const float &b) const { return a / b; }
  Value operator()(const float &a, const int &b) const { return a / b; }
  Value operator()(const float &a, const float &b) const { return a / b; }

  Value operator()(auto a, auto b) const {
    throw std::runtime_error("invalid types for div");
  }
};

// ----------------------------------------------------------------------
// note: equal can work for == and !=
struct equlizer {
  template <typename T, typename U,
            enable_if_t<is_arithmetic_v<T> && is_arithmetic_v<U>>>
  bool operator()(const T &a, const U &b) const {
    return a == b;
  }

  bool operator()(const std::string &a, const std::string &b) const {
    return a == b;
  }

  bool operator()(auto a, auto b) const {
    throw std::runtime_error("invalid types for equal");
  }
};

// note: can work for <, >, <=, >=
struct less_than {
  template <typename T, typename U,
            enable_if_t<is_arithmetic_v<T> && is_arithmetic_v<U>>>
  bool operator()(const T &a, const U &b) const {
    return a < b;
  }

  bool operator()(const std::string &a, const std::string &b) const {
    return a < b;
  }

  bool operator()(auto a, auto b) const {
    throw std::runtime_error("invalid types for less than");
  }
};

// ----------------------------------------------------------------------
struct log_and {
  bool operator()(const bool &a, const bool &b) const { return a && b; }
  bool operator()(const int &a, const int &b) const { return a && b; }

  bool operator()(auto a, auto b) const {
    throw std::runtime_error("invalid types for logical and");
  }
};

struct log_not {
  bool operator()(const bool &a) const { return !a; }
  bool operator()(const int &a) const { return !a; }

  bool operator()(auto a) const {
    throw std::runtime_error("invalid types for logical not");
  }
};

// ----------------------------------------------------------------------
struct type_op {
  int operator()(const int &a) const { return INTEGER; }
  int operator()(const float &a) const { return FLOAT; }
  int operator()(const std::string &a) const { return STRING; }
  int operator()(const bool &a) const { return BOOL; }

  int operator()(auto a) const {
    throw std::runtime_error("not supported type");
  }
};

struct printer {
  template <typename T> void operator()(const T &t) const { cout << t; }
};

// ----------------------------------------------------------------------
struct Expr {
  Value value;
  bool isConst;
  Expr() : value(0), isConst(false) {}
  Expr(Value v) : value(v), isConst(false) {}
  Expr(Value v, bool ct) : value(v), isConst(ct) {}
  bool constResult(const Expr &other) const { return isConst && other.isConst; }

  // overload type operator
  int type() const { return std::visit(type_op(), value); }

  // overload assignment operator
  Expr &operator=(const Expr &other) {
    value = other.value;
    isConst = other.isConst;
    return *this;
  }

  // overload arithmatic operators
  Expr operator-() const {
    return Expr(std::visit(multplier(), value, Value(-1)), isConst);
  }
  Expr operator+(const Expr &other) const {
    return Expr(std::visit(adder(), value, other.value),
                this->constResult(other));
  }
  Expr operator-(const Expr &other) const {
    return Expr(std::visit(subtractor(), value, other.value),
                this->constResult(other));
  }
  Expr operator*(const Expr &other) const {
    return Expr(std::visit(multplier(), value, other.value),
                this->constResult(other));
  }
  Expr operator/(const Expr &other) const {
    return Expr(std::visit(divider(), value, other.value),
                this->constResult(other));
  }

  // overload comparison operators
  bool operator==(const Expr &other) const {
    return std::visit(equlizer(), value, other.value);
  }
  bool operator!=(const Expr &other) const { return !(*this == other); }
  bool operator<(const Expr &other) const {
    return std::visit(less_than(), value, other.value);
  }
  bool operator>(const Expr &other) const { return other < *this; }
  bool operator<=(const Expr &other) const { return !(*this > other); }
  bool operator>=(const Expr &other) const { return !(*this < other); }

  // overload logical operators
  bool operator!() const { return std::visit(log_not(), value); }
  bool operator&&(const Expr &other) const {
    return std::visit(log_and(), value, other.value);
  }
  bool operator||(const Expr &other) const { return !(!(*this) && !other); }

  // overload stream operator
  friend ostream &operator<<(ostream &os, const Expr &expr) {
    std::visit(printer(), expr.value);
    return os;
  }
};

class ID {
public:
  string name;
  int line;
  int scope;
  bool isUsed;

  ID(string _name) {
    this->name = _name;
    this->line = yylineno;
    this->scope = current_scope;
    this->isUsed = false;
  }
  ~ID() {}
};

// ----------------------------------------------------------------------
class VarID : public ID {
public:
  yytokentype type;
  Expr expr;
  bool isVar = false;
  bool isConst = false;
  bool isInitialized = false;

  // for variables
  VarID(yytokentype _type, string _name, bool _isInit = false) : ID(_name) {
    this->type = _type;
    this->isInitialized = _isInit;
  }
  // for const variables
  VarID(yytokentype _type, string _name, Expr *_expr) : ID(_name) {
    this->type = _type;
    this->isConst = true;
    this->isInitialized = true;
    this->__setExpr(_expr);
  }
  ~VarID() {}

  // TODO: call doCast() when needed
  // checks are:
  // 1. if the variable is const, then it cannot be assigned again
  // 2. type of the variable and the expression must match or be convertible
  void setExpr(Expr *_expr) {
    if (this->isConst) {
      throw std::runtime_error("cannot assign to const variable " + this->name);
    }
    this->__setExpr(_expr);
  }

  Expr *getExpr() {
    if (!this->isInitialized) {
      throw std::runtime_error("variable " + this->name +
                               " is not initialized");
    }
    return &this->expr;
  }

private:
  void __setExpr(Expr *_expr) {
    if (!canCast(this->type, _expr->type())) {
      throw std::runtime_error("cannot assign Variable " + this->name +
                               " of type " + type2Str[this->type] +
                               " to expression of type " +
                               type2Str[_expr->type()]);
    }
    this->expr = *_expr;
    this->isInitialized = true;
  }
};

// ----------------------------------------------------------------------
struct StrList {
  vector<string> list;
  StrList(string item) { this->append(item); }
  StrList *append(string item) {
    list.push_back(item);
    return this;
  }
};

struct TypedList {
  vector<yytokentype> list;
  TypedList(yytokentype item) { this->append(item); }
  TypedList *append(yytokentype item) {
    list.push_back(item);
    return this;
  }
};

class FuncID : public ID {
public:
  yytokentype type;
  vector<yytokentype> funcParamsTypes;

  FuncID(yytokentype _type, string _name, vector<yytokentype> _paramsTypes)
      : ID(_name) {
    this->type = _type;
    this->funcParamsTypes = _paramsTypes;
  }
  ~FuncID() {}
};

class EnumID : public ID {
public:
  vector<string> enumVariants;

  EnumID(string _name, vector<string> _variants) : ID(_name) {
    this->enumVariants = _variants;
  }
  ~EnumID() {}
};

class EnumVariantID : public ID {
public:
  string enumName;

  EnumVariantID(string _name, string _enumName) : ID(_name) {
    this->enumName = _enumName;
  }
  ~EnumVariantID() {}
};


