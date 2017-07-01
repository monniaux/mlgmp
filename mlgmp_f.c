#include <caml/mlvalues.h>
#include <caml/custom.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "mlgmp.h"
#include "conversions.c"

#define MODULE "Gmp.F."

/*** Allocation functions */

void _mlgmp_f_finalize(value r)
{
  mpf_clear(*mpf_val(r));
}

int _mlgmp_f_custom_compare(value a, value b);

void _mlgmp_f_serialize(value v,
			unsigned long * wsize_32,
			unsigned long * wsize_64);
unsigned long _mlgmp_f_deserialize(void * dst);

struct custom_operations _mlgmp_custom_f =
  {
    field(identifier)  "Gmp.F.t",
    field(finalize)    &_mlgmp_f_finalize,
    field(compare)     &_mlgmp_f_custom_compare,
    field(hash)        custom_hash_default,
#ifdef SERIALIZE
    field(serialize)   &_mlgmp_f_serialize,
    field(deserialize) &_mlgmp_f_deserialize
#else
    field(serialize)   custom_serialize_default,
    field(deserialize) custom_deserialize_default
#endif
  };

value _mlgmp_f_create(value prec)
{
  CAMLparam1(prec);
  CAMLreturn(alloc_init_mpf(prec));
}

value _mlgmp_f_from_z(value prec, value a)
{
  CAMLparam2(prec, a);
  CAMLlocal1(r);
  r=alloc_init_mpf(prec);
  mpf_set_z(*mpf_val(r), *mpz_val(a));
  CAMLreturn(r);
}

value _mlgmp_f_from_q(value prec, value a)
{
  CAMLparam2(prec, a);
  CAMLlocal1(r);
  r=alloc_init_mpf(prec);
  mpf_set_q(*mpf_val(r), *mpq_val(a));
  CAMLreturn(r);
}

value _mlgmp_f_from_si(value prec, value a)
{
  CAMLparam2(prec, a);
  CAMLlocal1(r);
  r=alloc_init_mpf(prec);
  mpf_set_si(*mpf_val(r), Long_val(a));
  CAMLreturn(r);
}

value _mlgmp_f_from_float(value prec, value v)
{
  CAMLparam2(prec, v);
  CAMLlocal1(r);
  r=alloc_init_mpf(prec);
  mpf_set_d(*mpf_val(r), Double_val(v));
  CAMLreturn(r);
}

/*** Conversions */

value _mlgmp_f_to_float(value v)
{
  CAMLparam1(v);
  CAMLlocal1(r);
  r = copy_double(mpf_get_d(*mpf_val(v)));
  CAMLreturn(r);
}

value _mlgmp_f_to_string_exp_base_digits(value base, value digits, value val)
{
  CAMLparam3(base, digits, val);
  CAMLlocal2(r, rs);
  mp_exp_t exponent;
  char *s= mpf_get_str(NULL, &exponent, Int_val(base), Int_val(digits),
		       *mpf_val(val));
  rs=alloc_string(strlen(s));
  strcpy(String_val(rs), s);
  free(s);
  r=alloc_tuple(2);
  Store_field(r, 0, rs);
  Store_field(r, 1, Val_int(exponent));
  CAMLreturn(r);
}

value _mlgmp_f_from_string(value prec, value base, value str)
{
  CAMLparam3(prec, base, str);
  CAMLlocal1(r);
  r=alloc_init_mpf(prec);
  mpf_set_str(*mpf_val(r), String_val(str), Int_val(base));
  CAMLreturn(r);
}

/*** Operations */
/**** Arithmetic */

#define f_binary_op_mpf(op)	        			\
value _mlgmp_f_##op(value prec, value a, value b)	\
{							\
  CAMLparam3(prec, a, b);                               \
  CAMLlocal1(r);                                        \
  r=alloc_init_mpf(prec);	       		        \
  mpf_##op(*mpf_val(r), *mpf_val(a), *mpf_val(b));	\
  CAMLreturn(r);	       				\
}

#define f_binary_op_ui(op)	       			\
value _mlgmp_f_##op(value prec, value a, value b)	\
{							\
  CAMLparam3(prec, a, b);                               \
  CAMLlocal1(r);                                        \
  r=alloc_init_mpf(prec);	       		        \
  mpf_##op(*mpf_val(r), *mpf_val(a), Long_val(b));	\
  CAMLreturn(r);	       				\
}

#define f_binary_ui_op(op)	       			\
value _mlgmp_f_##op(value prec, value a, value b)	\
{							\
  CAMLparam3(prec, a, b);                               \
  CAMLlocal1(r);                                        \
  r=alloc_init_mpf(prec);	       		        \
  mpf_##op(*mpf_val(r), Long_val(a), *mpf_val(b));	\
  CAMLreturn(r);	       				\
}

