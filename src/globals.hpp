#pragma once

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

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
map<int, vector<int>> typeCast = {{INTEGER, {INTEGER, FLOAT, BOOL}},
                                  {FLOAT, {INTEGER, FLOAT, BOOL}},
                                  {BOOL, {INTEGER, FLOAT, BOOL}},
                                  {STRING, {STRING}},
                                  {ENUM_TYPE, {ENUM_TYPE, INTEGER}}};

map<int, string> type2Str = {{INTEGER, "INTEGER"},
                             {FLOAT, "FLOAT"},
                             {BOOL, "BOOL"},
                             {STRING, "STRING"},
                             {ENUM_TYPE, "ENUM_TYPE"}};

// map types to default values
map<int, Value> type2Default = {{INTEGER, Value(0)},
                                {FLOAT, Value(0.0f)},
                                {BOOL, Value(false)},
                                {STRING, Value("")},
                                {ENUM_TYPE, Value(0)}};

// key is pushed first, value.key is pushed second after casting
map<int, map<int, string>> e2id = {
    {INTEGER, {{FLOAT, "int2float\n"}, {BOOL, "int2bool\n"}}},
    {FLOAT, {{INTEGER, "float2int\n"}, {BOOL, "float2bool\n"}}},
    {BOOL, {{INTEGER, "bool2int\n"}, {FLOAT, "bool2float\n"}}}};


// key is pushed first, value.key is pushed second then cast value.key to key
map<int, map<int, string>> e2e = {
  {INTEGER, {{FLOAT, "pop tmp\nint2float\npush tmp\n"}, {BOOL, "bool2int\n"}}},
  {FLOAT,  {{INTEGER, "int2float\n"}, {BOOL, "bool2float\n"}}},
  {BOOL,  {{INTEGER, "pop tmp\nbool2int\npush tmp\n"}, {FLOAT, "pop tmp\nbool2float\npush tmp\n"}}}
};

// map type two its precedence (bigger number means higher pressedence)
map<int, int> type2Pre = {{INTEGER, 1}, {FLOAT, 2}, {BOOL, 3}};

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

// means expression is pushed first and then id later
string e2idCast(const int &etype, const int &idtype) {
  if (etype == idtype) {
    return "";
  }
  if (!canCast(etype, idtype)) {
    error("Cannot cast from " + type2Str[etype] + " to " + type2Str[idtype]);
  }
  auto it = e2id.find(etype);
  if (it == e2id.end()) {
    return "";
  }
  auto it2 = it->second.find(idtype);
  if (it2 == it->second.end()) {
    return "";
  }
  return it2->second;
}

// expr OP expr, so cast should be done from smaller type to bigger
string e2eCast(const int &first, const int &second) {
  if (first == second) {
    return "";
  }
  if (!canCast(first, second)) {
    error("Cannot cast from " + type2Str[first] + " to " + type2Str[second]);
  }
  auto it = e2e.find(first);
  if (it == e2e.end()) {
    return "";
  }
  auto it2 = it->second.find(second);
  if (it2 == it->second.end()) {
    return "";
  }
  return it2->second;
}

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
