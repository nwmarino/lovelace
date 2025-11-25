struct A { 
    struct B* b; 
};

struct B {
    struct A y;
    int x;
};

enum C {
    A,
    B,
    C,
};

typedef struct B Ab;

typedef enum C Ac;
