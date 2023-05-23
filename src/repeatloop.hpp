#pragma once

#include "expr.hpp"
#include "globals.hpp"

struct RepeatStmt {
  int scope, line;
  shared_ptr<Expr> expr;
  string condition_body;
  string code_block_body;

  RepeatStmt(
    const string &_condition_body,
    const string &_code_block_body,
    shared_ptr<Expr> _expr
  ) {
    this->scope = current_scope;
    this->line = yylineno;
    this->check(_expr);
    this->expr = _expr;
    this->condition_body = _condition_body;
    this->code_block_body = _code_block_body;
  }
  ~RepeatStmt() {}

  void check(shared_ptr<Expr> _expr) {
    if (!canCast(_expr->type(), yytokentype::BOOL)) {
      error("repeat loop  at L# " + to_string(this->line) + " condition must be type convertable to boolean");
    }
    if (_expr->type() == yytokentype::BOOL) {
      string cond = std::get<bool>(_expr->value) ? "true" : "false";
      warning("repeat loop at L# " + to_string(this->line) + " condition is always " + cond);
    }
  }

  string repr() {
    string res = "";

    int &cnt = scopeLabels[this->scope];
    string initLabel = buildLable(this->scope, cnt++);

    res += label(initLabel);
    res += this->code_block_body;
    res += this->condition_body;
    res += jnz(initLabel);

    scopeLabels[this->scope] += 3;
    return res;
  }
};

