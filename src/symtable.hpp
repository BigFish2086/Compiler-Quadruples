#pragma once

#include "expr.hpp"
#include "globals.hpp"
#include "id.hpp"

vector<map<string, ID *>> symbolTable(1);

void enterScope() {
  symbolTable.push_back(map<string, ID *>());
  current_scope++;
}

void exitScope() {
  map<string, ID *> scope = symbolTable[current_scope];
  for (auto it : scope) {
    ID *id = it.second;
    if (!id->isUsed) {
      cout << "Warning: ID " << id->name << " declared but not used" << endl;
    }
  }
  symbolTable.pop_back();
  current_scope--;
}

template <typename T, typename = typename std::enable_if<
                          std::is_base_of<ID, T>::value, T>::type>
T *getID(const string &name) {
  T *id = nullptr;
  for (int i = symbolTable.size() - 1; i >= 0; i--) {
    if (symbolTable[i].find(name) != symbolTable[i].end()) {
      id = dynamic_cast<T *>(symbolTable[i][name]);
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

void declareID(ID *id) {
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

void funcHasReturnStatment(const string &funcName) {
  auto [type, isReturned] = funcReturnTypesStack.back();
  if (isReturned) {
    error("function " + funcName + " has more than one return statement");
  }
  // TODO: add another return for safty measures
}

void validFuncReturnType(Expr *expr) {
  yytokentype returnType = funcReturnTypesStack.back().first;
  if (returnType != expr->type()) {
    error("function return type mismatch");
  }
  funcReturnTypesStack.back().second = true;
}

Expr *callingFunc(const string &name, const struct TypedList *paramsTypes) {
  FuncID *funcID = getID<FuncID>(name);
  if (funcID->funcParamsTypes.size() != paramsTypes->list.size()) {
    error("function " + name + " called with wrong number of arguments");
  }
  for (int i = 0; i < (int)funcID->funcParamsTypes.size(); i++) {
    if (funcID->funcParamsTypes[i] != paramsTypes->list[i]) {
      error("function " + name + " called with wrong argument type");
    }
  }
  return new Expr(funcID->type);
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
