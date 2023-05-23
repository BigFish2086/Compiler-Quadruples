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

void ForceSymbolTableClean() {
  symbolTable.clear();
  symbolTable.push_back(Scope());
  current_scope = 0;
}

template <typename T>
concept IDType = std::is_base_of<ID, T>::value;

template <IDType T> shared_ptr<T> getID(const string &name) {
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

void logSymbolTable() {
  string line(105, '-');
  string info =
      "N: Name L# Line S@ Scope U: Used T: Type [ID Type] [I: isInit] [C: isConst] [ID Expr] [Enum Type]";
  symlog << line << '\n' << info << '\n' << line << "\n\n";
  vector<vector<string>> table;
  int numberOfCols = 0;
  for (int i = 0; i < (int)symbolTable.size(); i++) {
    for (auto it : symbolTable[i]) {
      table.push_back(it.second->vstr());
      numberOfCols = max(numberOfCols, (int)table.back().size());
    }
  }
  vector<int> max_len(numberOfCols, 0);
  for (int r = 0; r < (int)table.size(); r++) {
    for (int c = 0; c < (int)table[r].size(); c++) {
      int len = table[r][c].length();
      max_len[c] = max(max_len[c], len);
    }
  }
  for (int r = 0; r < (int)table.size(); r++) {
    for (int c = 0; c < (int)table[r].size(); c++) {
      symlog << setw(max_len[c] + 3) << left << table[r][c];
      if (c != (int)table[r].size() - 1) {
        symlog << "| ";
      }
    }
    symlog << '\n';
  }
  symlog << "\n\n";
}

// ----------------------------------------------------------------------
struct RetData {
  yytokentype type;
  bool isReturned;
  string funcName;
};
vector<RetData> funcReturnTypesStack;

void enterFunc(yytokentype type, const string &name) {
  funcReturnTypesStack.emplace_back(type, false, name);
  enterScope();
}

void validFuncReturnType(shared_ptr<Expr> expr) {
  auto &[returnType, isReturned, funcName] = funcReturnTypesStack.back();
  if (returnType != expr->type()) {
    if (!canCast(expr->type(), returnType)) {
      error("function return type mismatch since it is " +
            type2Str[returnType] + " but " + type2Str[expr->type()] +
            " is given and cannot be casted");
    } else {
      warning("function " + funcName + " return type casted from " + type2Str[expr->type()] +
              " to " + type2Str[returnType]);
    }
  }
  isReturned = true;
}

void funcHasReturnStatment(const string &funcName) {
  auto [type, isReturned, name] = funcReturnTypesStack.back();
  if (!isReturned) {
    error("function " + funcName + " does not return any value");
  }
  // TODO: add another return for safty measures
}

void exitFunc(const string &funcName) {
  exitScope();
  auto [type, isReturned, name] = funcReturnTypesStack.back();
  funcReturnTypesStack.pop_back();
  if (!isReturned) {
    error("function " + funcName + " does not return any value");
  }
  // TODO: print the needed quads to simulate the return type
}

shared_ptr<Expr> callingFunc(const string &name, shared_ptr<TypedList> paramsTypes) {
  shared_ptr<FuncID> funcID = getID<FuncID>(name);
  if (funcID->funcParamsTypes->size() != paramsTypes->size()) {
    error("function " + name + " called with wrong number of arguments");
  }
  for (int i = 0; i < (int)funcID->funcParamsTypes->size(); i++) {
    yytokentype to = funcID->funcParamsTypes->list[i];
    yytokentype from = paramsTypes->list[i];
    if (!canCast(from, to)) {
      error("function " + name + " called with wrong argument type");
    }
  }
  return shared_ptr<Expr>(new Expr(type2Default[funcID->type]));
}

string callingFuncTypeConv(const string &name, shared_ptr<TypedList> paramsTypes) {
  shared_ptr<FuncID> funcID = getID<FuncID>(name);
  string ret = "";
  for (int i = funcID->funcParamsTypes->size() - 1; i >= 0; i--) {
    yytokentype to = funcID->funcParamsTypes->list[i];
    yytokentype from = paramsTypes->list[i];
    ret += e2idCast(from, to);
  }
  return ret;
}
