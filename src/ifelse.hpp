#pragma once

#include "expr.hpp"
#include "globals.hpp"

struct IFPart {
  shared_ptr<Expr> expr;
  int scope, line;
  yytokentype type;
  string condition_body;
  string code_block_body;
  string conv;

  IFPart(
    shared_ptr<Expr> _expr,
    const string &_condition_body,
    const string &_code_block_body
  ) {
    this->scope = current_scope;
    this->line = yylineno;
    this->check(_expr);
    this->expr = _expr;
    this->type = _expr->type();
    this->condition_body = _condition_body;
    this->code_block_body = _code_block_body;
  }

  // ELSE
  IFPart(const string &_body) {
    this->expr = nullptr;
    this->type = yytokentype::INTEGER;
    this->scope = current_scope;
    this->line = yylineno;
    this->condition_body = "";
    this->code_block_body = _body;
  }

  ~IFPart() {}

  void check(shared_ptr<Expr> _expr) {
    if (!canCast(_expr->type(), yytokentype::BOOL)) {
      error("if at L# " + to_string(this->line) + " condition must be of type convertable to boolean");
    }
    if (_expr->type() == yytokentype::BOOL) {
      string cond = std::get<bool>(_expr->value) ? "true" : "false";
      warning("if at L# " + to_string(this->line) + " condition is always " + cond);
    }
    this->conv = e2eCast(_expr->type(), yytokentype::BOOL);
  }

  string repr(const string &curLabel, const string &nextLabel,
              const string &retLabel) {
    string res = "";
    res += label(curLabel);
    if (expr != nullptr) {
      res += this->condition_body;
      res += this->conv;
      res += jz(nextLabel);
    }
    res += this->code_block_body;
    res += jmp(retLabel);
    return res;
  }
};

struct IFPartList {
  vector<IFPart *> list;
  IFPartList() {}
  IFPartList(const IFPart *item) { this->append(item); }
  IFPartList *append(const IFPart *item) {
    list.push_back((IFPart *)item);
    return this;
  }
  int size() { return list.size(); }
  ~IFPartList() {
    for (auto item : list) {
      delete item;
      item = 0;
    }
    this->list.clear();
  }
};

struct IFStmt {
  IFPartList *ifPartList = nullptr;
  IFPart *elsePart = nullptr;
  int scope, line;
  string returnLabel;

  IFStmt(IFPartList *_ifPartList) {
    this->ifPartList = _ifPartList;
    this->scope = current_scope;
    this->line = yylineno;
    int iret = scopeLabels[this->scope] + _ifPartList->size() + 1;
    this->returnLabel = buildLable(this->scope, iret);
  }

  ~IFStmt() {
    delete ifPartList;
    ifPartList = 0;

    delete elsePart;
    elsePart = 0;
  }

  string repr() {
    string res = "";
    int &branches = scopeLabels[this->scope];
    for (auto ifPart : ifPartList->list) {
      string curLabel = buildLable(this->scope, branches++);
      string nextLabel = buildLable(this->scope, branches);
      res += ifPart->repr(curLabel, nextLabel, this->returnLabel);
    }
    res += label(this->returnLabel);
    scopeLabels[this->scope] += 3;
    return res;
  }
};
