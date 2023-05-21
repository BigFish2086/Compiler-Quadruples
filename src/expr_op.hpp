#pragma once
#include "globals.hpp"

struct adder {
  Value operator()(const int &a, const int &b) const { return a + b; }
  Value operator()(const int &a, const float &b) const { return a + b; }
  Value operator()(const float &a, const int &b) const { return a + b; }
  Value operator()(const float &a, const float &b) const { return a + b; }

  Value operator()(const std::string &a, const std::string &b) const {
    return a + b;
  }

  Value operator()(auto a, auto b) const { error("invalid types for add"); }
};

struct subtractor {
  Value operator()(const int &a, const int &b) const { return a - b; }
  Value operator()(const int &a, const float &b) const { return a - b; }
  Value operator()(const float &a, const int &b) const { return a - b; }
  Value operator()(const float &a, const float &b) const { return a - b; }

  Value operator()(auto a, auto b) const { error("invalid types for add"); }
};

struct multplier {
  Value operator()(const int &a, const int &b) const { return a * b; }
  Value operator()(const int &a, const float &b) const { return a * b; }
  Value operator()(const float &a, const int &b) const { return a * b; }
  Value operator()(const float &a, const float &b) const { return a * b; }

  Value operator()(auto a, auto b) const { error("invalid types for mul"); }
};

struct divider {
  Value operator()(const int &a, const int &b) const { return a / b; }
  Value operator()(const int &a, const float &b) const { return a / b; }
  Value operator()(const float &a, const int &b) const { return a / b; }
  Value operator()(const float &a, const float &b) const { return a / b; }

  Value operator()(auto a, auto b) const { error("invalid types for div"); }
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

  bool operator()(auto a, auto b) const { error("invalid types for equal"); }
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
    error("invalid types for less than");
  }
};

// ----------------------------------------------------------------------
struct log_and {
  bool operator()(const bool &a, const bool &b) const { return a && b; }
  bool operator()(const int &a, const int &b) const { return a && b; }

  bool operator()(auto a, auto b) const {
    error("invalid types for logical and");
  }
};

struct log_not {
  bool operator()(const bool &a) const { return !a; }
  bool operator()(const int &a) const { return !a; }

  bool operator()(auto a) const { error("invalid types for logical not"); }
};

// ----------------------------------------------------------------------
struct type_op {
  yytokentype operator()(const int &a) const { return INTEGER; }
  yytokentype operator()(const float &a) const { return FLOAT; }
  yytokentype operator()(const std::string &a) const { return STRING; }
  yytokentype operator()(const bool &a) const { return BOOL; }

  yytokentype operator()(auto a) const { error("not supported type"); }
};

struct repr_op {
  template <typename T, std::enable_if_t<std::is_arithmetic_v<T>> * = nullptr>
  std::string operator()(const T &t) const {
    return std::to_string(t);
  }
  std::string operator()(const std::string &t) const { return t; }
  std::string operator()(const bool &t) const { return t ? "true" : "false"; }
};

struct printer {
  template <typename T> void operator()(const T &t) const { cout << t; }
};
