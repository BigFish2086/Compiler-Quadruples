#pragma once

#include <string>

#define error(msg) throw std::runtime_error(msg)
#define warning(msg) std::cerr << "warning: " << msg << std::endl

#define syntax_error_msg                                                          \
  cerr << "syntax error #" + std::to_string(++syntax_errors) +                   \
        " near: " + yytext + " in line #" + std::to_string(yylineno)

using std::string;

string quoted(const string &str) { return "\"" + str + "\""; }

string push(const string &arg) { return "push " + arg + "\n"; }
string pop() { return "pop\n"; }

string pushs(const string &str) { return "push " + quoted(str) + "\n"; }

string pushv(const string &name, int scp) {
  return "push v_" + name + std::to_string(scp) + "\n";
}
string popv(const string &name, int scp) {
  return "pop v_" + name + std::to_string(scp) + "\n";
}

string popt() { return "pop tmp\n"; }
string pusht() { return "push tmp\n"; }

string int2flt() { return "int2flt\n"; }
string flt2int() { return "flt2int\n"; }

string funcdef(const string &name, int scp) {
  return "def f_" + name + std::to_string(scp) + ":\n";
}
string funcall(const string &name) { return "call f_" + name + "\n"; }
string ret() { return "ret\n"; }

string print() { return "print\n"; }

string neg() { return "neg\n"; }
string add() { return "add\n"; }
string sub() { return "sub\n"; }
string mult() { return "mult\n"; }
string div() { return "div\n"; }

string lt() { return "lt\n"; }
string gt() { return "gt\n"; }
string le() { return "le\n"; }
string ge() { return "ge\n"; }
string eq() { return "eq\n"; }
string ne() { return "ne\n"; }

string qand() { return "and\n"; }
string qor() { return "or\n"; }
string qnot() { return "not\n"; }

string jmp(const string &label) { return "jmp " + label + "\n"; }
string jz(const string &label) { return "jz " + label + "\n"; }

string label(const string &label) { return "label" + label + ":\n"; }
