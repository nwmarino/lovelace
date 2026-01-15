#!/bin/bash

cd /home/lovelace/lace
./lace samples/linux.lace samples/A.lace samples/mem.lace
cd samples/
ld rt.o linux.lace.o A.lace.o mem.lace.o -o main
./main
