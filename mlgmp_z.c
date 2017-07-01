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
#include "mlgmp.h"
#include "conversions.c"

#define MODULE "Gmp.Z."

/*** Allocation functions */

void _mlgmp_z_finalize(value r)
{
  mpz_clear(*mpz_val(r));
}

int _mlgmp_z_custom_compare(value a, value b);
void _mlgmp_z_serialize(value v,
			unsigned long * wsize_32,
			unsigned long * wsize_64);
unsigned long _mlgmp_z_deserialize(void * dst);
long _mlgmp_z_hash(value v);

struct custom_operations _mlgmp_custom_z =
  {
    field(identifier)  "Gmp.Z.t",
    field(finalize)    &_mlgmp_z_finalize,
    field(compare)     &_mlgmp_z_custom_compare,
    field(hash)        &_mlgmp_z_hash,
#ifdef SERIALIZE
    field(serialize)   &_mlgmp_z_serialize,
    field(deserialize) &_mlgmp_z_deserialize
#else
    field(serialize)   custom_serialize_default,
    field(deserialize) custom_deserialize_default
#endif
  };

value _mlgmp_z_create(void)
{
  CAMLparam0();
  CAMLreturn(alloc_init_mpz());
}

value _mlgmp_z_copy(value from)
{
  CAMLparam1(from);
  CAMLlocal1(r);
  r = alloc_mpz();
  mpz_init_set(*mpz_val(r), *mpz_val(from));
  CAMLreturn(r);
}

value _mlgmp_z_from_int(value ml_val)
{
  CAMLparam1(ml_val);
  CAMLlocal1(r);
  r=alloc_mpz();
  mpz_init_set_si(*mpz_val(r), Long_val(ml_val));
  CAMLreturn(r);
}

value _mlgmp_z_from_string_base(value base, value ml_val)
{
  CAMLparam2(base, ml_val);
  CAMLlocal1(r);
  r=alloc_mpz();
  mpz_init_set_str(*mpz_val(r), String_val(ml_val), Int_val(base));
  CAMLreturn(r);
}

value _mlgmp_z_from_float(value ml_val)
{
  CAMLparam1(ml_val);
  CAMLlocal1(r);
  r=alloc_mpz();
  mpz_init_set_d(*mpz_val(r), Double_val(ml_val));
  CAMLreturn(r);
}

value _mlgmp_z2_from_int(value r, value ml_val)
{
  CAMLparam2(r, ml_val);
  mpz_init_set_si(*mpz_val(r), Long_val(ml_val));
  CAMLreturn(Val_unit);
}

value _mlgmp_z2_from_string_base(value r, value base, value ml_val)
{
  CAMLparam3(r, base, ml_val);
  mpz_init_set_str(*mpz_val(r), String_val(ml_val), Int_val(base));
  CAMLreturn(Val_unit);
}

value _mlgmp_z2_from_float(value r, value ml_val)
{
  CAMLparam2(r, ml_val);
  mpz_init_set_d(*mpz_val(r), Double_val(ml_val));
  CAMLreturn(Val_unit);
}

/*** Conversions */

value _mlgmp_z_to_string_base(value ml_base, value ml_val)
{
  int base;
  char *s;

  CAMLparam2(ml_base, ml_val);
  CAMLlocal1(r);
  base=Int_val(ml_base);

  /* This is sub-optimal, but using mpz_sizeinbase would
     need a means of shortening the length of a pre-allocated
     Caml string (mpz_sizeinbase sometimes overestimates lengths). */
  s=mpz_get_str(NULL, base, *mpz_val(ml_val));
  r=alloc_string(strlen(s));
  strcpy(String_val(r), s);
  free(s);

  CAMLreturn(r);
}

value _mlgmp_z_to_int(value ml_val)
{
  CAMLparam1(ml_val);
  CAMLreturn(Val_int(mpz_get_si(* mpz_val(ml_val))));
}

value _mlgmp_z_to_float(value v)
{
  CAMLparam1(v);
  CAMLlocal1(r);
  r = copy_double(mpz_get_d(*mpz_val(v)));
  CAMLreturn(r);
}

/*** Operations */
/**** Arithmetic */

