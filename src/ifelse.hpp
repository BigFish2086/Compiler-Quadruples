#pragma once

#include "expr.hpp"
#include "globals.hpp"

struct IFPart {
  shared_ptr<Expr> expr;
  int scope, line;
  yytokentype type;
  string body;

  // IF
  IFPart(shared_ptr<Expr> _expr, const string &_body) {
    this->check(_expr);
    this->expr = _expr;
    this->type = _expr->type();
    this->scope = current_scope;
    this->line = yylineno;
    this->body = _body;
  }

  // ELSE
  IFPart(const string &_body) {
    this->expr = nullptr;
    this->type = yytokentype::INTEGER;
    this->scope = current_scope;
    this->line = yylineno;
    this->body = _body;
  }

  ~IFPart() {}

  void check(shared_ptr<Expr> _expr) {
    if (!canCast(_expr->type(), yytokentype::BOOL)) {
      error("if condition must be of type convertable to boolean");
    }
    if (_expr->type() == yytokentype::BOOL) {
      string cond = std::get<bool>(_expr->value) ? "true" : "false";
      warning("if condition is always " + cond);
    }
  }

  string repr(const string &curLabel, const string &nextLabel,
              const string &retLabel) {
    string res = "";
    res += label(curLabel);
    if (expr != nullptr) {
      res += jz(nextLabel);
    }
    res += body;
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