#define f_binary_op(op)				\
f_binary_op_ui(op##_ui)				\
f_binary_op_mpf(op)

#define f_unary_op(op)				\
value _mlgmp_f_##op(value prec, value a)	\
{						\
  CAMLparam2(prec, a);				\
  CAMLlocal1(r);				\
  r=alloc_init_mpf(prec);      			\
  mpf_##op(*mpf_val(r), *mpf_val(a));		\
  CAMLreturn(r);				\
}

f_binary_op(add)
f_binary_op(sub)
f_binary_op(mul)

value _mlgmp_f_div(value prec, value n, value d)
{
  CAMLparam3(prec, n, d);
  CAMLlocal1(r);

  if (! mpf_sgn(*mpf_val(d)))
    division_by_zero(); /* is it ok to launch the exception here? */

  r = alloc_init_mpf(prec);
  mpf_div(*mpf_val(r), *mpf_val(n), *mpf_val(d));

  CAMLreturn(r);
}

value _mlgmp_f_div_ui(value prec, value n, value d)
{
  int real_d;
  CAMLparam3(prec, n, d);
  CAMLlocal1(r);
  real_d = Long_val(d);

  if (! real_d)
    division_by_zero(); /* is it ok to launch the exception here? */

  r = alloc_init_mpf(prec);
  mpf_div_ui(*mpf_val(r), *mpf_val(n), real_d);

  CAMLreturn(r);
}

value _mlgmp_f_ui_div(value prec, value n, value d)
{
  int real_d;
  CAMLparam3(prec, n, d);
  CAMLlocal1(r);

  if (! mpf_sgn(*mpf_val(d)))
    division_by_zero(); /* is it ok to launch the exception here? */

  r = alloc_init_mpf(prec);
  mpf_ui_div(*mpf_val(r), Long_val(n), *mpf_val(d));

  CAMLreturn(r);
}

f_binary_ui_op(ui_sub)

f_unary_op(neg)
f_unary_op(abs)

f_unary_op(ceil)
f_unary_op(floor)
f_unary_op(trunc)

/*** Compare */

int _mlgmp_f_custom_compare(value a, value b)
{
  CAMLparam2(a, b);
  CAMLreturn(mpf_cmp(*mpf_val(a), *mpf_val(b)));
}

value _mlgmp_f_cmp(value a, value b)
{
  CAMLparam2(a, b);
  CAMLreturn(Val_int(mpf_cmp(*mpf_val(a), *mpf_val(b))));
}

value _mlgmp_f_cmp_si(value a, value b)
{
  CAMLparam2(a, b);
  CAMLreturn(Val_int(mpf_cmp_si(*mpf_val(a), Long_val(b))));
}

value _mlgmp_f_sgn(value a)
{
  CAMLparam1(a);
  CAMLreturn(Val_int(mpf_sgn(*mpf_val(a))));
}

value _mlgmp_f_eq(value a, value b, value nbits)
{
  CAMLparam3(a, b, nbits);
  CAMLreturn(mpf_eq(*mpf_val(a), *mpf_val(b), Int_val(nbits))
	     ? Val_true : Val_false);
}

f_binary_op_mpf(reldiff)


/*** Random */
value _mlgmp_f_urandomb(value prec, value state, value nbits)
{
  CAMLparam3(prec, state, nbits);
  CAMLlocal1(r);
  r = alloc_init_mpf(prec);
  mpf_urandomb(*mpf_val(r), *randstate_val(state), Int_val(nbits));
  CAMLreturn(r);
}

value _mlgmp_f_random2(value prec, value nlimbs, value max_exp)
{
  CAMLparam3(prec, nlimbs, max_exp);
  CAMLlocal1(r);
  r = alloc_init_mpf(prec);
  mpf_random2(*mpf_val(r), Int_val(nlimbs), Int_val(max_exp));
  CAMLreturn(r);
}


/*** Serialization */
value _mlgmp_f_initialize()
{
  CAMLparam0();
  register_custom_operations(& _mlgmp_custom_f);
  CAMLreturn(Val_unit);
}

#ifdef SERIALIZE
void _mlgmp_f_serialize(value v,
			unsigned long * wsize_32,
			unsigned long * wsize_64)
{
  CAMLparam1(v);
  char *s;
  int len;
  mp_exp_t exponent;
  char exponent_buf[10];

  *wsize_32 = MPF_SIZE_ARCH32;
  *wsize_64 = MPF_SIZE_ARCH64;

  serialize_int_4(mpf_get_prec(*mpf_val(v)));

  s = mpf_get_str (NULL, &exponent, 16, 0, *mpf_val(v));
  len = strlen(s);
  serialize_int_4(len + 11);

  serialize_block_1("0.", 2);
  serialize_block_1(s, len);
  free(s);

  sprintf(exponent_buf, "@%08lx", (exponent & 0xFFFFFFFFUL));
  serialize_block_1(exponent_buf, 9);

  CAMLreturn0;
}

unsigned long _mlgmp_f_deserialize(void * dst)
{
  char *s;
  int len;

  mpf_init2(*((mpf_t*) dst), deserialize_uint_4());

  len = deserialize_uint_4();
  s = malloc(len+1);
  deserialize_block_1(s, len);
  s[len] = 0;
  mpf_set_str (*((mpf_t*) dst), s, 16);
  free(s);

  return sizeof(mpf_t);
}
#endif
