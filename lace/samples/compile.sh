#!/bin/bash

cd /home/statim/lace
./lace
cd samples/
as A.lace.s -o A.lace.o
as mem.lace.s -o mem.lace.o
as linux.lace.s -o linux.lace.o
ld rt.o linux.lace.o A.lace.o mem.lace.o -o main
./main
