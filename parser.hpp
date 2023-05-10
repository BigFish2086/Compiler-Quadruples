#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>

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

// related to Value type
// TODO:
// - add more type conversions
// - also print the appropriate quadruples when converting types
// - handle the error better

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

struct Expr {
  Value value;
  bool isConst;
  Expr(Value v) : value(v), isConst(false) {}
  Expr(Value v, bool ct) : value(v), isConst(ct) {}
  bool constResult(const Expr &other) const { return isConst && other.isConst; }

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


};
