#!/bin/bash

# g++ -o assembly_to_machine assembly_to_machine.cpp 
# ./assembly_to_machine
# g++ -o adapter adapter.cpp
# ./adapter


bison -d -y parser.y
echo 'Generated the parser C file as well the header file'
g++ -w -c -o y.o y.tab.c
echo 'Generated the parser object file'
flex lexer.lex
echo 'Generated the scanner C file'
g++ -w -c -o l.o lex.yy.c
# if the above command doesn't work try g++ -fpermissive -w -c -o l.o lex.yy.c
echo 'Generated the scanner object file'
g++ y.o l.o -lfl -o assembler.out
echo 'All ready, running' 
./assembler.out mips.txt KAGBLIPJNFMDCHEO
g++ -o adapter adapter.cpp
./adapter