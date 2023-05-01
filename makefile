bison:
	@echo -e \----------- Running Bison on Parser -----------
	bison --yacc parser.y -d --debug

flex: 
	@echo -e \----------- Running Flex on lexer -----------
	flex lexer.l

compile:
	@echo -e \----------- Compiling both -----------
	g++ -g -std=c++20 -o compiler y.tab.c lex.yy.c


build: bison flex compile
	
run:
	@echo -e \----------- Running the compiler -----------
	./compiler

clean: 
	@echo -e ----------- Cleaning The Directory -----------
	rm -f *.tab.c *.tab.h *.yy.c *.output compiler

all: clean build run
