/* libffi: build call interfaces at run time and invoke functions through them,
 * plus a closure -- all of which is architecture-specific assembly. */
#include "common.h"
#include <stdlib.h>
#include <ffi.h>
static int add3(int a, int b, int c) { return a + b + c; }
static double scale(double x, float f) { return x * f; }
static void closure_fn(ffi_cif *cif, void *ret, void **args, void *user) {
  (void)cif;
  *(int *)ret = *(int *)args[0] * *(int *)args[1] + *(int *)user;
}
int main(void) {
  QT_BEGIN("libffi");
  /* --- three integer arguments --- */
  ffi_cif cif;
  ffi_type *itypes[3] = { &ffi_type_sint, &ffi_type_sint, &ffi_type_sint };
  QT_CHECK(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 3, &ffi_type_sint, itypes) == FFI_OK,
           "ffi_prep_cif for int(int,int,int)");
  int a = 10, b = 20, c = 12, result = 0;
  void *iargs[3] = { &a, &b, &c };
  ffi_call(&cif, FFI_FN(add3), &result, iargs);
  QT_CHECK(result == 42, "ffi_call returned 42");

  /* --- mixed floating-point arguments exercise a different register class --- */
  ffi_cif fcif;
  ffi_type *ftypes[2] = { &ffi_type_double, &ffi_type_float };
  QT_CHECK(ffi_prep_cif(&fcif, FFI_DEFAULT_ABI, 2, &ffi_type_double, ftypes) == FFI_OK,
           "ffi_prep_cif for double(double,float)");
  double x = 2.5; float f = 4.0f, unused = 0; (void)unused;
  double dres = 0;
  void *fargs[2] = { &x, &f };
  ffi_call(&fcif, FFI_FN(scale), &dres, fargs);
  QT_CHECK(dres == 10.0, "float/double call returned 10.0");

  /* --- a closure: libffi generates a real function pointer at run time --- */
  void *code = NULL;
  ffi_closure *cl = ffi_closure_alloc(sizeof(ffi_closure), &code);
  QT_CHECK(cl != NULL && code != NULL, "ffi_closure_alloc");
  if (cl) {
    ffi_cif ccif;
    ffi_type *ctypes[2] = { &ffi_type_sint, &ffi_type_sint };
    int bias = 2;
    QT_CHECK(ffi_prep_cif(&ccif, FFI_DEFAULT_ABI, 2, &ffi_type_sint, ctypes) == FFI_OK,
             "ffi_prep_cif for the closure");
    QT_CHECK(ffi_prep_closure_loc(cl, &ccif, closure_fn, &bias, code) == FFI_OK,
             "ffi_prep_closure_loc");
    int (*fn)(int, int) = (int (*)(int, int))code;
    QT_CHECK(fn(5, 8) == 42, "calling through the closure returned 42");
    ffi_closure_free(cl);
  }
  QT_END();
}
