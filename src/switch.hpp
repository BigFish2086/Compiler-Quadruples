#pragma once

#include "expr.hpp"
#include "globals.hpp"

struct CaseStmt {
  Expr *expr;
  int scope, line;
  yytokentype type;
  string body;

  // CASE
  CaseStmt(Expr *_expr, const string &_body) {
    this->expr = _expr;
    this->type = _expr->type();
    this->scope = current_scope;
    this->line = yylineno;
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

  ~CaseStmt() { 
    if(expr != nullptr) {
      delete expr;
    }
  }

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
};

struct SwitchStmt {
  Expr *expr;
  int scope, line;
  yytokentype type;
  string returnLabel;
  CaseStmtList *caseStmtList;

  SwitchStmt(Expr *_expr, CaseStmtList *_caseStmtList) {
    this->expr = _expr;
    this->type = _expr->type();
    this->scope = current_scope;
    this->line = yylineno;
    this->check(_caseStmtList);
    this->caseStmtList = _caseStmtList;
    int iret = scopeLabels[this->scope] + _caseStmtList->size() + 1;
    this->returnLabel = buildLable(this->scope, iret);
  }

  ~SwitchStmt() {
    if(expr != nullptr) {
      delete expr;
    }
    if(caseStmtList != nullptr) {
      delete caseStmtList;
    }
  }

  void check(CaseStmtList *_caseStmtList) {
    this->caseStmtList = _caseStmtList;
    if (_caseStmtList->list.size() == 0) {
      error("switch case without any case");
    }
    for (auto caseStmt : _caseStmtList->list) {
      if(caseStmt->expr == nullptr) {
        continue;
      }
      if (caseStmt->type != this->type) {
        error("switch case type mismatch");
      }
    }
  }

  string repr() {
    string res = "";
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