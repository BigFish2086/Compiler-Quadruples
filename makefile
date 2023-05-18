bison:
	@echo -e \----------- Running Bison on Parser -----------
	bison -d parser.y --debug # -Wother -Wcounterexample # --yacc

flex: 
	@echo -e \----------- Running Flex on lexer -----------
	flex lexer.l

compile:
	@echo -e \----------- Compiling both -----------
	# g++ -g -std=c++20 parser.tab.c lex.yy.c -o compiler 
	g++ -DDEBUG -DLOCAL -std=c++20 -Wshadow -Wall -Wno-unused-result -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG parser.tab.c lex.yy.c -o compiler


build: bison flex compile
	
run:
	@echo -e \----------- Running the compiler -----------
	./compiler

clean: 
	@echo -e ----------- Cleaning The Directory -----------
	rm -f *.tab.c *.tab.h *.yy.c *.output compiler

all: clean build run
