#ifndef TEST_PSEUDODYLIB_SIMPLE_H
#define TEST_PSEUDODYLIB_SIMPLE_H

// #define PD_CTOR_DTOR
#define PD_EXTERN_SYMS

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PD_CTOR_DTOR
extern void ctor(void);
#endif
#ifdef PD_EXTERN_SYMS
typedef int (*fact_fptr_t)(int n);
extern int fact(int n);
#endif
typedef int (*dbl_fptr_t)(int n);
extern int dbl(int n);
#ifdef PD_CTOR_DTOR
extern void dtor(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
