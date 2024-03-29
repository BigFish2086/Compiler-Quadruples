#pragma once

#include "expr.hpp"
#include "globals.hpp"

struct ForStmt {
  int scope, line;
  shared_ptr<Expr> expr;
  string optioanl_assign_body;
  string condition_body;
  string optional_increment_body;
  string code_block_body;
  string conv;

  ForStmt(const string &_optioanl_assign_body, const string &_condition_body,
          const string &_optional_increment_body,
          const string &_code_block_body, shared_ptr<Expr> _expr) {

    this->scope = current_scope;
    this->line = yylineno;

    this->check(_expr);
    this->expr = _expr;

    this->optioanl_assign_body = _optioanl_assign_body;
    this->condition_body = _condition_body;
    this->optional_increment_body = _optional_increment_body;
    this->code_block_body = _code_block_body;
  }

  ~ForStmt() {}

  void check(shared_ptr<Expr> _expr) {
    if (!canCast(_expr->type(), yytokentype::BOOL)) {
      error("for at L# " + to_string(this->line) + " condition must be of type convertable to boolean");
    }
    if (_expr->type() == yytokentype::BOOL) {
      string cond = std::get<bool>(_expr->value) ? "true" : "false";
      warning("for at L# " + to_string(this->line) + " condition is always " + cond);
    }
    this->conv = e2eCast(_expr->type(), yytokentype::BOOL);
  }

  string repr() {
    string res = "";

    int &cnt = scopeLabels[this->scope];
    string initLabel = buildLable(this->scope, cnt++);
    string retLabel = buildLable(this->scope, cnt++);

    res += this->optioanl_assign_body;
    res += label(initLabel);
    res += this->condition_body;
    res += this->conv;
    res += jz(retLabel);
    res += this->code_block_body;
    res += this->optional_increment_body;
    res += jmp(initLabel);
    res += label(retLabel);

    scopeLabels[this->scope] += 3;
    return res;
  }
};
