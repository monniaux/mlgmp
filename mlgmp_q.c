#include <caml/mlvalues.h>
#include <caml/custom.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "config.h"
#include "conversions.c"

#define MODULE "Gmp.Q."

#define CAMLcheckreturn(r) \
assert(r > 0x10000); \
CAMLreturn(r)

/*** Allocation functions */

void _mlgmp_q_finalize(value r)
{
  mpq_clear(*mpq_val(r));
}

int _mlgmp_q_custom_compare(value a, value b);
void _mlgmp_q_serialize(value v,
			unsigned long * wsize_32,
			unsigned long * wsize_64);
unsigned long _mlgmp_q_deserialize(void * dst);

int _mlgmp_q_custom_compare(value a, value b);
long _mlgmp_q_hash(value v);

struct custom_operations _mlgmp_custom_q =
  {
    field(identifier)  "Gmp.Q.t",
    field(finalize)    &_mlgmp_q_finalize,
    field(compare)     &_mlgmp_q_custom_compare,
    field(hash)        &_mlgmp_q_hash,
#ifdef SERIALIZE
    field(serialize)   &_mlgmp_q_serialize,
    field(deserialize) &_mlgmp_q_deserialize
#else
    field(serialize)   custom_serialize_default,
    field(deserialize) custom_deserialize_default
#endif
  };

value _mlgmp_q_create(void)
{
  CAMLparam0();
  trace(create);
  CAMLcheckreturn(alloc_init_mpq());
}

value _mlgmp_q_from_z(value a)
{
  CAMLparam1(a);
  CAMLlocal1(r);
  trace(from_z);
  r=alloc_init_mpq();
  mpq_set_z(*mpq_val(r), *mpz_val(a));
  CAMLcheckreturn(r);
}

value _mlgmp_q_from_si(value n, value d)
{
  CAMLparam2(n, d);
  CAMLlocal1(r);
  trace(from_si);
  r=alloc_init_mpq();
  mpq_set_si(*mpq_val(r), Long_val(n), Long_val(d));
  mpq_canonicalize(*mpq_val(r));
  CAMLcheckreturn(r);
}

/*** Conversions */

value _mlgmp_q_from_float(value v)
{
  CAMLparam1(v);
  CAMLlocal1(r);
  trace(from_float);
  r=alloc_init_mpq();
  mpq_set_d(*mpq_val(r), Double_val(v));
  CAMLcheckreturn(r);
}

value _mlgmp_q_to_float(value v)
{
  CAMLparam1(v);
  CAMLlocal1(r);
  trace(to_float);
  r = copy_double(mpq_get_d(*mpq_val(v)));
  CAMLreturn(r);
}

/*** Operations */
/**** Arithmetic */

#define q_binary_op(op)	        			\
value _mlgmp_q_##op(value a, value b)			\
{							\
  CAMLparam2(a, b);                                     \
  CAMLlocal1(r);                                        \
  trace(op);	                		\
  r=alloc_init_mpq();				        \
  mpq_##op(*mpq_val(r), *mpq_val(a), *mpq_val(b));	\
  CAMLcheckreturn(r);	       				\
}                                                       \
                                                        \
value _mlgmp_q2_##op(value r, value a, value b)		\
{							\
  CAMLparam3(r, a, b);                                  \
  mpq_##op(*mpq_val(r), *mpq_val(a), *mpq_val(b));	\
  CAMLreturn(Val_unit);	       				\
}

#define q_unary_op(op)				\
value _mlgmp_q_##op(value a)			\
{						\
  CAMLparam1(a);				\
  CAMLlocal1(r);				\
  trace(op);	                	\
  r=alloc_init_mpq();				\
  mpq_##op(*mpq_val(r), *mpq_val(a));		\
  CAMLcheckreturn(r);				\
}

q_binary_op(add)
q_binary_op(sub)
q_binary_op(mul)
q_binary_op(div)

q_unary_op(neg)
q_unary_op(inv)
q_unary_op(abs)

#define q_z_unary_op(op)			\
value _mlgmp_q_##op(value a)			\
{						\
  CAMLparam1(a);				\
  CAMLlocal1(r);				\
  trace(op);	                	\
  r=alloc_init_mpz();				\
  mpq_##op(*mpz_val(r), *mpq_val(a));		\
  CAMLcheckreturn(r);				\
}

q_z_unary_op(get_num)
q_z_unary_op(get_den)

/*** Compare */

int _mlgmp_q_custom_compare(value a, value b)
{
  CAMLparam2(a, b);
  CAMLreturn(mpq_cmp(*mpq_val(a), *mpq_val(b)));
}

value _mlgmp_q_cmp(value a, value b)
{
  CAMLparam2(a, b);
  trace(cmp);	                	\
  CAMLreturn(Val_int(mpq_cmp(*mpq_val(a), *mpq_val(b))));
}

value _mlgmp_q_cmp_ui(value a, value n, value d)
{
  CAMLparam3(a, n, d);
  trace(cmp_ui);	                	\
  CAMLreturn(Val_int(mpq_cmp_ui(*mpq_val(a), Long_val(n), Long_val(d))));
}

value _mlgmp_q_sgn(value a)
{
  CAMLparam1(a);
  trace(sgn);	                	\
  CAMLreturn(Val_int(mpq_sgn(*mpq_val(a))));
}

/*** Serialization */
value _mlgmp_q_initialize()
{
  CAMLparam0();
  register_custom_operations(& _mlgmp_custom_q);
  CAMLreturn(Val_unit);
}

#ifdef SERIALIZE
void _mlgmp_q_serialize(value v,
			unsigned long * wsize_32,
			unsigned long * wsize_64)
{
  CAMLparam1(v);
  char *s;
  int len;

  *wsize_32 = MPQ_SIZE_ARCH32;
  *wsize_64 = MPQ_SIZE_ARCH64;

  s = mpz_get_str (NULL, 16, mpq_numref(*mpq_val(v)));
  len = strlen(s);
  serialize_int_4(len);
  serialize_block_1(s, len);
  free(s);

  s = mpz_get_str (NULL, 16, mpq_denref(*mpq_val(v)));
  len = strlen(s);
  serialize_int_4(len);
  serialize_block_1(s, len);
  free(s);

  CAMLreturn0;
}

unsigned long _mlgmp_q_deserialize(void * dst)
{
  char *s;
  int len;

  len = deserialize_uint_4();
  s = malloc(len+1);
  deserialize_block_1(s, len);
  s[len] = 0;
  mpz_init_set_str (mpq_numref(*((mpq_t*) dst)), s, 16);
  free(s);

  len = deserialize_uint_4();
  s = malloc(len+1);
  deserialize_block_1(s, len);
  s[len] = 0;
  mpz_init_set_str (mpq_denref(*((mpq_t*) dst)), s, 16);
  free(s);

  return sizeof(mpq_t);
}
#endif

long _mlgmp_q_hash(value v)
{
  CAMLparam1(v);
  mpz_t dummy;
  long r;
  mpz_init(dummy);

  r = mpz_mod_ui(dummy, mpq_denref(*mpq_val(v)), HASH_MODULUS)
    ^ mpz_mod_ui(dummy, mpq_numref(*mpq_val(v)), HASH_MODULUS);

  mpz_clear(dummy);
  CAMLreturn(r);
}
