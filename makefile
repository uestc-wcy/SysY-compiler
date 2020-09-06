object=SysY.tab.o lex.yy.o ast.o gen_code.o ir_optimize.o gen_arm.o 

compiler: $(object)
	g++ -g -o compiler $(object)
	-rm ir_optimize.o

lex.yy.o:lex.yy.c SysY.tab.h
	g++ -g -c lex.yy.c 

SysY.tab.o:SysY.tab.c
	g++ -g -c SysY.tab.c

gen_arm.o:gen_arm.cpp	
	g++ -g -c gen_arm.cpp

ast.o:ast.cpp
	g++ -g -c ast.cpp

gen_code.o:gen_code.cpp	
	g++ -g -c gen_code.cpp

ir_optimize.o:ir_optimize.cpp	
	g++ -g -c ir_optimize.cpp

lex.yy.c:SysY.l
	flex SysY.l

SysY.tab.h SysY.tab.c:SysY.y
	bison -d SysY.y

