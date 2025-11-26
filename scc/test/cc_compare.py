#
# Copyright (c) 2025 Nick Marino
# All rights reserved.
#

#
# This script implements a basic testing wrapper for comparing the output of
# a compiled executable to some expected variant (i.e. compiled by another cc).
#

import subprocess

tests = []

def run_test(test_file):
    subprocess.run(["scc", test_file, "-o", "main"], check=True)
    
    res = subprocess.run(["./main"], capture_output=True, text=True)
    exp = open(test_file + ".expected").read()
    return res.stdout == exp

for test_file in tests:
    if run_test(test_file):
        print("OK:", test_file)
    else:
        print("FAIL:", test_file)
