#pragma once
#include "expr_op.hpp"
#include "globals.hpp"

struct Expr {
  Value value;
  bool isConst;
  Expr() : value(0), isConst(true) {}
  Expr(Value v) : value(v), isConst(true) {}
  Expr(Value v, bool ct) : value(v), isConst(ct) {}
  virtual ~Expr() {}

  bool constResult(const Expr *other) const {
    return isConst && other->isConst;
  }

  // overload type operator
  yytokentype type() const { return std::visit(type_op(), value); }

  // overload assignment operator
  Expr &operator=(const Expr *other) {
    value = other->value;
    isConst = other->isConst;
    return *this;
  }

  // overload arithmatic operators
  Expr *operator-() const {
    return new Expr(std::visit(multplier(), value, Value(-1)), isConst);
  }
  Expr *operator+(const Expr *other) const {
    return new Expr(std::visit(adder(), value, other->value),
                    this->constResult(other));
  }
  Expr *operator-(const Expr *other) const {
    return new Expr(std::visit(subtractor(), value, other->value),
                    this->constResult(other));
  }
  Expr *operator*(const Expr *other) const {
    return new Expr(std::visit(multplier(), value, other->value),
                    this->constResult(other));
  }
  Expr *operator/(const Expr *other) const {
    return new Expr(std::visit(divider(), value, other->value),
                    this->constResult(other));
  }

  // overload comparison operators
  bool operator==(const Expr *other) const {
    return std::visit(equlizer(), value, other->value);
  }
  bool operator!=(const Expr *other) const { return !(*this == other); }
  bool operator<(const Expr *other) const {
    return std::visit(less_than(), value, other->value);
  }
  bool operator>(const Expr *other) const { return other < this; }
  bool operator<=(const Expr *other) const { return !(*this > other); }
  bool operator>=(const Expr *other) const { return !(*this < other); }

  // overload logical operators
  bool operator!() const { return std::visit(log_not(), value); }
  bool operator&&(const Expr *other) const {
    return std::visit(log_and(), value, other->value);
  }
  bool operator||(const Expr *other) const { return !(!(*this) && !other); }

  // overload stream operator
  friend ostream &operator<<(ostream &os, const Expr *expr) {
    std::visit(printer(), expr->value);
    return os;
  }

  string repr() const { return std::visit(repr_op(), value); }
};

struct EnumExpr : public Expr{
  string enumName;

  EnumExpr(const string &_enumName, int v) : Expr(v), enumName(_enumName) {}

};

struct ExprStmt {
  Expr *expr;
  string reprsentation;
  ExprStmt(Expr *e) : expr(e) {}
  ExprStmt(Expr *e, const string &str) : expr(e), reprsentation(str) {}
  ~ExprStmt() { delete expr; }

  void setRepr(const string &str) { reprsentation = str; }
  string repr() const { return reprsentation; }

  Expr *getExpr() const { return expr; }
  yytokentype type() const { return expr->type(); }
};
