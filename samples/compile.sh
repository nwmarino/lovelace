#!/bin/bash

cd ../lace/samples/
as rt.s -o rt.o
cd ../../samples/

for file in *.lace; do
    if [[ -f "$file" ]]; then
        ./../lace/lace "$file"
        ld "$file".o ../lace/samples/rt.o -o "${file%.lace}"
    fi
done
