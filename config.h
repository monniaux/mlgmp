#define SERIALIZE
#define USE_MPFR
#define NDEBUG
#undef TRACE

#include <stdint.h>

#include <gmp.h>
#ifdef USE_MPFR
#include <mpfr.h>
#endif

/* This is the largest prime less than 2^32 */
#define HASH_MODULUS 4294967291UL

#ifdef TRACE
#define trace(x) do { fprintf(stderr, "mlgmp: %s%s\n", MODULE, #x);\
                      fflush(stderr); } while(0)
#else
#define trace(x)
#endif

#ifdef __GNUC__
#define noreturn __attribute__((noreturn))
#else
#define noreturn
#endif

/* In C99 or recent versions of gcc,
   - you can specify which field you want to initialize
   - you have "inline". */

#if defined(__GNUC__) || (defined(__STDC__) && __STDC_VERSION__ >= 199901L)
#define field(x) .x =
#else
#define field(x)
#define inline
#endif

#ifdef SERIALIZE
/* Sizes of types on arch 32/ arch 64 */

/* THOSE SIZES ARE A HACK. */

/* __mpz_struct = 2*int + ptr */
#define MPZ_SIZE_ARCH32 12
#define MPZ_SIZE_ARCH64 16

/* __mpq_struct = 2 * __mpz_struct */
#define MPQ_SIZE_ARCH32 (2 * MPZ_SIZE_ARCH32)
#define MPQ_SIZE_ARCH64 (2 * MPZ_SIZE_ARCH64)

/* __mpf_struct = 3 * int + ptr */
#define MPF_SIZE_ARCH32 16
#define MPF_SIZE_ARCH64 24

/* __mpfr_struct = 3 * int + ptr */
#define MPFR_SIZE_ARCH32 16
#define MPFR_SIZE_ARCH64 24

extern void serialize_int_4(int32_t i);
extern void serialize_block_1(void * data, long len);

extern uint32_t deserialize_uint_4(void);
extern int32_t deserialize_sint_4(void);
extern void deserialize_block_1(void * data, long len);

#endif /* SERIALIZE */
