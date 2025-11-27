// Defining a previously declared function.

int foo();

int foo() {
    return 5;
}

// Defining a previously declared function with parameters.

int bar(int x);

int bar(int x) {
    return x + 1;
}

// Redeclaring a previously defined function.

int zoo() {
    return 5;
}

int zoo();

// Defining a previously declared function with different parameters (error).

int spa();

int spa(int x) {
    return 1;
}

// Redefining a previously defined function (error).

int hotel() {
    return 1;
}

int hotel() {
    return 2;
}

