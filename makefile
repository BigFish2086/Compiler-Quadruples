bison:
	@echo -e [*] Running Bison on parser.y
	bison -d parser.y # --debug # -Wother -Wcounterexample # --yacc
	@echo -e "\n\n"

flex: 
	@echo -e [*] Running Flex on lexer.l
	flex lexer.l
	@echo -e "\n\n"

compile:
	@echo -e [*] Compiling the compiler
	# g++ -g -std=c++20 parser.tab.c lex.yy.c -o compiler 
	g++ -DDEBUG -DLOCAL -std=c++20 -Wshadow -Wall -Wno-unused-result -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG parser.tab.c lex.yy.c -o compiler
	@echo -e "\n\n"
	
clean: 
	@echo -e [*] Cleaning up
	rm -f *.tab.c *.tab.h *.yy.c *.output compiler
	@echo -e "\n\n"

run:
	@echo -e [*] Running the compiler
	./compiler
	@echo -e "\n\n"

build: clean bison flex compile

all: clean bison flex compile run
