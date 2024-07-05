#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#undef NULL
#define NULL ((void*)0)

typedef int bool_ty;
#define b_TRUE ((int)1)
#define b_FALSE ((int)0)

typedef unsigned int size_ty;

struct color_st {
    float r;
    float g;
    float b;
    float a;
};

struct point_st {
    int x;
    int y;
};

struct pointf_st {
    float x;
    float y;
};

struct rect_st {
    struct point_st position;
    struct point_st size;
};

struct rectf_st {
    struct pointf_st position;
    struct pointf_st size;
};

#endif