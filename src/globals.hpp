#pragma once

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <unordered_set>

#include "../parser.tab.h"
#include "../quads.hpp"

using namespace std;

using Value = std::variant<int, float, std::string, bool>;
map<int, int> scopeLabels;

extern int yylex();
extern int yylineno;
extern char *yytext;

int current_scope = 0;
int current_label = 0;
int syntax_errors = 0;

// Output files.
string fout;
ofstream symlog;
ofstream outputFile;

// type `key` can be treated as any of its `values`
map<int, vector<int>> typeCast = {
  {INTEGER,   {INTEGER, FLOAT, BOOL}},
  {FLOAT,     {INTEGER, FLOAT, BOOL}},
  {BOOL,      {INTEGER, FLOAT, BOOL}},
  {STRING,    {STRING}},
  {ENUM_TYPE, {ENUM_TYPE, INTEGER}}
};

map<int, string> type2Str = {
  {INTEGER,   "INTEGER"},
  {FLOAT,     "FLOAT"},
  {BOOL,      "BOOL"},
  {STRING,    "STRING"},
  {ENUM_TYPE, "ENUM_TYPE"}
};

// map types to default values
map<int, Value> type2Default = {
  {INTEGER,   Value(0)},
  {FLOAT,     Value(0.0f)},
  {BOOL,      Value(false)},
  {STRING,    Value("")},
  {ENUM_TYPE, Value(0)}
};

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

string buildLable(int scp, int cnt) {
  return " s" + to_string(scp) + "_l" + to_string(cnt);
}

struct GStmt {
  string representation = "";
  GStmt() = default;
  GStmt(const string &repr) : representation(repr) {}
  ~GStmt() = default;

  string repr() const { return representation; }
  void setRepr(const string &repr) { representation = repr; }
  GStmt *append(const string &repr) {
    representation += repr;
    return this;
  }
};
