#include <caml/mlvalues.h>
#include <caml/custom.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "conversions.c"

#define MODULE "Gmp."

value _mlgmp_get_runtime_version(value dummy)
{
  CAMLparam0();
  CAMLlocal1(r);
  r = alloc_string(strlen(gmp_version));
  strcpy(String_val(r), gmp_version);
  CAMLreturn(r);
}

value _mlgmp_get_compile_version(value dummy)
{
  CAMLparam0();
  CAMLlocal1(r);
  r = alloc_tuple(3);
  Store_field(r, 0, Val_int(__GNU_MP_VERSION));
  Store_field(r, 1, Val_int(__GNU_MP_VERSION_MINOR));
  Store_field(r, 2, Val_int(__GNU_MP_VERSION_PATCHLEVEL));
  CAMLreturn(r);
}

value _mlgmp_is_mpfr_available(value dummy)
{
#ifdef USE_MPFR
  return Val_true;
#else
  return Val_false;
#endif
}

void division_by_zero(void)
{
  raise_constant(*caml_named_value("Gmp.Division_by_zero"));
}

void raise_unimplemented(char *s)
{
  raise_with_string(*caml_named_value("Gmp.Unimplemented"), s);
}
