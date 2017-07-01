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

#define MODULE "Gmp.FR."


#define unimplemented(message) \
  raise_unimplemented("Gmp.FR." #message);

/*** Allocation functions */

void _mlgmp_fr_finalize(value r)
{
#ifdef USE_MPFR
  mpfr_clear(*mpfr_val(r));
#endif
}

int _mlgmp_fr_custom_compare(value a, value b);

void _mlgmp_fr_serialize(value v,
			unsigned long * wsize_32,
			unsigned long * wsize_64);
unsigned long _mlgmp_fr_deserialize(void * dst);

struct custom_operations _mlgmp_custom_fr =
  {
    field(identifier)  "Gmp.F.t",
    field(finalize)    &_mlgmp_fr_finalize,
    field(compare)     &_mlgmp_fr_custom_compare,
    field(hash)        custom_hash_default,
#if defined(SERIALIZE) && defined(USE_MPFR)
    field(serialize)   &_mlgmp_fr_serialize,
    field(deserialize) &_mlgmp_fr_deserialize
#else
    field(serialize)   custom_serialize_default,
    field(deserialize) custom_deserialize_default
#endif
  };

value _mlgmp_fr_create(value prec, value dummy)
{
#ifdef USE_MPFR
  CAMLparam2(prec, dummy);
  CAMLreturn(alloc_init_mpfr(prec));
#else
  unimplemented(create);
#endif
}

value _mlgmp_fr_from_z(value prec, value mode, value a)
{
#ifdef USE_MPFR
  CAMLparam2(prec, a);
  CAMLlocal1(r);
  r=alloc_init_mpfr(prec);
  mpfr_set_z(*mpfr_val(r), *mpz_val(a), Mode_val(mode));
  CAMLreturn(r);
#else
  unimplemented(from_z);
#endif
}

value _mlgmp_fr_from_q(value prec, value mode, value a)
{
#ifdef USE_MPFR
  CAMLparam2(prec, a);
  CAMLlocal1(r);
  r=alloc_init_mpfr(prec);
  mpfr_set_q(*mpfr_val(r), *mpq_val(a), Mode_val(mode));
  CAMLreturn(r);
#else
  unimplemented(from_q);
#endif
}

value _mlgmp_fr_from_si(value prec, value mode, value a)
{
#ifdef USE_MPFR
  CAMLparam2(prec, a);
  CAMLlocal1(r);
  r=alloc_init_mpfr(prec);
  mpfr_set_si(*mpfr_val(r), Long_val(a), Mode_val(mode));
  CAMLreturn(r);
#else
  unimplemented(from_si);
#endif
}

value _mlgmp_fr_from_float(value prec, value mode, value v)
{
#ifdef USE_MPFR
  CAMLparam2(prec, v);
  CAMLlocal1(r);
  r=alloc_init_mpfr(prec);
  mpfr_set_d(*mpfr_val(r), Double_val(v), Mode_val(mode));
  CAMLreturn(r);
#else
  unimplemented(from_float);
#endif
}

/*** Conversions */

value _mlgmp_fr_to_float(value mode, value v)
{
#ifdef USE_MPFR
  CAMLparam1(v);
  CAMLlocal1(r);
  r = copy_double(mpfr_get_d(*mpfr_val(v), Mode_val(mode)));
  CAMLreturn(r);
#else
  unimplemented(to_float);
#endif
}

value _mlgmp_fr_to_z_exp(value v)
{
#ifdef USE_MPFR
  CAMLparam1(v);
  CAMLlocal2(r, z);
  r = alloc_tuple(2);
  z = alloc_init_mpz();
  Store_field(r, 0, z);
  Store_field(r, 1, Val_int(mpfr_get_z_exp(*mpz_val(z), *mpfr_val(v))));
  CAMLreturn(r);
#else
  unimplemented(to_z_exp);
#endif
}

value _mlgmp_fr_to_string_exp_base_digits(value mode,
					  value base, value digits, value val)
{
#ifdef USE_MPFR
  CAMLparam4(mode, base, digits, val);
  CAMLlocal2(r, rs);
  mp_exp_t exponent;
  char *s= mpfr_get_str(NULL, &exponent, Int_val(base), Int_val(digits),
		       *mpfr_val(val), Mode_val(mode));
  rs=alloc_string(strlen(s));
  strcpy(String_val(rs), s);
  free(s);
  r=alloc_tuple(2);
  Store_field(r, 0, rs);
  Store_field(r, 1, Val_int(exponent));
  CAMLreturn(r);
#else
  unimplemented(to_string);
#endif
}

value _mlgmp_fr_from_string(value prec, value mode, value base, value str)
{
#ifdef USE_MPFR
  CAMLparam4(prec, mode, base, str);
  CAMLlocal1(r);
  r=alloc_init_mpfr(prec);
  mpfr_set_str(*mpfr_val(r), String_val(str), Int_val(base), Mode_val(mode));
  CAMLreturn(r);
#else
  unimplemented(from_string);
#endif
}

/*** Operations */
/**** Arithmetic */

#ifdef USE_MPFR

#define fr_binary_op_mpfr(op)	        		\
value _mlgmp_fr_##op(value prec, value mode, value a, value b)	\
{							\
  CAMLparam3(prec, a, b);                               \
  CAMLlocal1(r);                                        \
  r=alloc_init_mpfr(prec);	       		        \
  mpfr_##op(*mpfr_val(r), *mpfr_val(a), *mpfr_val(b), Mode_val(mode));	\
  CAMLreturn(r);	       				\
}

#define fr_binary_op_ui(op)	       			\
value _mlgmp_fr_##op(value prec, value mode, value a, value b)	\
{							\
  CAMLparam3(prec, a, b);                               \
  CAMLlocal1(r);                                        \
  r=alloc_init_mpfr(prec);	       		        \
  mpfr_##op(*mpfr_val(r), *mpfr_val(a), Long_val(b), Mode_val(mode));	\
  CAMLreturn(r);	       				\
}

#define fr_binary_ui_op(op)	       			\
value _mlgmp_fr_##op(value prec, value mode, value a, value b)	\
{							\
  CAMLparam3(prec, a, b);                               \
  CAMLlocal1(r);                                        \
  r=alloc_init_mpfr(prec);	       		        \
  mpfr_##op(*mpfr_val(r), Long_val(a), *mpfr_val(b), Mode_val(mode));	\
  CAMLreturn(r);	       				\
}

#define fr_unary_op(op)				\
value _mlgmp_fr_##op(value prec, value mode, value a)	\
{						\
  CAMLparam2(prec, a);				\
  CAMLlocal1(r);				\
  r=alloc_init_mpfr(prec);      			\
  mpfr_##op(*mpfr_val(r), *mpfr_val(a), Mode_val(mode));	    \
  CAMLreturn(r);				\
}

#define fr_rounding_op(op)				\
value _mlgmp_fr_##op(value prec, value a)	\
{						\
  CAMLparam2(prec, a);				\
  CAMLlocal1(r);				\
  r=alloc_init_mpfr(prec);      			\
  mpfr_##op(*mpfr_val(r), *mpfr_val(a));	    \
  CAMLreturn(r);				\
}

#else

#define fr_binary_op_mpfr(op)	        		\
value _mlgmp_fr_##op(value prec, value mode, value a, value b)	\
{							\
  unimplemented(op)                                     \
}

#define fr_binary_op_ui(op)	       			\
value _mlgmp_fr_##op(value prec, value mode, value a, value b)	\
{							\
  unimplemented(op)                                     \
}

#define fr_binary_ui_op(op)	       			\
value _mlgmp_fr_##op(value prec, value mode, value a, value b)	\
{							\
  unimplemented(op)                                     \
}

#define fr_unary_op(op)				\
value _mlgmp_fr_##op(value prec, value mode, value a)	\
{						\
  unimplemented(op)                             \
}

#define fr_rounding_op(op)				\
value _mlgmp_fr_##op(value prec, value mode, value a)	\
{						\
  unimplemented(op)                             \
}
#endif


#define fr_binary_op(op)				\
fr_binary_op_ui(op##_ui)				\
fr_binary_op_mpfr(op)

fr_binary_op(add)
fr_binary_op(sub)
fr_binary_op(mul)
fr_binary_op(div)
fr_binary_ui_op(ui_sub)
fr_binary_ui_op(ui_div)
fr_binary_op_ui(mul_2ui)
fr_binary_op_ui(div_2ui)
fr_binary_op(pow)
fr_binary_ui_op(ui_pow)

fr_unary_op(neg)
fr_unary_op(abs)

fr_unary_op(sin)
fr_unary_op(cos)
fr_unary_op(tan)
fr_unary_op(asin)
fr_unary_op(acos)
fr_unary_op(atan)
fr_unary_op(sinh)
fr_unary_op(cosh)
fr_unary_op(tanh)
fr_unary_op(asinh)
fr_unary_op(acosh)
fr_unary_op(atanh)

fr_unary_op(sqrt)
fr_unary_op(exp)
fr_unary_op(exp2)

fr_unary_op(rint)
fr_rounding_op(ceil)
fr_rounding_op(floor)
fr_rounding_op(trunc)

/*** Compare */

int _mlgmp_fr_custom_compare(value a, value b)
{
#ifdef USE_MPFR
  CAMLparam2(a, b);
  CAMLreturn(mpfr_cmp(*mpfr_val(a), *mpfr_val(b)));
#else
  unimplemented(create);
#endif
}

value _mlgmp_fr_cmp(value a, value b)
{
#ifdef USE_MPFR
  CAMLparam2(a, b);
  CAMLreturn(Val_int(mpfr_cmp(*mpfr_val(a), *mpfr_val(b))));
#else
  unimplemented(create);
#endif
}

value _mlgmp_fr_cmp_si(value a, value b)
{
#ifdef USE_MPFR
  CAMLparam2(a, b);
  CAMLreturn(Val_int(mpfr_cmp_si(*mpfr_val(a), Long_val(b))));
#else
  unimplemented(create);
#endif
}

value _mlgmp_fr_sgn(value a)
{
#ifdef USE_MPFR
  CAMLparam1(a);
  CAMLreturn(Val_int(mpfr_sgn(*mpfr_val(a))));
#else
  unimplemented(create);
#endif
}

value _mlgmp_fr_is_nan(value a)
{
#ifdef USE_MPFR
  CAMLparam1(a);
  CAMLreturn(mpfr_nan_p(*mpfr_val(a)) ? Val_true : Val_false);
#else
  unimplemented(create);
#endif
}

value _mlgmp_fr_eq(value a, value b, value nbits)
{
#ifdef USE_MPFR
  CAMLparam3(a, b, nbits);
  CAMLreturn(mpfr_eq(*mpfr_val(a), *mpfr_val(b), Int_val(nbits))
	     ? Val_true : Val_false);
#else
  unimplemented(create);
#endif
}

fr_binary_op_mpfr(reldiff)


/*** Random */
value _mlgmp_fr_urandomb(value prec, value state)
{
#ifdef USE_MPFR
  CAMLparam2(prec, state);
  CAMLlocal1(r);
  r = alloc_init_mpfr(prec);
  mpfr_urandomb(*mpfr_val(r), *randstate_val(state));
  CAMLreturn(r);
#else
  unimplemented(urandomb);
#endif
}

/* Old mpfr - no longer exists in 3.0.1-p3
value _mlgmp_fr_random(value prec)
{
#ifdef USE_MPFR
  CAMLparam1(prec);
  CAMLlocal1(r);
  r = alloc_init_mpfr(prec);
  mpfr_random(*mpfr_val(r));
  CAMLreturn(r);
#else
  unimplemented(urandomb);
#endif
}
*/

/* Old MPFR -no longer exists in 20011026
value _mlgmp_fr_srandom(value seed)
{
#ifdef USE_MPFR
  mpfr_srandom(Int_val(seed));
  return Val_unit;
#else
  unimplemented(urandomb);
#endif
}
*/

/* Old mpfr - no longer exists in 3.0.1-p3
value _mlgmp_fr_random2(value prec, value nlimbs, value max_exp)
{
#ifdef USE_MPFR
  CAMLparam3(prec, nlimbs, max_exp);
  CAMLlocal1(r);
  r = alloc_init_mpfr(prec);
  mpfr_random2(*mpfr_val(r), Int_val(nlimbs), Int_val(max_exp));
  CAMLreturn(r);
#else
  unimplemented(urandomb);
#endif
}
*/

/*** Serialization */
value _mlgmp_fr_initialize(void)
{
#ifdef USE_MPFR
  CAMLparam0();
  register_custom_operations(& _mlgmp_custom_f);
  CAMLreturn(Val_unit);
#endif
}

#if defined(SERIALIZE) && defined(USE_MPFR)
void _mlgmp_fr_serialize(value v,
			unsigned long * wsize_32,
			unsigned long * wsize_64)
{
  CAMLparam1(v);
  char *s;
  int len;
  mp_exp_t exponent;
  char exponent_buf[10];

  *wsize_32 = MPFR_SIZE_ARCH32;
  *wsize_64 = MPFR_SIZE_ARCH64;

  serialize_int_4(mpfr_get_prec(*mpfr_val(v)));

  s = mpfr_get_str (NULL, &exponent, 16, 0, *mpfr_val(v), GMP_RNDN);
  len = strlen(s);
  serialize_int_4(len + 11);

  serialize_block_1("0.", 2);
  serialize_block_1(s, len);
  free(s);

  sprintf(exponent_buf, "@%08lx", (exponent & 0xFFFFFFFFUL));
  serialize_block_1(exponent_buf, 9);

  CAMLreturn0;
}

unsigned long _mlgmp_fr_deserialize(void * dst)
{
  char *s;
  int len;

  mpfr_init2(*((mpfr_t*) dst), deserialize_uint_4());

  len = deserialize_uint_4();
  s = malloc(len+1);
  deserialize_block_1(s, len);
  s[len] = 0;
  mpfr_set_str (*((mpfr_t*) dst), s, 16, GMP_RNDN);
  free(s);

  return sizeof(mpfr_t);
}
#endif
