#define error(msg) throw std::runtime_error(msg)

#define push(arg) output << "\tPUSH " << arg << '\n'
#define pop() output << "\tPOP" << '\n'

#define pushs(str) output << "\tPUSH " << quoted(str) << '\n'

#define pushv(name, scp) output << "\tPUSH v_" << name << scp << '\n'
#define popv(name, scp) output << "\tPOP v_" << name << scp << '\n'

#define popt() output << "\tPOP " << "tmp" << '\n'
#define pusht() output << "\tPUSH " << "tmp" << '\n'

#define int2flt() output << "\tINT2FLT" << '\n'
#define flt2int() output << "\tFLT2INT" << '\n'

#define funcdef(name, scp) output << "\nDEF f_" << name << scp << ":" << '\n'
#define funcall(name, scp) output << "\tCALL f_" << name << scp << '\n'
#define ret() output << "\tRET" << '\n'

#define print() output << "\tPRINT" << '\n'
#define newline() output << '\n'

// arithmetic Operations
#define neg() output << "\tNEG" << '\n'
#define add() output << "\tADD" << '\n'
#define sub() output << "\tSUB" << '\n'
#define mult() output << "\tMULT" << '\n'
#define div() output << "\tDIV" << '\n'

// comparison operations
#define lt() output << "\tLT" << '\n'
#define gt() output << "\tGT" << '\n'
#define le() output << "\tLE" << '\n'
#define ge() output << "\tGE" << '\n'
#define eq() output << "\tEQ" << '\n'
#define ne() output << "\tNE" << '\n'

// logical opeartions
#define qand() output << "\tAND" << '\n'
#define qor() output << "\tOR" << '\n'
#define qnot() output << "\tNOT" << '\n'

