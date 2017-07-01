#include <caml/mlvalues.h>
#include <caml/custom.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <gmp.h>
#include <stdio.h>

#include "config.h"

#define MODULE "Gmp.Random."

/*** Allocation functions */

void _mlgmp_random_finalize(value r)
{
  gmp_randclear(Data_custom_val(r));
}

struct custom_operations _mlgmp_custom_random =
  {
    field(identifier)  "Gmp.Random.randstate_t",
    field(finalize)    &_mlgmp_random_finalize,
    field(compare)     custom_compare_default,
    field(hash)        custom_hash_default,
    field(serialize)   custom_serialize_default,
    field(deserialize) custom_deserialize_default
  };

#undef field

value _mlgmp_randinit_lc(value n)
{
  CAMLparam1(n);
  CAMLlocal1(r);
  r = alloc_custom(&_mlgmp_custom_random,
		       sizeof(gmp_randstate_t),
		       4,
		       1000000);
  gmp_randinit(*((gmp_randstate_t*) Data_custom_val(r)),
	       GMP_RAND_ALG_LC,
	       Long_val(n));
  CAMLreturn(r); 
}
