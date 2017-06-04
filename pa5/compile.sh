#!/bin/sh
./lexer test1.cl | ./parser $* | ./semant $* > test.semant
./cgen > test.s
spim test.s
