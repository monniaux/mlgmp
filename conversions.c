/*
 * ML GMP - Interface between Objective Caml and GNU MP
 * Copyright (C) 2001 David MONNIAUX
 * 
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 published by the Free Software Foundation,
 * or any more recent version published by the Free Software
 * Foundation, at your choice.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * See the GNU Library General Public License version 2 for more details
 * (enclosed in the file LGPL).
 *
 * As a special exception to the GNU Library General Public License, you
 * may link, statically or dynamically, a "work that uses the Library"
 * with a publicly distributed version of the Library to produce an
 * executable file containing portions of the Library, and distribute
 * that executable file under terms of your choice, without any of the
 * additional requirements listed in clause 6 of the GNU Library General
 * Public License.  By "a publicly distributed version of the Library",
 * we mean either the unmodified Library as distributed by INRIA, or a
 * modified version of the Library that is distributed under the
 * conditions defined in clause 3 of the GNU Library General Public
 * License.  This exception does not however invalidate any other reasons
 * why the executable file might be covered by the GNU Library General
 * Public License.
 */

#include <assert.h>

struct custom_operations _mlgmp_custom_z;

static inline gmp_randstate_t *randstate_val(value val)
{
  return ((gmp_randstate_t *) (Data_custom_val(val)));
}

static inline int Int_option_val(value val, int default_val)
{
  if (val == Val_int(0)) return default_val;
  return Int_val(Field(val, 0));
}

static inline mpz_t * mpz_val (value val)
{
  return ((mpz_t *) (Data_custom_val(val)));
}

static inline value alloc_mpz (void)
{
  return alloc_custom(&_mlgmp_custom_z,
		       sizeof(mpz_t),
		       0,
		       1);
}

static inline value alloc_init_mpz (void)
{
  value r= alloc_mpz();
  mpz_init(*mpz_val(r));
  return r;
}

#ifdef PRAGMA_INLINE
#pragma inline(Int_option_val, mpz_val, alloc_mpz, alloc_init_mpz)
#endif

struct custom_operations _mlgmp_custom_q;

static inline mpq_t * mpq_val (value val)
{
  return ((mpq_t *) (Data_custom_val(val)));
}

static inline value alloc_mpq (void)
{
  return alloc_custom(&_mlgmp_custom_q,
		       sizeof(mpq_t),
		       0,
		       1);
}

static inline value alloc_init_mpq (void)
{
  value r= alloc_mpq();
  mpq_init(*mpq_val(r));
  return r;
}

#ifdef PRAGMA_INLINE
#pragma inline(mpq_val, alloc_mpq, alloc_init_mpq)
#endif

struct custom_operations _mlgmp_custom_f;

static inline mpf_t * mpf_val (value val)
{
  return ((mpf_t *) (Data_custom_val(val)));
}

static inline value alloc_mpf (void)
{
  return alloc_custom(&_mlgmp_custom_f,
		       sizeof(mpf_t),
		       0,
		       1);
}

static inline value alloc_init_mpf (value prec)
{
  value r= alloc_mpf();
  mpf_init2(*mpf_val(r), Int_val(prec));
  return r;
}


struct custom_operations _mlgmp_custom_fr;

#ifdef USE_MPFR
static inline mpfr_t * mpfr_val (value val)
{
  return ((mpfr_t *) (Data_custom_val(val)));
}

static inline mp_rnd_t Mode_val (value val)
{
  return (mp_rnd_t) (Int_val(val));
}

static inline value alloc_mpfr (void)
{
  return alloc_custom(&_mlgmp_custom_fr,
		       sizeof(mpfr_t),
		       0,
		       1);
}

static inline value alloc_init_mpfr (value prec)
{
  value r= alloc_mpfr();
  mpfr_init2(*mpfr_val(r), Int_val(prec));
  return r;
}
#endif
