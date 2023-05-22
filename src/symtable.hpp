#pragma once

#include "expr.hpp"
#include "globals.hpp"
#include "id.hpp"

typedef shared_ptr<ID> IDPtr;
typedef map<string, IDPtr> Scope;

vector<Scope> symbolTable(1);

void enterScope() {
  symbolTable.push_back(Scope());
  current_scope++;
}

void exitScope() {
  Scope scope = symbolTable.back();
  for (auto it : scope) {
    auto id = it.second;
    if (!id->isUsed) {
      warning("ID " + id->name + " is not used");
    }
  }
  symbolTable.pop_back();
  current_scope--;
}

template <typename T>
concept IDType = std::is_base_of<ID, T>::value;

template <IDType T>
shared_ptr<T> getID(const string &name) {
  shared_ptr<T> id = nullptr;
  for (int i = symbolTable.size() - 1; i >= 0; i--) {
    if (symbolTable[i].find(name) != symbolTable[i].end()) {
      id = dynamic_pointer_cast<T>(symbolTable[i][name]);
      if (id != nullptr) {
        break;
      }
    }
  }
  if (id == nullptr) {
    error("ID " + name + " is not declared");
  }
  id->isUsed = true;
  return id;
}

void declareID(shared_ptr<ID> id) {
  if (symbolTable[current_scope].find(id->name) !=
      symbolTable[current_scope].end()) {
    error("ID " + id->name + " already declared");
  }
  symbolTable[current_scope][id->name] = id;
}

// ----------------------------------------------------------------------
vector<pair<yytokentype, bool>> funcReturnTypesStack;

void enterFunc(yytokentype type) {
  funcReturnTypesStack.emplace_back(type, false);
  enterScope();
}

void validFuncReturnType(shared_ptr<Expr> expr) {
  yytokentype returnType = funcReturnTypesStack.back().first;
  if (returnType != expr->type()) {
    if (!canCast(expr->type(), returnType)) {
      error("function return type mismatch since it is " +
            type2Str[returnType] + " but " + type2Str[expr->type()] +
            " is given and cannot be casted");
    } else {
      warning("function return type casted from " + type2Str[expr->type()] +
              " to " + type2Str[returnType]);
    }
  }
  funcReturnTypesStack.back().second = true;
}

void funcHasReturnStatment(const string &funcName) {
  auto [type, isReturned] = funcReturnTypesStack.back();
  if (!isReturned) {
    error("function " + funcName + " does not return any value");
  }
  // TODO: add another return for safty measures
}

void exitFunc() {
  auto [type, isReturned] = funcReturnTypesStack.back();
  funcReturnTypesStack.pop_back();
  if (!isReturned) {
    error("function does not return any value");
  }
  // TODO: print the needed quads to simulate the return type
  // exitScope();
}

shared_ptr<Expr> callingFunc(const string &name, const struct TypedList *paramsTypes) {
  shared_ptr<FuncID> funcID = getID<FuncID>(name);
  if (funcID->funcParamsTypes.size() != paramsTypes->list.size()) {
    error("function " + name + " called with wrong number of arguments");
  }
  for (int i = 0; i < (int)funcID->funcParamsTypes.size(); i++) {
    if (funcID->funcParamsTypes[i] != paramsTypes->list[i]) {
      error("function " + name + " called with wrong argument type");
    }
  }
  return shared_ptr<Expr>(new Expr(funcID->type));
}

