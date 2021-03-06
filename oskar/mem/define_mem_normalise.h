/* Copyright (c) 2019, The University of Oxford. See LICENSE file. */

#define OSKAR_MEM_NORMALISE_REAL(NAME, FP) KERNEL(NAME) (\
        const unsigned int offset, const unsigned int n,\
        GLOBAL FP* a, const unsigned int idx)\
{\
    const FP scal = ((FP)1) / a[idx];\
    KERNEL_LOOP_X(unsigned int, i, 0, n)\
    a[offset + i] *= scal;\
    KERNEL_LOOP_END\
}\
OSKAR_REGISTER_KERNEL(NAME)

#define OSKAR_MEM_NORMALISE_COMPLEX(NAME, FP, FP2) KERNEL(NAME) (\
        const unsigned int offset, const unsigned int n,\
        GLOBAL FP2* a, const unsigned int idx)\
{\
    const FP2 val = a[idx];\
    const FP scal = ((FP)1) / sqrt(val.x * val.x + val.y * val.y);\
    KERNEL_LOOP_X(unsigned int, i, 0, n)\
    a[offset + i].x *= scal; a[offset + i].y *= scal;\
    KERNEL_LOOP_END\
}\
OSKAR_REGISTER_KERNEL(NAME)

#define OSKAR_MEM_NORMALISE_MATRIX(NAME, FP, FP4c) KERNEL(NAME) (\
        const unsigned int offset, const unsigned int n,\
        GLOBAL FP4c* a, const unsigned int idx)\
{\
    const FP4c val = a[idx];\
    const FP amp = val.a.x * val.a.x + val.a.y * val.a.y +\
            val.b.x * val.b.x + val.b.y * val.b.y +\
            val.c.x * val.c.x + val.c.y * val.c.y +\
            val.d.x * val.d.x + val.d.y * val.d.y;\
    const FP scal = ((FP)1) / sqrt(amp / (FP)2);\
    KERNEL_LOOP_X(unsigned int, i, 0, n)\
    const int j = offset + i;\
    a[j].a.x *= scal; a[j].a.y *= scal;\
    a[j].b.x *= scal; a[j].b.y *= scal;\
    a[j].c.x *= scal; a[j].c.y *= scal;\
    a[j].d.x *= scal; a[j].d.y *= scal;\
    KERNEL_LOOP_END\
}\
OSKAR_REGISTER_KERNEL(NAME)
