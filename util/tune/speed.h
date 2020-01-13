/*
 * speed.h
 *
 *  Created on: Jan 6, 2020, 12:44:30 PM
 *      Author: Joshua Fehrenbach
 *
 * Everything here is adapted from GMP tuning functions
 */

#ifndef TUNE_SPEED_H_
#define TUNE_SPEED_H_

#include <stdlib.h>
#include <gmp.h>
#include "crypto.h"
#include "tmp-alloc.h"

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#define numberof(x)  (sizeof (x) / sizeof ((x)[0]))

#define CACHE_LINE_SIZE  64
#define CACHE_LINE_MASK  (CACHE_LINE_SIZE - 1)

#define SPEED_TMP_ALLOC(name,ptr,size,align) do { \
	void   *__ptr; \
	size_t  __ptr_align, __ptr_add; \
	__ptr = TMP_ALLOC(name, size + CACHE_LINE_MASK); \
	__ptr_align = (__ptr - (void *) NULL); \
	__ptr_add = ((align) - __ptr_align) & CACHE_LINE_MASK; \
	(ptr) = __ptr + __ptr_add; \
} while (0)

extern double speed_unittime;
extern double speed_cycletime;
extern int    speed_precision;
extern char   speed_time_string[];

void speed_time_init(void);
void speed_cycletime_fail(const char *str);
void speed_cycletime_init(void);
void speed_cycletime_need_cycles(void);
void speed_cycletime_need_seconds(void);
void speed_starttime(void);
double speed_endtime(void);

double speed_current_timestamp(void);

typedef void (*random_function_t) (void *ctx, mpz_ptr dst, mp_size_t size);
typedef void (*random_close_function_t) (void *ctx);

struct speed_params {
	random_function_t random;
	random_close_function_t close;
	void *rand_ctx;
	unsigned reps;	/* how many times to run the routine */
	mpz_t xp;				/* first argument */
	mpz_t yp;				/* second argument */
	mp_size_t  size;		/* size of both arguments */
	mp_size_t  align_xp;	/* alignment of xp */
	mp_size_t  align_yp;	/* alignment of yp */

	double     time_divisor; /* optionally set by the speed routine */
};

typedef double (*speed_function_t) (struct speed_params *s);

struct param_t {
	const char              *name;
	speed_function_t        function;
	speed_function_t        function2;
	double                  step_factor;    /* how much to step relatively */
	int                     step;           /* how much to step absolutely */
	double                  function_fudge; /* multiplier for "function" speeds. */
	int                     stop_since_change;
	double                  stop_factor;
	mp_size_t               min_size;
	int                     min_is_always;
	mp_size_t               max_size;
	mp_size_t               check_size;
	mp_size_t               size_extra;
	int                     noprint;
};

#if defined(__cplusplus)
extern "C" {
#endif

void
__crypto_tune_assert_fail(const char *fn, int ln, const char *expr) ATTRIBUTE(noreturn);

#ifdef __LINE__
#define ASSERT_LINE __LINE__
#else
#define ASSERT_LINE -1
#endif

#ifdef __FILE__
#define ASSERT_FILE __FILE__
#else
#define ASSERT_FILE ""
#endif

#define ASSERT_FAIL(expr) __crypto_tune_assert_fail(ASSERT_FILE, ASSERT_LINE, #expr)

#define ASSERT(expr) do { \
	if (UNLIKELY(!(expr))) { \
		ASSERT_FAIL(expr); \
	} \
} while (0)

double speed_measure(speed_function_t fun, struct speed_params *);

double tuneup_measure(speed_function_t fun, const struct param_t *param,
		struct speed_params *s);

void one(size_t *threshold, struct param_t *param);

/* low 32-bits in p[0], high 32-bits in p[1] */
void speed_cyclecounter (unsigned p[2]);

void mftb_function (unsigned p[2]);

double speed_cyclecounter_diff (const unsigned [2], const unsigned [2]);
int gettimeofday_microseconds_p (void);
int getrusage_microseconds_p (void);
int cycles_works_p (void);
long clk_tck (void);
double freq_measure (const char *, double (*)(void));

int double_cmp_ptr (const double *, const double *);
typedef int (*qsort_function_t) (const void *, const void *);

void noop(void);
void noop_1(int);

/* "get" is called repeatedly until it ticks over, just in case on a fast
   processor it takes less than a microsecond, though this is probably
   unlikely if it's a system call.

   speed_cyclecounter is called on the same side of the "get" for the start
   and end measurements.  It doesn't matter how long it takes from the "get"
   sample to the cycles sample, since that period will cancel out in the
   difference calculation (assuming it's the same each time).

   Letting the test run for more than a process time slice is probably only
   going to reduce accuracy, especially for getrusage when the cycle counter
   is real time, or for gettimeofday if the cycle counter is in fact process
   time.  Use CLK_TCK/2 as a reasonable stop.

   It'd be desirable to be quite accurate here.  The default speed_precision
   for a cycle counter is 10000 cycles, so to mix that with getrusage or
   gettimeofday the frequency should be at least that accurate.  But running
   measurements for 10000 microseconds (or more) is too long.  Be satisfied
   with just a half clock tick (5000 microseconds usually).  */

#define FREQ_MEASURE_ONE(name, type, get, getc, sec, usec) do { \
	type      st1, st, et1, et; \
	unsigned  sc[2], ec[2]; \
	long      dt, half_tick; \
	double    dc, cyc; \
	\
	half_tick = (1000000L / clk_tck()) / 2; \
	\
	get (st1); \
	do { \
		get (st); \
	} while (usec(st) == usec(st1) && sec(st) == sec(st1)); \
	\
	getc (sc); \
	\
	for (;;) { \
		get (et1); \
		do { \
			get (et); \
		} while (usec(et) == usec(et1) && sec(et) == sec(et1)); \
		\
		getc (ec); \
		\
		dc = speed_cyclecounter_diff (ec, sc); \
		\
		/* allow secs to cancel before multiplying */ \
		dt = sec(et) - sec(st); \
		dt = dt * 1000000L + (usec(et) - usec(st)); \
		\
		if (dt >= half_tick) { \
			break; \
		} \
	} \
	cyc = dt * 1e-6 / dc; \
	return dt * 1e-6 / dc; \
} while (0)

#endif /* TUNE_SPEED_H_ */