#define z_binary_op_ui(op)                              \
value _mlgmp_z_##op(value a, value b)		        \
{							\
  CAMLparam2(a, b);                                     \
  CAMLlocal1(r);                                        \
  r=alloc_init_mpz();				        \
  mpz_##op(*mpz_val(r), *mpz_val(a), Long_val(b));	\
  CAMLreturn(r);					\
}                                                       \
                                                        \
value _mlgmp_z2_##op(value r, value a, value b)		\
{							\
  CAMLparam3(r, a, b);                                  \
  mpz_##op(*mpz_val(r), *mpz_val(a), Long_val(b));	\
  CAMLreturn(Val_unit);					\
}

#define z_binary_op_mpz(op)				\
value _mlgmp_z_##op(value a, value b)			\
{							\
  CAMLparam2(a, b);                                     \
  CAMLlocal1(r);                                        \
  r=alloc_init_mpz();				        \
  mpz_##op(*mpz_val(r), *mpz_val(a), *mpz_val(b));	\
  CAMLreturn(r);	       				\
}                                                       \
                                                        \
value _mlgmp_z2_##op(value r, value a, value b)	       	\
{							\
  CAMLparam3(r, a, b);                                  \
  mpz_##op(*mpz_val(r), *mpz_val(a), *mpz_val(b));	\
  CAMLreturn(Val_unit);	       				\
}

