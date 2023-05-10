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

struct printer {
  template <typename T> void operator()(const T &t) const { cout << t; }
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

