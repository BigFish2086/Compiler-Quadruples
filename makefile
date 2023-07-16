bison:
	@echo -e [*] Running Bison on parser.y
	bison -d parser.y # --debug # -Wother -Wcounterexample # --yacc
	@echo -e "\n"

flex: 
	@echo -e [*] Running Flex on lexer.l
	flex lexer.l
	@echo -e "\n"

compile:
	@echo -e [*] Compiling the compiler
	g++ -g -std=c++20 parser.tab.c lex.yy.c -o compiler 
	# g++ -DDEBUG -DLOCAL -std=c++20 -Wshadow -Wall -Wno-unused-result -g -fsanitize=address -fsanitize=undefined -D_GLIBCXX_DEBUG parser.tab.c lex.yy.c -o compiler
	@echo -e "\n"
	
clean: 
	@echo -e [*] Cleaning up
	rm -f *.tab.c *.tab.h *.yy.c ./bin/compiler
	@echo -e "\n"

mv:
	@echo -e [*] Moving files
	mv ./compiler ./bin/
	@echo -e "\n"

run:
	@echo -e [*] Running the compiler
	./bin/compiler
	@echo -e "\n"

build: clean bison flex compile mv

all: clean bison flex compile mv run
