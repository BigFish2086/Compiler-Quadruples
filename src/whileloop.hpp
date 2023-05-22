#pragma once

#include "expr.hpp"
#include "globals.hpp"

struct WhileStmt {
  int scope, line;
  shared_ptr<Expr> expr;
  string condition_body;
  string code_block_body;

  WhileStmt(
    const string &_condition_body,
    const string &_code_block_body,
    shared_ptr<Expr> _expr
  ) {
    this->check(_expr);
    this->expr = _expr;
    this->condition_body = _condition_body;
    this->code_block_body = _code_block_body;

    this->scope = current_scope;
    this->line = yylineno;
  }
  ~WhileStmt() {}

  void check(shared_ptr<Expr> _expr) {
    if (!canCast(_expr->type(), yytokentype::BOOL)) {
      error("while condition must be type convertable to boolean");
    }
    if (_expr->type() == yytokentype::BOOL) {
      string cond = std::get<bool>(_expr->value) ? "true" : "false";
      warning("while condition is always " + cond);
    }
  }

  string repr() {
    string res = "";

    int &cnt = scopeLabels[this->scope];
    string initLabel = buildLable(this->scope, cnt++);
    string retLabel = buildLable(this->scope, cnt++);

    res += label(initLabel);
    res += this->condition_body;
    res += jz(retLabel);
    res += this->code_block_body;
    res += jmp(initLabel);
    res += label(retLabel);

    scopeLabels[this->scope] += 3;
    return res;
  }
};