#define z_binary_op(op)				\
z_binary_op_mpz(op)				\
z_binary_op_ui(op##_ui)

z_binary_op(add)
z_binary_op(sub)
z_binary_op(mul)

/**** Powers */
z_binary_op_ui(pow_ui)


value _mlgmp_z_powm_ui(value a, value b, value modulus)
{
  CAMLparam3(a, b, modulus);
  CAMLlocal1(r);
  r=alloc_init_mpz();
  mpz_powm_ui(*mpz_val(r), *mpz_val(a), Long_val(b), *mpz_val(modulus));
  CAMLreturn(r);
}

value _mlgmp_z_ui_pow_ui(value a, value b)
{
  CAMLparam2(a, b);
  CAMLlocal1(r);
  r=alloc_init_mpz();
  mpz_ui_pow_ui(*mpz_val(r), Long_val(a), Long_val(b));
  CAMLreturn(r);
}

value _mlgmp_z_powm(value a, value b, value modulus)
{
  CAMLparam3(a, b, modulus);
  CAMLlocal1(r);
  r=alloc_init_mpz();
  mpz_powm(*mpz_val(r), *mpz_val(a), *mpz_val(b), *mpz_val(modulus));
  CAMLreturn(r);
}

value _mlgmp_z2_powm_ui(value r, value a, value b, value modulus)
{
  CAMLparam4(r, a, b, modulus);
  mpz_powm_ui(*mpz_val(r), *mpz_val(a), Long_val(b), *mpz_val(modulus));
  CAMLreturn(Val_unit);
}

value _mlgmp_z2_ui_pow_ui(value r, value a, value b)
{
  CAMLparam3(r, a, b);
  mpz_ui_pow_ui(*mpz_val(r), Long_val(a), Long_val(b));
  CAMLreturn(Val_unit);
}

value _mlgmp_z2_powm(value r, value a, value b, value modulus)
{
  CAMLparam4(r, a, b, modulus);
  mpz_powm(*mpz_val(r), *mpz_val(a), *mpz_val(b), *mpz_val(modulus));
  CAMLreturn(Val_unit);
}

/**** Unary */
#define z_unary_op(op)				\
value _mlgmp_z_##op(value a)			\
{						\
  CAMLparam1(a);				\
  CAMLlocal1(r);				\
  r=alloc_init_mpz();   			\
  mpz_##op(*mpz_val(r), *mpz_val(a));		\
  CAMLreturn(r);				\
}                                               \
                                                \
value _mlgmp_z2_##op(value r, value a)	        \
{						\
  CAMLparam2(r, a);				\
  mpz_##op(*mpz_val(r), *mpz_val(a));		\
  CAMLreturn(Val_unit);				\
}

z_unary_op(neg)
z_unary_op(abs)

/**** Roots */

/* Negative ?*/
z_unary_op(sqrt)

value _mlgmp_z_sqrtrem(value a)
{
  CAMLparam1(a);
  CAMLlocal3(q, r, qr);
  q=alloc_init_mpz();
  r=alloc_init_mpz();

  mpz_sqrtrem(*mpz_val(q), *mpz_val(r), *mpz_val(a));

  qr=alloc_tuple(2);
  Store_field(qr, 0, q);
  Store_field(qr, 1, r);
  CAMLreturn(qr);
}
									
z_binary_op_ui(root)

#define z_unary_p(name)				\
value _mlgmp_z_##name(value a)			\
{						\
  CAMLparam1(a);                                \
  CAMLreturn(Val_bool(mpz_##name(*mpz_val(a))));\
}

z_unary_p(perfect_power_p)
z_unary_p(perfect_square_p)

/**** Division */

/* IMPORTANT NOTE:
Storing mpz_val(d) into a temporary pointer won't work because the GC
may move the data when allocating q and r.
*/

#define z_xdivision_op(kind)						\
value _mlgmp_z_##kind##div_qr(value n, value d)				\
{									\
  CAMLparam2(n, d);							\
  CAMLlocal3(q, r, qr);							\
  if (! mpz_sgn(*mpz_val(d)))						\
    division_by_zero();							\
									\
  q=alloc_init_mpz();							\
  r=alloc_init_mpz();							\
									\
  mpz_##kind##div_qr(*mpz_val(q), *mpz_val(r), *mpz_val(n), *mpz_val(d));\
									\
  qr=alloc_tuple(2);							\
  Store_field(qr, 0, q);						\
  Store_field(qr, 1, r);						\
  CAMLreturn(qr);						        \
}									\
									\
value _mlgmp_z_##kind##div_q(value n, value d)				\
{									\
  CAMLparam2(n, d);                                                     \
  CAMLlocal1(q);						      	\
									\
  if (! mpz_sgn(*mpz_val(d)))						\
    division_by_zero();							\
									\
  q=alloc_init_mpz();							\
									\
  mpz_##kind##div_q(*mpz_val(q), *mpz_val(n), *mpz_val(d));	       	\
									\
  CAMLreturn(q);	       						\
}									\
									\
value _mlgmp_z2_##kind##div_q(value q, value n, value d)		\
{									\
  CAMLparam3(q, n, d);                                                  \
									\
  if (! mpz_sgn(*mpz_val(d)))						\
    division_by_zero();							\
									\
  mpz_##kind##div_q(*mpz_val(q), *mpz_val(n), *mpz_val(d));	       	\
									\
  CAMLreturn(Val_unit);	       						\
}									\
									\
value _mlgmp_z_##kind##div_r(value n, value d)				\
{									\
  CAMLparam2(n, d);                                                     \
  CAMLlocal1(r);						      	\
									\
  if (! mpz_sgn(*mpz_val(d)))						\
    division_by_zero();							\
									\
  r=alloc_init_mpz();							\
									\
  mpz_##kind##div_r(*mpz_val(r), *mpz_val(n), *mpz_val(d));	       	\
									\
  CAMLreturn(r);	       						\
}									\
									\
value _mlgmp_z2_##kind##div_r(value r, value n, value d)      		\
{									\
  CAMLparam3(r, n, d);                                                     \
									\
  if (! mpz_sgn(*mpz_val(d)))						\
    division_by_zero();							\
									\
  mpz_##kind##div_r(*mpz_val(r), *mpz_val(n), *mpz_val(d));	       	\
									\
  CAMLreturn(Val_unit);	       						\
}									\
									\
value _mlgmp_z_##kind##div_qr_ui(value n, value d)			\
{									\
  CAMLparam2(n, d);                                                     \
  CAMLlocal3(q, r, qr);							\
  unsigned long int ui_d = Long_val(d);					\
									\
  if (! ui_d) division_by_zero();					\
									\
  q=alloc_init_mpz();							\
  r=alloc_init_mpz();							\
									\
  mpz_##kind##div_qr_ui(*mpz_val(q), *mpz_val(r), *mpz_val(n), ui_d);	\
									\
  qr=alloc_tuple(2);							\
  Store_field(qr, 0, q);						\
  Store_field(qr, 1, r);						\
  CAMLreturn(qr);	       						\
}									\
									\
value _mlgmp_z_##kind##div_q_ui(value n, value d)			\
{									\
  CAMLparam2(n, d);                                                     \
  CAMLlocal1(q);       							\
  unsigned long int ui_d = Long_val(d);					\
									\
 if (! ui_d) division_by_zero();					\
									\
  q=alloc_init_mpz();							\
									\
  mpz_##kind##div_q_ui(*mpz_val(q), *mpz_val(n), ui_d);			\
									\
  CAMLreturn(q);	       						\
}									\
									\
value _mlgmp_z2_##kind##div_q_ui(value q, value n, value d)		\
{									\
  CAMLparam3(q, n, d);                                                     \
  unsigned long int ui_d = Long_val(d);					\
									\
 if (! ui_d) division_by_zero();					\
									\
  mpz_##kind##div_q_ui(*mpz_val(q), *mpz_val(n), ui_d);			\
									\
  CAMLreturn(Val_unit);	       						\
}									\
									\
value _mlgmp_z_##kind##div_r_ui(value n, value d)			\
{									\
  CAMLparam2(n, d);                                                     \
  CAMLlocal1(r);       							\
  unsigned long int ui_d = Long_val(d);					\
									\
  if (! ui_d) division_by_zero();					\
									\
  r=alloc_init_mpz();							\
									\
  mpz_##kind##div_r_ui(*mpz_val(r), *mpz_val(n), ui_d);			\
									\
  CAMLreturn(r);	       						\
}									\
									\
value _mlgmp_z2_##kind##div_r_ui(value r, value n, value d)		\
{									\
  CAMLparam3(r, n, d);                                                  \
  unsigned long int ui_d = Long_val(d);					\
									\
 if (! ui_d) division_by_zero();					\
									\
  mpz_##kind##div_r_ui(*mpz_val(r), *mpz_val(n), ui_d);			\
									\
  CAMLreturn(Val_unit);	       						\
}									\
									\
value _mlgmp_z_##kind##div_ui(value n, value d)				\
{									\
  CAMLparam2(n, d);                                                     \
  unsigned long int ui_d = Long_val(d);					\
									\
  if (! ui_d) division_by_zero();					\
									\
  CAMLreturn(Val_int(mpz_##kind##div_ui(*mpz_val(n), ui_d)));	        \
}

z_xdivision_op(t)
z_xdivision_op(f)
z_xdivision_op(c)

#define z_division_op(op)			\
value _mlgmp_z_##op(value n, value d)		\
{						\
  CAMLparam2(n, d);				\
  CAMLlocal1(q);				\
						\
  if (! mpz_sgn(*mpz_val(d)))			\
    division_by_zero();				\
						\
  q=alloc_init_mpz();				\
						\
  mpz_##op(*mpz_val(q), *mpz_val(n), *mpz_val(d)); \
						\
  CAMLreturn(q);				\
}						\
						\
value _mlgmp_z2_##op(value q, value n, value d)	\
{						\
  CAMLparam3(q, n, d);				\
						\
  if (! mpz_sgn(*mpz_val(d)))			\
    division_by_zero();				\
						\
  mpz_##op(*mpz_val(q), *mpz_val(n), *mpz_val(d)); \
						\
  CAMLreturn(Val_unit);				\
}

#define z_division_op_ui(op)			\
value _mlgmp_z_##op(value n, value d)		\
{						\
  CAMLparam2(n, d);				\
  CAMLlocal1(q);				\
  unsigned long ld = Long_val(d);			\
						\
  if (! ld)	                 		\
    division_by_zero();				\
						\
  q=alloc_init_mpz();				\
						\
  mpz_##op(*mpz_val(q), *mpz_val(n), ld);	\
						\
  CAMLreturn(q);				\
}						\
						\
value _mlgmp_z2_##op(value q, value n, value d)	\
{						\
  CAMLparam3(q, n, d);				\
  unsigned long ld = Long_val(d);			\
						\
  if (! ld)			                \
    division_by_zero();				\
						\
  mpz_##op(*mpz_val(q), *mpz_val(n), ld);	\
						\
  CAMLreturn(Val_unit);				\
}

z_division_op(divexact)
z_division_op(mod)
z_division_op_ui(mod_ui)

/*** Shift ops */
#define z_shift_op(type)				\
value _mlgmp_z_##type(value a, value shift)		\
{                                                       \
  CAMLparam2(a, shift);                                 \
  CAMLlocal1(r);					\
  r=alloc_init_mpz();   				\
  mpz_##type(*mpz_val(r), *mpz_val(a), Int_val(shift));	\
  CAMLreturn(r);       					\
}                                                       \
                                                        \
value _mlgmp_z2_##type(value r, value a, value shift)	\
{                                                       \
  CAMLparam3(r, a, shift);                              \
  mpz_##type(*mpz_val(r), *mpz_val(a), Int_val(shift));	\
  CAMLreturn(Val_unit);     				\
}

#define z_shift_op_unimplemented(type)			\
value _mlgmp_z_##type(value a, value shift)		\
{                                                       \
  CAMLparam2(a, shift);                                 \
  CAMLreturn0();       					\
}                                                       \
                                                        \
value _mlgmp_z2_##type(value r, value a, value shift)	\
{                                                       \
  CAMLparam3(r, a, shift);                              \
  unimplemented(z2_##type);                             \
  CAMLreturn0();     				        \
}

z_shift_op(mul_2exp)
z_shift_op(tdiv_q_2exp)
z_shift_op(tdiv_r_2exp)
z_shift_op(fdiv_q_2exp)
z_shift_op(fdiv_r_2exp)

#if __GNU_MP_VERSION >= 4
z_shift_op(cdiv_q_2exp)
z_shift_op(cdiv_r_2exp)
#else
z_shift_op_unimplemented(cdiv_q_2exp)
z_shift_op_unimplemented(cdiv_r_2exp)
#endif

/*** Comparisons */

int _mlgmp_z_custom_compare(value a, value b)
{
  CAMLparam2(a, b);
  CAMLreturn(mpz_cmp(*mpz_val(a), *mpz_val(b)));
}

value _mlgmp_z_compare(value a, value b)
{
  CAMLparam2(a, b);
  CAMLreturn(Val_int(mpz_cmp(*mpz_val(a), *mpz_val(b))));
}

value _mlgmp_z_compare_si(value a, value b)
{
  CAMLparam2(a, b);
  CAMLreturn(Val_int(mpz_cmp_si(*mpz_val(a), Long_val(b))));
}

/*** Number theory */

value _mlgmp_z_probab_prime_p(value n, value reps)
{
  CAMLparam2(n, reps);
  CAMLreturn(Val_bool(mpz_probab_prime_p(*mpz_val(n), Int_val(reps))));
}

z_unary_op(nextprime)

z_binary_op(gcd)
z_binary_op_mpz(lcm)

value  _mlgmp_z_gcdext(value a, value b)
{
  CAMLparam2(a, b);
  CAMLlocal4(g, s, t, r);
  g=alloc_init_mpz();  
  s=alloc_init_mpz();  
  t=alloc_init_mpz();
  mpz_gcdext(*mpz_val(g), *mpz_val(s), *mpz_val(t), *mpz_val(a), *mpz_val(b));
  r=alloc_tuple(3);
  Store_field(r, 0, g);
  Store_field(r, 1, s);
  Store_field(r, 2, t);
  CAMLreturn(r);
}						     

value  _mlgmp_z_invert(value a, value b)
{
  CAMLparam2(a, b);
  CAMLlocal2(i, r);
  i = alloc_init_mpz();
  if (! mpz_invert(*mpz_val(i),*mpz_val(a), *mpz_val(b)))
    {
      r=Val_false;
    }
  else
    {
      r=alloc_tuple(1);
      Store_field(r, 0, i);
    }
  CAMLreturn(r);
}						     

#define z_int_binary_op(op)					\
value _mlgmp_z_##op(value a, value b)				\
{								\
  CAMLparam2(a, b);						\
  CAMLreturn(Val_int(mpz_##op(*mpz_val(a), *mpz_val(b))));	\
}

z_int_binary_op(legendre)
z_int_binary_op(jacobi)

value _mlgmp_z_kronecker_si(value a, value b)
{
  CAMLparam2(a, b);
  CAMLreturn(Val_int(mpz_kronecker_si(*mpz_val(a), Long_val(b))));
}

value _mlgmp_z_si_kronecker(value a, value b)
{
  CAMLparam2(a, b);
  CAMLreturn(Val_int(mpz_si_kronecker(Long_val(a), *mpz_val(b))));
}

value _mlgmp_z_remove(value a, value b)
{
  int x;
  CAMLparam2(a, b);
  CAMLlocal2(f, r);
  f = alloc_init_mpz();
  x = mpz_remove(*mpz_val(f), *mpz_val(a), *mpz_val(b));
  r=alloc_tuple(2);
  Store_field(r, 0, f);
  Store_field(r, 1, Val_int(x));
  CAMLreturn(r);
}

#define z_unary_op_ui(op)			\
value _mlgmp_z_##op(value a)			\
{						\
  CAMLparam1(a);				\
  CAMLlocal1(r);				\
  r = alloc_init_mpz();				\
  mpz_##op(*mpz_val(r), Long_val(a));		\
  CAMLreturn(r);				\
}

z_unary_op_ui(fac_ui)
z_unary_op_ui(fib_ui)
z_binary_op_ui(bin_ui)

value _mlgmp_z_bin_uiui(value n, value k)
{
  CAMLparam2(n, k);
  CAMLlocal1(r);
  r = alloc_init_mpz();
  mpz_bin_uiui(*mpz_val(r), Long_val(n), Long_val(k));
  CAMLreturn(r);
}

#define z_int_unary_op(op)			\
value _mlgmp_z_##op(value a)			\
{						\
  CAMLparam1(a);				\
  CAMLreturn(Val_int(mpz_##op(*mpz_val(a))));	\
}

z_int_unary_op(sgn)

z_binary_op_mpz(and)
z_binary_op_mpz(ior)
z_binary_op_mpz(xor)
z_unary_op(com)

z_int_unary_op(popcount)
z_int_binary_op(hamdist)

#define z_int_binary_op_ui(op)					\
value _mlgmp_z_##op(value a, value b)				\
{								\
  CAMLparam2(a, b);						\
  CAMLreturn(Val_int(mpz_##op(*mpz_val(a), Long_val(b))));	\
}

z_int_binary_op_ui(scan0)
z_int_binary_op_ui(scan1)

/*** Random */
#define z_random_op_ui(op)					\
value _mlgmp_z_##op(value state, value n)			\
{								\
  CAMLparam2(state, n);						\
  CAMLlocal1(r);						\
  r = alloc_init_mpz();						\
  mpz_##op(*mpz_val(r), *randstate_val(state), Long_val(n));	\
  CAMLreturn(r);						\
}

#define z_random_op(op)			        		\
value _mlgmp_z_##op(value state, value n)			\
{								\
  CAMLparam2(state, n);						\
  CAMLlocal1(r);						\
  r = alloc_init_mpz();						\
  mpz_##op(*mpz_val(r), *randstate_val(state), *mpz_val(n));	\
  CAMLreturn(r);						\
}

z_random_op_ui(urandomb)
z_random_op(urandomm)
z_random_op_ui(rrandomb)

/*** Serialization */
value _mlgmp_z_initialize()
{
  CAMLparam0();
  register_custom_operations(& _mlgmp_custom_z);
  CAMLreturn(Val_unit);
}

#ifdef SERIALIZE
void _mlgmp_z_serialize(value v,
			unsigned long * wsize_32,
			unsigned long * wsize_64)
{
  CAMLparam1(v);
  char *s;
  int len;

  *wsize_32 = MPZ_SIZE_ARCH32;
  *wsize_64 = MPZ_SIZE_ARCH64;

  s = mpz_get_str (NULL, 16, *mpz_val(v));
  len = strlen(s);
  serialize_int_4(len);
  serialize_block_1(s, len);

  free(s);
  CAMLreturn0;
}

unsigned long _mlgmp_z_deserialize(void * dst)
{
  char *s;
  int len;

  len = deserialize_uint_4();
  s = malloc(len+1);
  deserialize_block_1(s, len);
  s[len] = 0;
  mpz_init_set_str (*((mpz_t*) dst), s, 16);
  free(s);

  return sizeof(mpz_t);
}
#endif

/* Hash */

long _mlgmp_z_hash(value v)
{
  CAMLparam1(v);
  mpz_t dummy;
  long r;
  mpz_init(dummy);

  r = mpz_mod_ui(dummy, *mpz_val(v), HASH_MODULUS);

  mpz_clear(dummy);
  CAMLreturn(r);
}
