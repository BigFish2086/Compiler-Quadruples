#pragma once

#include "expr.hpp"
#include "globals.hpp"

struct CaseStmt {
  shared_ptr<Expr> expr;
  int scope, line;
  yytokentype type;
  string body;

  // CASE
  CaseStmt(shared_ptr<Expr> _expr, const string &_body) {
    this->scope = current_scope;
    this->line = yylineno;
    this->expr = _expr;
    this->type = _expr->type();
    this->body = _body;
  }

  // DEFAULT
  CaseStmt(const string &_body) {
    this->expr = nullptr;
    this->type = yytokentype::INTEGER;
    this->scope = current_scope;
    this->line = yylineno;
    this->body = _body;
  }

  ~CaseStmt() {}

  string repr(const string &curLabel, const string &nextLabel,
              const string &retLabel) {
    string res = "";
    res += label(curLabel);
    if(expr != nullptr) {
      res += popt() + pusht() + pusht();
      res += push(expr->repr());
      res += eq();
      res += jz(nextLabel);
    }
    res += body;
    res += pop();
    res += jmp(retLabel);
    return res;
  }
};

struct CaseStmtList {
  vector<CaseStmt *> list;
  CaseStmtList() {}
  CaseStmtList(const CaseStmt *item) { this->append(item); }
  CaseStmtList *append(const CaseStmt *item) {
    list.push_back((CaseStmt *)item);
    return this;
  }
  int size() { return list.size(); }
  ~CaseStmtList() {
    for (auto item : list) {
      delete item;
      item = 0;
    }
    this->list.clear();
  }
};

struct SwitchStmt {
  shared_ptr<Expr> expr;
  int scope, line;
  yytokentype type;
  string returnLabel;
  string condition_body;
  CaseStmtList *caseStmtList = nullptr;

  SwitchStmt(shared_ptr<Expr> _expr, const string &condition_body, CaseStmtList *_caseStmtList) {
    this->scope = current_scope;
    this->line = yylineno;
    this->expr = _expr;
    this->type = _expr->type();
    this->check(_caseStmtList);
    this->caseStmtList = _caseStmtList;
    int iret = scopeLabels[this->scope] + _caseStmtList->size() + 1;
    this->condition_body = condition_body;
    this->returnLabel = buildLable(this->scope, iret);
  }

  ~SwitchStmt() {
    delete caseStmtList;
    caseStmtList = 0;
  }

  void check(CaseStmtList *_caseStmtList) {
    this->caseStmtList = _caseStmtList;
    if (_caseStmtList->list.size() == 0) {
      error("switch at L# " + to_string(this->line) + " without any case");
    }
    for (auto caseStmt : _caseStmtList->list) {
      if(caseStmt->expr == nullptr) {
        continue;
      }
      if (this->type != caseStmt->type) {
        error("switch at L# " + to_string(this->line) + " type mismatch in case at L# " + to_string(caseStmt->line));
      }
    }
  }

  string repr() {
    string res = "";
    res += this->condition_body;
    int &caseLabelsCounter = scopeLabels[this->scope];
    for (auto caseStmt : caseStmtList->list) {
      string curLabel = buildLable(this->scope, caseLabelsCounter++);
      string nextLabel = buildLable(this->scope, caseLabelsCounter);
      res += caseStmt->repr(curLabel, nextLabel, this->returnLabel);
    }
    res += label(this->returnLabel);
    scopeLabels[this->scope] += 3;
    return res;
  }
};
