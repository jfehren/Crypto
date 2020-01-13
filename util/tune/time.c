/*
 * time.c
 *
 *  Created on: Jan 6, 2020, 12:53:53 PM
 *      Author: Joshua Fehrenbach
 *
 * Everything here is essentially copied directly from GMP 6.1.2's
 * tune/time.c and tune/freq.c, with some adaptations made to fit
 * in here. The copyright and license on the relevant GMP source
 * files is as follows:
 *
 * Copyright 1999-2004, 2010-2012 Free Software Foundation, Inc.
 *
 * The GNU MP Library is free software; you can redistribute it and/or modify
 * it under the terms of either:
 *
 *   * the GNU Lesser General Public License as published by the Free
 *     Software Foundation; either version 3 of the License, or (at your
 *     option) any later version.
 *
 * or
 *
 *   * the GNU General Public License as published by the Free Software
 *     Foundation; either version 2 of the License, or (at your option) any
 *     later version.
 *
 * or both in parallel, as here.
 *
 * The GNU MP Library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received copies of the GNU General Public License and the
 * GNU Lesser General Public License along with the GNU MP Library.  If not,
 * see https://www.gnu.org/licenses/.
 */

#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <sys/types.h>

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#if HAVE_SYS_SYSSGI_H
#include <sys/syssgi.h>
#endif

#if HAVE_SYS_SYSTEMCFG_H
#include <sys/systemcfg.h>
#endif

#if HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif

#if HAVE_INVENT_H
#include <invent.h>
#endif

#if HAVE_SYS_ATTRIBUTES_H
#include <sys/attributes.h>
#endif

#if HAVE_SYS_IOGRAPH_H
#include <sys/iograph.h>
#endif

#if HAVE_SYS_PARAM_H
#include <sys/params.h>
#endif

#if HAVE_SYS_PSTAT_H
#include <sys/pstat.h>
#endif

#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#if HAVE_SYS_PROCESSOR_H
#include <sys/processor.h>
#endif

/* On AIX 5.1 with gcc 2.9-aix51-020209 in -maix64 mode, <sys/sysinfo.h>
   gets an error about "fill" in "struct cpuinfo" having a negative size,
   apparently due to __64BIT_KERNEL not being defined because _KERNEL is not
   defined.  Avoid this file if we don't actually need it, which we don't on
   AIX since there's no getsysinfo there.  */
#if HAVE_SYS_SYSINFO_H && HAVE_GETSYSINFO
#include <sys/sysinfo.h>
#endif

#if HAVE_MACHINE_HAL_SYSINFO_H
#include <machine/hal_sysinfo.h>
#endif

/* Remove definitions from NetBSD <sys/param.h>, to avoid conflicts */
#ifdef MIN
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif


#include "crypto.h"
#include "speed.h"

void ATTRIBUTE(noreturn)
__crypto_tune_assert_fail(const char *fn, int ln, const char *expr) {
	if (fn != NULL && fn[0] != '\0' && ln != -1) {
		fprintf(stderr, "%s:%d: Crypto tuning assertion failed: %s\n", fn, ln, expr);
	} else if (fn != NULL && fn[0] != '\0') {
		fprintf(stderr, "%s: Crypto tuning assertion failed: %s\n", fn, expr);
	} else {
		fprintf(stderr, "Crypto tuning assertion failed: %s\n", expr);
	}
	abort();
}

char    speed_time_string[256];
int     speed_precision = 0;
double  speed_unittime;
double  speed_cycletime = 0.0;


/* don't rely on "unsigned" to "double" conversion, it's broken in SunOS 4
   native cc */
#define M_2POWU   (((double) INT_MAX + 1.0) * 2.0)

#define M_2POW32  4294967296.0
#define M_2POW64  (M_2POW32 * M_2POW32)

/* Conditionals for the time functions available are done with normal C
   code, which is a lot easier than wildly nested preprocessor directives.

   The choice of what to use is partly made at run-time, according to
   whether the cycle counter works and the measured accuracy of getrusage
   and gettimeofday.

   A routine that's not available won't be getting called, but is an abort()
   to be sure it isn't called mistakenly.

   It can be assumed that if a function exists then its data type will, but
   if the function doesn't then the data type might or might not exist, so
   the type can't be used unconditionally.  The "struct_rusage" etc macros
   provide dummies when the respective function doesn't exist. */

#ifndef HAVE_SPEED_CYCLECOUNTER
#define HAVE_SPEED_CYCLECOUNTER __IS_x86__
#endif

#if HAVE_SPEED_CYCLECOUNTER
static const int have_cycles = HAVE_SPEED_CYCLECOUNTER;
#else
static const int have_cycles = 0;
#define speed_cyclecounter(p) ASSERT_FAIL(speed_cyclecounter not available)
#endif

/* "stck" returns ticks since 1 Jan 1900 00:00 GMT, where each tick is 2^-12
   microseconds.  Same #ifdefs here as in longlong.h.  */
#if defined (__GNUC__) && ! defined (NO_ASM)                            \
  && (defined (__i370__) || defined (__s390__) || defined (__mvs__))
static const int  have_stck = 1;
static const int  use_stck = 1;  /* always use when available */
typedef uint64_t  stck_t; /* gcc for s390 is quite new, always has uint64_t */
#define STCK(timestamp) do { \
	asm ("stck %0" : "=Q" (timestamp)); \
} while (0)
#else
static const int  have_stck = 0;
static const int  use_stck = 0;
typedef unsigned long  stck_t;   /* dummy */
#define STCK(timestamp)  ASSERT_FAIL (stck instruction not available)
#endif
#define STCK_PERIOD      (1.0 / 4096e6)   /* 2^-12 microseconds */

/* mftb
   Enhancement: On 64-bit chips mftb gives a 64-bit value, no need for mftbu
   and a loop (see powerpc64.asm).  */
#if HAVE_HOST_CPU_FAMILY_powerpc
static const int  have_mftb = 1;
#if defined (__GNUC__) && ! defined (NO_ASM)
#define MFTB(a) do { \
	unsigned  __h1, __l, __h2; \
	do { \
		asm volatile ( \
			"mftbu %0\n" \
			"mftb  %1\n" \
			"mftbu %2" \
			: "=r" (__h1), "=r" (__l), "=r" (__h2)); \
	} while (__h1 != __h2); \
	a[0] = __l; \
	a[1] = __h1; \
} while (0)
#else
#define MFTB(a)   mftb_function (a)
#endif
#else /* ! powerpc */
static const int  have_mftb = 0;
#define MFTB(a) do { \
	a[0] = 0; \
	a[1] = 0; \
	ASSERT_FAIL (mftb not available); \
} while (0)
#endif

/* Unicos 10.X has syssgi(), but not mmap(). */
#if HAVE_SYSSGI && HAVE_MMAP
static const int  have_sgi = 1;
#else
static const int  have_sgi = 0;
#endif

#if HAVE_READ_REAL_TIME
static const int have_rrt = 1;
#else
static const int have_rrt = 0;
#define read_real_time(t,s)     ASSERT_FAIL (read_real_time not available)
#define time_base_to_time(t,s)  ASSERT_FAIL (time_base_to_time not available)
#define RTC_POWER     1
#define RTC_POWER_PC  2
#define timebasestruct_t   struct timebasestruct_dummy
struct timebasestruct_dummy {
  int             flag;
  unsigned int    tb_high;
  unsigned int    tb_low;
};
#endif


#if HAVE_CLOCK_GETTIME
static const int have_cgt = 1;
#define struct_timespec  struct timespec
#else
static const int have_cgt = 0;
#define struct_timespec       struct timespec_dummy
#define clock_gettime(id,ts)  (ASSERT_FAIL (clock_gettime not available), -1)
#define clock_getres(id,ts)   (ASSERT_FAIL (clock_getres not available), -1)
#endif

#if HAVE_GETRUSAGE
static const int have_grus = 1;
#define struct_rusage   struct rusage
#else
static const int have_grus = 0;
#define getrusage(n,ru)  ASSERT_FAIL (getrusage not available)
#define struct_rusage    struct rusage_dummy
#endif

#if HAVE_GETTIMEOFDAY
static const int have_gtod = 1;
#define struct_timeval   struct timeval
#else
static const int have_gtod = 0;
#define gettimeofday(tv,tz)  ASSERT_FAIL (gettimeofday not available)
#define struct_timeval   struct timeval_dummy
#endif

#if HAVE_TIMES
static const int have_times = 1;
#define struct_tms   struct tms
#else
static const int have_times = 0;
#define times(tms)   ASSERT_FAIL (times not available)
#define struct_tms   struct tms_dummy
#endif


struct tms_dummy {
  long  tms_utime;
};
struct timeval_dummy {
  long  tv_sec;
  long  tv_usec;
};
struct rusage_dummy {
  struct_timeval ru_utime;
};
struct timespec_dummy {
  long  tv_sec;
  long  tv_nsec;
};

static int  use_cycles;
static int  use_mftb;
static int  use_sgi;
static int  use_rrt;
static int  use_cgt;
static int  use_gtod;
static int  use_grus;
static int  use_times;
static int  use_tick_boundary;

static unsigned         start_cycles[2];
static stck_t           start_stck;
static unsigned         start_mftb[2];
static unsigned         start_sgi;
static timebasestruct_t start_rrt;
static struct_timespec  start_cgt;
static struct_rusage    start_grus;
static struct_timeval   start_gtod;
static struct_tms       start_times;

static double  cycles_limit = 1e100;
static double  mftb_unittime;
static double  sgi_unittime;
static double  cgt_unittime;
static double  grus_unittime;
static double  gtod_unittime;
static double  times_unittime;

/* for RTC_POWER format, ie. seconds and nanoseconds */
#define TIMEBASESTRUCT_SECS(t)  ((t)->tb_high + (t)->tb_low * 1e-9)

/* Return a string representing a time in seconds, nicely formatted.
   Eg. "10.25ms".  */
char *
unittime_string (double t) {
	static char  buf[128];
	const char  *unit;
	int         prec;

	/* choose units and scale */
	if (t < 1e-6) {
		t *= 1e9;
		unit = "ns";
	} else if (t < 1e-3) {
		t *= 1e6;
		unit = "us";
	} else if (t < 1.0) {
		t *= 1e3;
		unit = "ms";
	} else {
		unit = "s";
	}

	/* want 4 significant figures */
	if (t < 1.0) {
		prec = 4;
	} else if (t < 10.0) {
		prec = 3;
	} else if (t < 100.0) {
		prec = 2;
	} else {
		prec = 1;
	}

	sprintf (buf, "%.*f%s", prec, t, unit);
	return buf;
}


static jmp_buf  cycles_works_buf;

#ifndef RETSIGTYPE
#define RETSIGTYPE void
#endif

static RETSIGTYPE
cycles_works_handler (int sig) {
	longjmp (cycles_works_buf, 1);
}

int
cycles_works_p (void) {
	static int  result = -1;

	if (result != -1) {
		goto done;
	}

	/* FIXME: On linux, the cycle counter is not saved and restored over
	 * context switches, making it almost useless for precise cputime
	 * measurements. When available, it's better to use clock_gettime,
	 * which seems to have reasonable accuracy (tested on x86_32,
	 * linux-2.6.26, glibc-2.7). However, there are also some linux
	 * systems where clock_gettime is broken in one way or the other,
	 * like CLOCK_PROCESS_CPUTIME_ID not implemented (easy case) or
	 * kind-of implemented but broken (needs code to detect that), and
	 * on those systems a wall-clock cycle counter is the least bad
	 * fallback.
	 *
	 * So we need some code to disable the cycle counter on some but not
	 * all linux systems. */
#ifdef SIGILL
	{
		RETSIGTYPE (*old_handler) (int);
		unsigned  cycles[2];

		old_handler = signal (SIGILL, cycles_works_handler);
		if (old_handler == SIG_ERR) {
			goto yes;
		}
		if (setjmp (cycles_works_buf)) {
			result = 0;
			goto done;
		}
		speed_cyclecounter (cycles);
		signal (SIGILL, old_handler);
	}
#else
	goto yes;
#endif

	yes:
	result = 1;

	done:
	return result;
}

/* The number of clock ticks per second, but looking at sysconf rather than
   just CLK_TCK, where possible.  */
long
clk_tck (void) {
	static long  result = -1L;
	if (result != -1L) {
		return result;
	}

#if HAVE_SYSCONF
	result = sysconf (_SC_CLK_TCK);
	if (result != -1L) {
		return result;
	}

	fprintf (stderr,
			"sysconf(_SC_CLK_TCK) not working, using CLK_TCK instead\n");
#endif

#ifdef CLK_TCK
	result = CLK_TCK;
	return result;
#else
	fprintf (stderr, "CLK_TCK not defined, cannot continue\n");
	abort ();
#endif
}


/* If two times can be observed less than half a clock tick apart, then
   assume "get" is microsecond accurate.

   Two times only 1 microsecond apart are not believed, since some kernels
   take it upon themselves to ensure gettimeofday doesn't return the same
   value twice, for the benefit of applications using it for a timestamp.
   This is obviously very stupid given the speed of CPUs these days.

   Making "reps" many calls to noop_1() is designed to waste some CPU, with
   a view to getting measurements 2 microseconds (or more) apart.  "reps" is
   increased progressively until such a period is seen.

   The outer loop "attempts" are just to allow for any random nonsense or
   system load upsetting the measurements (ie. making two successive calls
   to "get" come out as a longer interval than normal).

   Bugs:

   The assumption that any interval less than a half tick implies
   microsecond resolution is obviously fairly rash, the true resolution
   could be anything between a microsecond and that half tick.  Perhaps
   something special would have to be done on a system where this is the
   case, since there's no obvious reliable way to detect it
   automatically.  */

#define MICROSECONDS_P(name, type, get, sec, usec) { \
	static int  result = -1; \
	type      st, et; \
	long      dt, half_tick; \
	unsigned  attempt, reps, i, j; \
	\
	if (result != -1) { \
		return result; \
	} \
	result = 0; \
	half_tick = (1000000L / clk_tck ()) / 2; \
	\
	for (attempt = 0; attempt < 5; attempt++) { \
		reps = 0; \
		for (;;) { \
			get (st); \
			for (i = 0; i < reps; i++) { \
				for (j = 0; j < 100; j++) { \
					noop_1(0); \
				} \
			} \
			get (et); \
			\
			dt = (sec(et)-sec(st))*1000000L + usec(et)-usec(st); \
			\
			if (dt >= 2) { \
				break; \
			} \
			reps = (reps == 0 ? 1 : 2*reps); \
			if (reps == 0) { \
				break;  /* uint overflow, not normal */ \
			} \
		} \
		\
		if (dt < half_tick) { \
			result = 1; \
			break; \
		} \
	} \
	\
	return result; \
}


int
gettimeofday_microseconds_p (void) {
#define call_gettimeofday(t)   gettimeofday (&(t), NULL)
#define timeval_tv_sec(t)      ((t).tv_sec)
#define timeval_tv_usec(t)     ((t).tv_usec)
	MICROSECONDS_P ("gettimeofday", struct_timeval,
			call_gettimeofday, timeval_tv_sec, timeval_tv_usec);
}

int
getrusage_microseconds_p (void) {
#define call_getrusage(t)   getrusage (0, &(t))
#define rusage_tv_sec(t)    ((t).ru_utime.tv_sec)
#define rusage_tv_usec(t)   ((t).ru_utime.tv_usec)
	MICROSECONDS_P ("getrusage", struct_rusage,
			call_getrusage, rusage_tv_sec, rusage_tv_usec);
}


/* Test whether getrusage goes backwards, return non-zero if it does
   (suggesting it's flawed).

   On a macintosh m68040-unknown-netbsd1.4.1 getrusage looks like it's
   microsecond accurate, but has been seen remaining unchanged after many
   microseconds have elapsed.  It also regularly goes backwards by 1000 to
   5000 usecs, this has been seen after between 500 and 4000 attempts taking
   perhaps 0.03 seconds.  We consider this too broken for good measuring.
   We used to have configure pretend getrusage didn't exist on this system,
   but a runtime test should be more reliable, since we imagine the problem
   is not confined to just this exact system tuple.  */

int
getrusage_backwards_p (void) {
	static int result = -1;
	struct_rusage  start, prev, next;
	long  d;
	int   i;

	if (result != -1) {
		return result;
	}

	if (!have_grus) {
		result = 0;
		return result;
	}

	getrusage (0, &start);
	memcpy (&next, &start, sizeof (next));

	result = 0;
	i = 0;
	for (;;) {
		memcpy (&prev, &next, sizeof (prev));
		getrusage (0, &next);

		if (next.ru_utime.tv_sec < prev.ru_utime.tv_sec
				|| (next.ru_utime.tv_sec == prev.ru_utime.tv_sec
						&& next.ru_utime.tv_usec < prev.ru_utime.tv_usec)) {
			result = 1;
			break;
		}

		/* minimum 1000 attempts, then stop after either 0.1 seconds or 50000
		 * attempts, whichever comes first */
		d = 1000000 * (next.ru_utime.tv_sec - start.ru_utime.tv_sec)
			+ (next.ru_utime.tv_usec - start.ru_utime.tv_usec);
		i++;
		if (i > 50000 || (i > 1000 && d > 100000)) {
			break;
		}
	}

	return result;
}


/* CLOCK_PROCESS_CPUTIME_ID looks like it's going to be in a future version
   of glibc (some time post 2.2).

   CLOCK_VIRTUAL is process time, available in BSD systems (though sometimes
   defined, but returning -1 for an error).  */

#ifdef CLOCK_PROCESS_CPUTIME_ID
# define CGT_ID        CLOCK_PROCESS_CPUTIME_ID
#else
# ifdef CLOCK_VIRTUAL
#  define CGT_ID       CLOCK_VIRTUAL
# endif
#endif
#ifdef CGT_ID
const int  have_cgt_id = 1;
#else
const int  have_cgt_id = 0;
# define CGT_ID       (ASSERT_FAIL (CGT_ID not determined), -1)
#endif

#define CGT_DELAY_COUNT 1000

int
cgt_works_p (void) {
	static int  result = -1;
	struct_timespec  unit;

	if (result != -1) {
		return result;
	}

	if (!have_cgt || !have_cgt_id) {
		result = 0;
		return result;
	}

	/* trial run to see if it works */
	if (clock_gettime (CGT_ID, &unit) != 0) {
		result = 0;
		return result;
	}

	/* get the resolution */
	if (clock_getres (CGT_ID, &unit) != 0) {
		result = 0;
		return result;
	}

	cgt_unittime = unit.tv_sec + unit.tv_nsec * 1e-9;

	if (cgt_unittime < 10e-9) {
		/* Do we believe this? */
		struct timespec start, end;
		static volatile int counter;
		double duration;
		if (clock_gettime (CGT_ID, &start)) {
			result = 0;
			return result;
		}
		/* Loop of at least 1000 memory accesses, ought to take at
		 * least 100 ns*/
		for (counter = 0; counter < CGT_DELAY_COUNT; counter++)
			;
		if (clock_gettime (CGT_ID, &end)) {
			result = 0;
			return result;
		}
		duration = (end.tv_sec + end.tv_nsec * 1e-9
				- start.tv_sec - start.tv_nsec * 1e-9);
		if (duration < 100e-9) {
			result = 0;
			return result;
		}
	}
	result = 1;
	return result;
}

static double
freq_measure_mftb_one (void) {
#define call_gettimeofday(t)   gettimeofday (&(t), NULL)
#define timeval_tv_sec(t)      ((t).tv_sec)
#define timeval_tv_usec(t)     ((t).tv_usec)
	FREQ_MEASURE_ONE ("mftb", struct_timeval,
			call_gettimeofday, MFTB,
			timeval_tv_sec, timeval_tv_usec);
}

static jmp_buf  mftb_works_buf;

static RETSIGTYPE
mftb_works_handler (int sig) {
	longjmp (mftb_works_buf, 1);
}

int
mftb_works_p (void) {
	unsigned   a[2];
	RETSIGTYPE (*old_handler) (int);
	double     cycletime;

	/* suppress a warning about a[] unused */
	a[0] = 0;

	if (! have_mftb) {
		return 0;
	}

#ifdef SIGILL
	old_handler = signal (SIGILL, mftb_works_handler);
	if (old_handler == SIG_ERR) {
		return 1;
	}
	if (setjmp (mftb_works_buf)) {
		return 0;
	}
	MFTB (a);
	signal (SIGILL, old_handler);
#endif

#if ! HAVE_GETTIMEOFDAY
	return 0;
#endif

	/* The time base is normally 1/4 of the bus speed on 6xx and 7xx chips, on
     other chips it can be driven from an external clock. */
	cycletime = freq_measure ("mftb", freq_measure_mftb_one);
	if (cycletime == -1.0) {
		return 0;
	}

	mftb_unittime = cycletime;
	return 1;
}


volatile unsigned  *sgi_addr;

int
sgi_works_p (void) {
#if HAVE_SYSSGI && HAVE_MMAP
	static int  result = -1;
	size_t          pagesize, offset;
	__psunsigned_t  phys, physpage;
	void            *virtpage;
	unsigned        period_picoseconds;
	int             size, fd;

	if (result != -1) {
		return result;
	}

	phys = syssgi (SGI_QUERY_CYCLECNTR, &period_picoseconds);
	if (phys == (__psunsigned_t) -1) {
		/* ENODEV is the error when a counter is not available */
		result = 0;
		return result;
	}
	sgi_unittime = period_picoseconds * 1e-12;

	/* IRIX 5 doesn't have SGI_CYCLECNTR_SIZE, assume 32 bits in that case.
     Challenge/ONYX hardware has a 64 bit byte counter, but there seems no
     obvious way to identify that without SGI_CYCLECNTR_SIZE.  */
#ifdef SGI_CYCLECNTR_SIZE
	size = syssgi (SGI_CYCLECNTR_SIZE);
	if (size == -1) {
		size = 32;
	}
#else
	size = 32;
#endif

	if (size < 32) {
		result = 0;
		return result;
	}

	pagesize = getpagesize();
	offset = (size_t) phys & (pagesize-1);
	physpage = phys - offset;

	/* shouldn't cross over a page boundary */
	ASSERT_ALWAYS (offset + size/8 <= pagesize);

	fd = open("/dev/mmem", O_RDONLY);
	if (fd == -1) {
		result = 0;
		return result;
	}

	virtpage = mmap (0, pagesize, PROT_READ, MAP_PRIVATE, fd, (off_t) physpage);
	if (virtpage == (void *) -1) {
		result = 0;
		return result;
	}

	/* address of least significant 4 bytes, knowing mips is big endian */
	sgi_addr = (unsigned *) ((char *) virtpage + offset
			+ size/8 - sizeof(unsigned));
	result = 1;
	return result;
#else /* ! (HAVE_SYSSGI && HAVE_MMAP) */
	return 0;
#endif
}


#define DEFAULT(var,n) do { \
	if (!(var)) { \
		(var) = (n); \
	} \
} while (0)


static void
speed_timestamp_init(void) {

}

void
speed_time_init (void) {
	double supplement_unittime = 0.0;

	static int  speed_time_initialized = 0;
	if (speed_time_initialized) {
		return;
	}
	speed_time_initialized = 1;

	speed_cycletime_init ();

	if (have_cycles && cycles_works_p ()) {
		use_cycles = 1;
		DEFAULT (speed_cycletime, 1.0);
		speed_unittime = speed_cycletime;
		DEFAULT (speed_precision, 10000);
		strcpy (speed_time_string, "CPU cycle counter");

		/* only used if a supplementary method is chosen below */
		cycles_limit = (have_cycles == 1 ? M_2POW32 : M_2POW64) / 2.0
				* speed_cycletime;

		if (have_grus && getrusage_microseconds_p() && ! getrusage_backwards_p()) {
			/* this is a good combination */
			use_grus = 1;
			supplement_unittime = grus_unittime = 1.0e-6;
			strcpy (speed_time_string,
					"CPU cycle counter, supplemented by microsecond getrusage()");
		} else if (have_cycles == 1) {
			/* When speed_cyclecounter has a limited range, look for something
			 * to supplement it. */
			if (have_gtod && gettimeofday_microseconds_p()) {
				use_gtod = 1;
				supplement_unittime = gtod_unittime = 1.0e-6;
				strcpy (speed_time_string,
						"CPU cycle counter, supplemented by microsecond gettimeofday()");
			} else if (have_grus) {
				use_grus = 1;
				supplement_unittime = grus_unittime = 1.0 / (double) clk_tck ();
				sprintf (speed_time_string,
						"CPU cycle counter, supplemented by %s clock tick getrusage()",
						unittime_string (supplement_unittime));
			} else if (have_times) {
				use_times = 1;
				supplement_unittime = times_unittime = 1.0 / (double) clk_tck ();
				sprintf (speed_time_string,
						"CPU cycle counter, supplemented by %s clock tick times()",
						unittime_string (supplement_unittime));
			} else if (have_gtod) {
				use_gtod = 1;
				supplement_unittime = gtod_unittime = 1.0 / (double) clk_tck ();
				sprintf (speed_time_string,
						"CPU cycle counter, supplemented by %s clock tick gettimeofday()",
						unittime_string (supplement_unittime));
			} else {
				fprintf (stderr, "WARNING: cycle counter is 32 bits and there's no other functions.\n");
				fprintf (stderr, "    Wraparounds may produce bad results on long measurements.\n");
			}
		}

		if (use_grus || use_times || use_gtod) {
			/* must know cycle period to compare cycles to other measuring
			 * (via cycles_limit) */
			speed_cycletime_need_seconds ();

			if (speed_precision * supplement_unittime > cycles_limit) {
				fprintf (stderr, "WARNING: requested precision can't always be achieved due to limited range\n");
				fprintf (stderr, "    cycle counter and limited precision supplemental method\n");
				fprintf (stderr, "    (%s)\n", speed_time_string);
			}
		}
	} else if (have_stck) {
		strcpy (speed_time_string, "STCK timestamp");
		/* stck is in units of 2^-12 microseconds, which is very likely higher
		 * resolution than a cpu cycle */
		if (speed_cycletime == 0.0) {
			speed_cycletime_fail ("Need to know CPU frequency for effective stck unit");
		}
		speed_unittime = MAX (speed_cycletime, STCK_PERIOD);
		DEFAULT (speed_precision, 10000);
	} else if (have_mftb && mftb_works_p ()) {
		use_mftb = 1;
		DEFAULT (speed_precision, 10000);
		speed_unittime = mftb_unittime;
		sprintf (speed_time_string, "mftb counter (%s)",
				unittime_string (speed_unittime));
	} else if (have_sgi && sgi_works_p ()) {
		use_sgi = 1;
		DEFAULT (speed_precision, 10000);
		speed_unittime = sgi_unittime;
		sprintf (speed_time_string,
				"syssgi() mmap counter (%s), supplemented by millisecond getrusage()",
				unittime_string (speed_unittime));
		/* supplemented with getrusage, which we assume to have 1ms resolution */
		use_grus = 1;
		supplement_unittime = 1e-3;
	} else if (have_rrt) {
		timebasestruct_t  t;
		use_rrt = 1;
		DEFAULT (speed_precision, 10000);
		read_real_time (&t, sizeof(t));
		switch (t.flag) {
			case RTC_POWER:
				/* FIXME: What's the actual RTC resolution? */
				speed_unittime = 1e-7;
				strcpy (speed_time_string, "read_real_time() power nanoseconds");
				break;
			case RTC_POWER_PC:
				t.tb_high = 1;
				t.tb_low = 0;
				time_base_to_time (&t, sizeof(t));
				speed_unittime = TIMEBASESTRUCT_SECS(&t) / M_2POW32;
				sprintf (speed_time_string, "%s read_real_time() powerpc ticks",
						unittime_string (speed_unittime));
				break;
			default:
				fprintf (stderr, "ERROR: Unrecognised timebasestruct_t flag=%d\n",
						t.flag);
				abort ();
		}
	} else if (have_cgt && cgt_works_p() && cgt_unittime < 1.5e-6) {
		/* use clock_gettime if microsecond or better resolution */
	choose_cgt:
		use_cgt = 1;
		speed_unittime = cgt_unittime;
		DEFAULT (speed_precision, (cgt_unittime <= 0.1e-6 ? 10000 : 1000));
		strcpy (speed_time_string, "microsecond accurate clock_gettime()");
	} else if (have_times && clk_tck() > 1000000) {
		/* Cray vector systems have times() which is clock cycle resolution
		 * (eg. 450 MHz).  */
		DEFAULT (speed_precision, 10000);
		goto choose_times;
	} else if (have_grus && getrusage_microseconds_p() && ! getrusage_backwards_p()) {
		use_grus = 1;
		speed_unittime = grus_unittime = 1.0e-6;
		DEFAULT (speed_precision, 1000);
		strcpy (speed_time_string, "microsecond accurate getrusage()");
	} else if (have_gtod && gettimeofday_microseconds_p()) {
		use_gtod = 1;
		speed_unittime = gtod_unittime = 1.0e-6;
		DEFAULT (speed_precision, 1000);
		strcpy (speed_time_string, "microsecond accurate gettimeofday()");
	} else if (have_cgt && cgt_works_p() && cgt_unittime < 1.5/clk_tck()) {
		/* use clock_gettime if 1 tick or better resolution */
		goto choose_cgt;
	} else if (have_times) {
		use_tick_boundary = 1;
		DEFAULT (speed_precision, 200);
	choose_times:
		use_times = 1;
		speed_unittime = times_unittime = 1.0 / (double) clk_tck ();
		sprintf (speed_time_string, "%s clock tick times()",
				unittime_string (speed_unittime));
	} else if (have_grus) {
		use_grus = 1;
		use_tick_boundary = 1;
		speed_unittime = grus_unittime = 1.0 / (double) clk_tck ();
		DEFAULT (speed_precision, 200);
		sprintf (speed_time_string, "%s clock tick getrusage()\n",
				unittime_string (speed_unittime));
	} else if (have_gtod) {
		use_gtod = 1;
		use_tick_boundary = 1;
		speed_unittime = gtod_unittime = 1.0 / (double) clk_tck ();
		DEFAULT (speed_precision, 200);
		sprintf (speed_time_string, "%s clock tick gettimeofday()",
				unittime_string (speed_unittime));
	} else {
		fprintf (stderr, "No time measuring method available\n");
		fprintf (stderr, "None of: speed_cyclecounter(), STCK(), getrusage(), gettimeofday(), times()\n");
		abort ();
	}
}


/* Burn up CPU until a clock tick boundary, for greater accuracy.  Set the
   corresponding "start_foo" appropriately too. */

void
grus_tick_boundary (void) {
	struct_rusage  prev;
	getrusage (0, &prev);
	do {
		getrusage (0, &start_grus);
	} while (start_grus.ru_utime.tv_usec == prev.ru_utime.tv_usec);
}

void
gtod_tick_boundary (void) {
	struct_timeval  prev;
	gettimeofday (&prev, NULL);
	do {
		gettimeofday (&start_gtod, NULL);
	} while (start_gtod.tv_usec == prev.tv_usec);
}

void
times_tick_boundary (void) {
	struct_tms  prev;
	times (&prev);
	do {
		times (&start_times);
	} while (start_times.tms_utime == prev.tms_utime);
}



/* "have_" values are tested to let unused code go dead.  */

void
speed_starttime (void) {
	speed_time_init ();

	if (have_grus && use_grus) {
		if (use_tick_boundary) {
			grus_tick_boundary ();
		} else {
			getrusage (0, &start_grus);
		}
	}

	if (have_gtod && use_gtod) {
		if (use_tick_boundary) {
			gtod_tick_boundary ();
		} else {
			gettimeofday (&start_gtod, NULL);
		}
	}

	if (have_times && use_times) {
		if (use_tick_boundary) {
			times_tick_boundary ();
		} else {
			times (&start_times);
		}
	}

	if (have_cgt && use_cgt) {
		clock_gettime (CGT_ID, &start_cgt);
	}

	if (have_rrt && use_rrt) {
		read_real_time (&start_rrt, sizeof(start_rrt));
	}

	if (have_sgi && use_sgi) {
		start_sgi = *sgi_addr;
	}

	if (have_mftb && use_mftb) {
		MFTB (start_mftb);
	}

	if (have_stck && use_stck) {
		STCK (start_stck);
	}

	/* Cycles sampled last for maximum accuracy. */
	if (have_cycles && use_cycles) {
		speed_cyclecounter (start_cycles);
	}
}

/* Calculate the difference between two cycle counter samples, as a "double"
   counter of cycles.

   The start and end values are allowed to cancel in integers in case the
   counter values are bigger than the 53 bits that normally fit in a double.

   This works even if speed_cyclecounter() puts a value bigger than 32-bits
   in the low word (the high word always gets a 2**32 multiplier though). */

double
speed_cyclecounter_diff (const unsigned end[2], const unsigned start[2]) {
	unsigned  d;
	double    t;

	if (have_cycles == 1) {
		t = (end[0] - start[0]);
	} else {
		d = end[0] - start[0];
		t = d - (d > end[0] ? M_2POWU : 0.0);
		t += (end[1] - start[1]) * M_2POW32;
	}
	return t;
}

double
speed_mftb_diff (const unsigned end[2], const unsigned start[2]) {
	unsigned  d;
	double    t;

	d = end[0] - start[0];
	t = (double) d - (d > end[0] ? M_2POW32 : 0.0);
	t += (end[1] - start[1]) * M_2POW32;
	return t;
}


/* Calculate the difference between "start" and "end" using fields "sec" and
   "psec", where each "psec" is a "punit" of a second.

   The seconds parts are allowed to cancel before being combined with the
   psec parts, in case a simple "sec+psec*punit" exceeds the precision of a
   double.

   Total time is only calculated in a "double" since an integer count of
   psecs might overflow.  2^32 microseconds is only a bit over an hour, or
   2^32 nanoseconds only about 4 seconds.

   The casts to "long" are for the benefit of timebasestruct_t, where the
   fields are only "unsigned int", but we want a signed difference.  */

#define DIFF_SECS_ROUTINE(sec, psec, punit) { \
	long  sec_diff, psec_diff; \
	sec_diff = (long) end->sec - (long) start->sec; \
	psec_diff = (long) end->psec - (long) start->psec; \
	return (double) sec_diff + punit * (double) psec_diff; \
}

double
timeval_diff_secs (const struct_timeval *end, const struct_timeval *start) {
	DIFF_SECS_ROUTINE (tv_sec, tv_usec, 1e-6);
}

double
rusage_diff_secs (const struct_rusage *end, const struct_rusage *start) {
	DIFF_SECS_ROUTINE (ru_utime.tv_sec, ru_utime.tv_usec, 1e-6);
}

double
timespec_diff_secs (const struct_timespec *end, const struct_timespec *start) {
	DIFF_SECS_ROUTINE (tv_sec, tv_nsec, 1e-9);
}

/* This is for use after time_base_to_time, ie. for seconds and nanoseconds. */
double
timebasestruct_diff_secs (const timebasestruct_t *end,
		const timebasestruct_t *start) {
	DIFF_SECS_ROUTINE (tb_high, tb_low, 1e-9);
}


double
speed_endtime (void) {
#define END_USE(name,value) do { \
	result = value; \
	goto done; \
} while (0)

#define END_ENOUGH(name,value) do { \
	result = value; \
	goto done; \
} while (0)

#define END_EXCEED(name,value) do { \
	result = value; \
	goto done; \
} while (0)

	unsigned          end_cycles[2];
	stck_t            end_stck;
	unsigned          end_mftb[2];
	unsigned          end_sgi;
	timebasestruct_t  end_rrt;
	struct_timespec   end_cgt;
	struct_timeval    end_gtod;
	struct_rusage     end_grus;
	struct_tms        end_times;
	double            t_gtod, t_grus, t_times, t_cgt;
	double            t_rrt, t_sgi, t_mftb, t_stck, t_cycles;
	double            result;

	/* Cycles sampled first for maximum accuracy.
     "have_" values tested to let unused code go dead.  */

	if (have_cycles && use_cycles)  speed_cyclecounter (end_cycles);
	if (have_stck   && use_stck)    STCK (end_stck);
	if (have_mftb   && use_mftb)    MFTB (end_mftb);
	if (have_sgi    && use_sgi)     end_sgi = *sgi_addr;
	if (have_rrt    && use_rrt)     read_real_time (&end_rrt, sizeof(end_rrt));
	if (have_cgt    && use_cgt)     clock_gettime (CGT_ID, &end_cgt);
	if (have_gtod   && use_gtod)    gettimeofday (&end_gtod, NULL);
	if (have_grus   && use_grus)    getrusage (0, &end_grus);
	if (have_times  && use_times)   times (&end_times);

	result = -1.0;

	if (use_rrt) {
		time_base_to_time (&start_rrt, sizeof(start_rrt));
		time_base_to_time (&end_rrt, sizeof(end_rrt));
		t_rrt = timebasestruct_diff_secs (&end_rrt, &start_rrt);
		END_USE ("read_real_time()", t_rrt);
	}

	if (use_cgt) {
		t_cgt = timespec_diff_secs (&end_cgt, &start_cgt);
		END_USE ("clock_gettime()", t_cgt);
	}

	if (use_grus) {
		t_grus = rusage_diff_secs (&end_grus, &start_grus);

		/* Use getrusage() if the cycle counter limit would be exceeded, or if
		 * it provides enough accuracy already. */
		if (use_cycles) {
			if (t_grus >= speed_precision*grus_unittime) {
				END_ENOUGH ("getrusage()", t_grus);
			}
			if (t_grus >= cycles_limit) {
				END_EXCEED ("getrusage()", t_grus);
			}
		}
	}

	if (use_times) {
		t_times = (end_times.tms_utime - start_times.tms_utime) * times_unittime;

		/* Use times() if the cycle counter limit would be exceeded, or if
		 * it provides enough accuracy already. */
		if (use_cycles) {
			if (t_times >= speed_precision*times_unittime) {
				END_ENOUGH ("times()", t_times);
			}
			if (t_times >= cycles_limit) {
				END_EXCEED ("times()", t_times);
			}
		}
	}

	if (use_gtod) {
		t_gtod = timeval_diff_secs (&end_gtod, &start_gtod);

		/* Use gettimeofday() if it measured a value bigger than the cycle
		 * counter can handle.  */
		if (use_cycles) {
			if (t_gtod >= cycles_limit) {
				END_EXCEED ("gettimeofday()", t_gtod);
			}
		}
	}

	if (use_mftb) {
		t_mftb = speed_mftb_diff (end_mftb, start_mftb) * mftb_unittime;
		END_USE ("mftb", t_mftb);
	}

	if (use_stck) {
		t_stck = (end_stck - start_stck) * STCK_PERIOD;
		END_USE ("stck", t_stck);
	}

	if (use_sgi) {
		t_sgi = (end_sgi - start_sgi) * sgi_unittime;
		END_USE ("SGI hardware counter", t_sgi);
	}

	if (use_cycles) {
		t_cycles = speed_cyclecounter_diff (end_cycles, start_cycles)
			* speed_cycletime;
		END_USE ("cycle counter", t_cycles);
	}

	if (use_grus && getrusage_microseconds_p()) {
		END_USE ("getrusage()", t_grus);
	}

	if (use_gtod && gettimeofday_microseconds_p()) {
		END_USE ("gettimeofday()", t_gtod);
	}

	if (use_times)  END_USE ("times()",        t_times);
	if (use_grus)   END_USE ("getrusage()",    t_grus);
	if (use_gtod)   END_USE ("gettimeofday()", t_gtod);

	fprintf (stderr, "speed_endtime(): oops, no time method available\n");
	abort ();

done:
	if (result < 0.0) {
		result = 0.0;
	}
	return result;
}

double
speed_current_timestamp(void) {
	static unsigned zero[2] = { 0, 0 };
	unsigned          cycles[2];
	stck_t            stck;
	unsigned          mftb[2];
	unsigned          sgi;
	timebasestruct_t  rrt;
	struct_timespec   cgt;
	struct_timeval    gtod;
	struct_rusage     grus;
	struct_tms        _times;
	double t_gtod, t_grus, t_times, t_cgt;
	double t_rrt, t_sgi, t_mftb, t_stck, t_cycles;
	double result;

	if (have_times  && use_times)   times (&_times);
	if (have_grus   && use_grus)    getrusage (0, &grus);
	if (have_gtod   && use_gtod)    gettimeofday (&gtod, NULL);
	if (have_cgt    && use_cgt)     clock_gettime (CGT_ID, &cgt);
	if (have_rrt    && use_rrt)     read_real_time (&rrt, sizeof(rrt));
	if (have_sgi    && use_sgi)     sgi = *sgi_addr;
	if (have_mftb   && use_mftb)    MFTB (mftb);
	if (have_stck   && use_stck)    STCK (stck);
	if (have_cycles && use_cycles)  speed_cyclecounter (cycles);
}


double
speed_measure(speed_function_t fun, struct speed_params *s) {
#define TOLERANCE    1.01  /* 1% */
	const int max_zeros = 10;

	struct speed_params  s_dummy;
	int     i, j, e;
	double  t[30];
	double  t_unsorted[30];
	double  reps_d;
	int     zeros = 0;

	/* Use dummy parameters if caller doesn't provide any.  Only a few special
	 * "fun"s will cope with this, speed_noop() is one.  */
	if (s == NULL) {
		memset (&s_dummy, '\0', sizeof (s_dummy));
		s = &s_dummy;
	}

	s->reps = 1;
	s->time_divisor = 1.0;
	for (i = 0; i < numberof (t); i++) {
		for (;;) {
			t[i] = (*fun) (s);

			if (t[i] == 0.0) {
				zeros++;
				if (zeros > max_zeros) {
					fprintf (stderr,
							"Fatal error: too many (%d) failed measurements (0.0)\n",
							zeros);
					abort ();
				}
				if (s->reps < 10000) {
					s->reps *= 2;
				}
				continue;
			}

			if (t[i] == -1.0) {
				return -1.0;
			}
			if (t[i] >= speed_unittime * speed_precision) {
				break;
			}
			/* go to a value of reps to make t[i] >= precision */
			reps_d = ceil (1.1 * s->reps
					* speed_unittime * speed_precision
					/ MAX (t[i], speed_unittime));
			if (reps_d > 2e9 || reps_d < 1.0) {
				fprintf (stderr, "Fatal error: new reps bad: %.2f\n", reps_d);
				fprintf (stderr,
						"  (old reps %u, unittime %.4g, precision %d, t[i] %.4g)\n",
						s->reps, speed_unittime, speed_precision, t[i]);
				abort ();
			}
			s->reps = (unsigned) reps_d;
		}
		t[i] /= s->reps;
		t_unsorted[i] = t[i];

		if (speed_precision == 0) {
			return t[i];
		}

		/* require 3 values within TOLERANCE when >= 2 secs, 4 when below */
		if (t[0] >= 2.0) {
			e = 3;
		} else {
			e = 4;
		}

		/* Look for e many t[]'s within TOLERANCE of each other to consider a
		 * valid measurement.  Return smallest among them.  */
		if (i >= e) {
			qsort (t, i+1, sizeof(t[0]), (qsort_function_t) double_cmp_ptr);
			for (j = e-1; j < i; j++) {
				if (t[j] <= t[j-e+1] * TOLERANCE) {
					return t[j-e+1] / s->time_divisor;
				}
			}
		}
	}

	fprintf (stderr, "speed_measure() could not get %d results within %.1f%%\n",
			e, (TOLERANCE-1.0)*100.0);
	fprintf (stderr, "    unsorted         sorted\n");
	fprintf (stderr, "  %.12f    %.12f    is about 0.5%%\n",
			t_unsorted[0]*(TOLERANCE-1.0), t[0]*(TOLERANCE-1.0));
	for (i = 0; i < numberof (t); i++) {
		fprintf (stderr, "  %.09f       %.09f\n", t_unsorted[i], t[i]);
	}

	return -1.0;
}





/* Functions from GMP 6.1.2's tune/freq.c */

#define HELP(str) \
if (help) { \
	printf ("    - %s\n", str); \
	return 0; \
}

/* CRYPTO_CPU_FREQUENCY environment variable.  Should be in Hertz and can be
   floating point, for example "450e6". */
static int
freq_environment (int help) {
	char  *e;

	HELP ("environment variable CRYPTO_CPU_FREQUENCY (in Hertz)");

	e = getenv ("CRYPTO_CPU_FREQUENCY");
	if (e == NULL) {
		return 0;
	}

	speed_cycletime = 1.0 / atof (e);

	return 1;
}

/* getsysinfo is available on OSF, or 4.0 and up at least.
   The man page (on 4.0) suggests a 0 return indicates information not
   available, but that seems to be the normal return for GSI_CPU_INFO.  */
static int
freq_getsysinfo (int help) {
#if HAVE_GETSYSINFO
	struct cpu_info  c;
	int              start;

	HELP ("getsysinfo() GSI_CPU_INFO");

	start = 0;
	if (getsysinfo (GSI_CPU_INFO, (caddr_t) &c, sizeof (c),
			&start, NULL, NULL) != -1) {
		speed_cycletime = 1e-6 / (double) c.mhz;
		return 1;
	}
#endif
	return 0;
}

/* In HPUX 10 and up, pstat_getprocessor() psp_iticksperclktick is the
   number of CPU cycles (ie. the CR16 register) per CLK_TCK.  HPUX 9 doesn't
   have that field in pst_processor though, and has no apparent
   equivalent.  */
static int
freq_pstat_getprocessor (int help) {
#if HAVE_PSTAT_GETPROCESSOR && HAVE_PSP_ITICKSPERCLKTICK
	struct pst_processor  p;

	HELP ("pstat_getprocessor() psp_iticksperclktick");

	if (pstat_getprocessor (&p, sizeof(p), 1, 0) != -1) {
		long  c = clk_tck();
		speed_cycletime = 1.0 / (c * p.psp_iticksperclktick);
		return 1;
	}
#endif
	return 0;
}

/* i386 FreeBSD 2.2.8 sysctlbyname machdep.i586_freq is in Hertz.
   There's no obvious defines available to get this from plain sysctl.  */
static int
freq_sysctlbyname_i586_freq (int help) {
#if HAVE_SYSCTLBYNAME
	unsigned  val;
	size_t    size;

	HELP ("sysctlbyname() machdep.i586_freq");

	size = sizeof(val);
	if (sysctlbyname ("machdep.i586_freq", &val, &size, NULL, 0) == 0
			&& size == sizeof(val)) {
		speed_cycletime = 1.0 / (double) val;
		return 1;
	}
#endif
	return 0;
}

/* i368 FreeBSD 3.3 sysctlbyname machdep.tsc_freq is in Hertz.
   There's no obvious defines to get this from plain sysctl.  */
static int
freq_sysctlbyname_tsc_freq (int help) {
#if HAVE_SYSCTLBYNAME
	unsigned  val;
	size_t    size;

	HELP ("sysctlbyname() machdep.tsc_freq");

	size = sizeof(val);
	if (sysctlbyname ("machdep.tsc_freq", &val, &size, NULL, 0) == 0
			&& size == sizeof(val)) {
		speed_cycletime = 1.0 / (double) val;
		return 1;
	}
#endif
	return 0;
}

/* Apple powerpc Darwin 1.3 sysctl hw.cpufrequency is in hertz.  For some
   reason only seems to be available from sysctl(), not sysctlbyname().  */
static int
freq_sysctl_hw_cpufrequency (int help) {
#if HAVE_SYSCTL && defined (CTL_HW) && defined (HW_CPU_FREQ)
	int       mib[2];
	unsigned  val;
	size_t    size;

	HELP ("sysctl() hw.cpufrequency");

	mib[0] = CTL_HW;
	mib[1] = HW_CPU_FREQ;
	size = sizeof(val);
	if (sysctl (mib, 2, &val, &size, NULL, 0) == 0) {
		speed_cycletime = 1.0 / (double) val;
		return 1;
	}
#endif
	return 0;
}

/* The following ssyctl hw.model strings have been observed,

       Alpha FreeBSD 4.1:   Digital AlphaPC 164LX 599 MHz
       NetBSD 1.4:          Digital AlphaPC 164LX 599 MHz
       NetBSD 1.6.1:        CY7C601 @ 40 MHz, TMS390C602A FPU

   NetBSD 1.4 doesn't seem to have sysctlbyname, so sysctl() is used.  */
static int
freq_sysctl_hw_model (int help) {
#if HAVE_SYSCTL && defined (CTL_HW) && defined (HW_MODEL)
	int       mib[2];
	char      str[128];
	unsigned  val;
	size_t    size;
	char      *p;
	int       end;

	HELP ("sysctl() hw.model");

	mib[0] = CTL_HW;
	mib[1] = HW_MODEL;
	size = sizeof(str);
	if (sysctl (mib, 2, str, &size, NULL, 0) == 0) {
		for (p = str; *p != '\0'; p++) {
			end = 0;
			if (sscanf (p, "%u MHz%n", &val, &end) == 1 && end != 0) {
				speed_cycletime = 1e-6 / (double) val;
				return 1;
			}
		}
	}
#endif
	return 0;
}

/* /proc/cpuinfo for linux kernel.

   Linux doesn't seem to have any system call to get the CPU frequency, at
   least not in 2.0.x or 2.2.x, so it's necessary to read /proc/cpuinfo.

   i386 2.0.36 - "bogomips" is the CPU frequency.

   i386 2.2.13 - has both "cpu MHz" and "bogomips", and it's "cpu MHz" which
                 is the frequency.

   alpha 2.2.5 - "cycle frequency [Hz]" seems to be right, "BogoMIPS" is
                 very slightly different.

   alpha 2.2.18pre21 - "cycle frequency [Hz]" is 0 on at least one system,
                 "BogoMIPS" seems near enough.

   powerpc 2.2.19 - "clock" is the frequency, bogomips is something weird
  */
static int
freq_proc_cpuinfo (int help) {
	FILE    *fp;
	char    buf[128];
	double  val;
	int     ret = 0;
	int     end;

	HELP ("linux kernel /proc/cpuinfo file, cpu MHz or bogomips");

	if ((fp = fopen ("/proc/cpuinfo", "r")) != NULL) {
		while (fgets (buf, sizeof (buf), fp) != NULL) {
			if (sscanf (buf, "cycle frequency [Hz]    : %lf", &val) == 1
					&& val != 0.0) {
				speed_cycletime = 1.0 / val;
				ret = 1;
				break;
			}
			if (sscanf (buf, "cpu MHz : %lf\n", &val) == 1) {
				goto MHz;
			}
			end = 0;
			if (sscanf (buf, "clock : %lfMHz\n%n", &val, &end) == 1 && end != 0) {
				goto MHz;
			}
			if (sscanf (buf, "bogomips : %lf\n", &val) == 1
					|| sscanf (buf, "BogoMIPS : %lf\n", &val) == 1) {
			MHz:
				speed_cycletime = 1e-6 / val;
				ret = 1;
				break;
			}
		}
		fclose (fp);
	}
	return ret;
}

/* /bin/sysinfo for SunOS 4.
   Prints a line like: cpu0 is a "75 MHz TI,TMS390Z55" CPU */
static int
freq_sunos_sysinfo (int help)
{
	int     ret = 0;
#if HAVE_POPEN
	FILE    *fp;
	char    buf[128];
	double  val;
	int     end;

	HELP ("SunOS /bin/sysinfo program output, cpu0");

	/* Error messages are sent to /dev/null in case /bin/sysinfo doesn't
     exist.  The brackets are necessary for some shells. */
	if ((fp = popen ("(/bin/sysinfo) 2>/dev/null", "r")) != NULL) {
		while (fgets (buf, sizeof (buf), fp) != NULL) {
			end = 0;
			if (sscanf (buf, " cpu0 is a \"%lf MHz%n", &val, &end) == 1
					&& end != 0) {
				speed_cycletime = 1e-6 / val;
				ret = 1;
				break;
			}
		}
		pclose (fp);
	}
#endif
	return ret;
}

/* "/etc/hw -r cpu" for SCO OpenUnix 8, printing a line like
	The speed of the CPU is approximately 450MHz */
static int
freq_sco_etchw (int help) {
	int     ret = 0;
#if HAVE_POPEN
	FILE    *fp;
	char    buf[128];
	double  val;
	int     end;

	HELP ("SCO /etc/hw program output");

	/* Error messages are sent to /dev/null in case /etc/hw doesn't exist.
     The brackets are necessary for some shells. */
	if ((fp = popen ("(/etc/hw -r cpu) 2>/dev/null", "r")) != NULL) {
		while (fgets (buf, sizeof (buf), fp) != NULL) {
			end = 0;
			if (sscanf (buf, " The speed of the CPU is approximately %lfMHz%n",
					&val, &end) == 1 && end != 0) {
				speed_cycletime = 1e-6 / val;
				ret = 1;
				break;
			}
		}
		pclose (fp);
	}
#endif
	return ret;
}

/* attr_get("/hw/cpunum/0",INFO_LBL_DETAIL_INVENT) ic_cpu_info.cpufq for
   IRIX 6.5.  Past versions don't have INFO_LBL_DETAIL_INVENT,
   invent_cpuinfo_t, or /hw/cpunum/0.

   The same information is available from the "hinv -c processor" command,
   but it seems better to make a system call where possible. */
static int
freq_attr_get_invent (int help) {
	int     ret = 0;
#if HAVE_ATTR_GET && HAVE_INVENT_H && defined (INFO_LBL_DETAIL_INVENT)
	invent_cpuinfo_t  inv;
	int               len, val;

	HELP ("attr_get(\"/hw/cpunum/0\") ic_cpu_info.cpufq");

	len = sizeof (inv);
	if (attr_get ("/hw/cpunum/0", INFO_LBL_DETAIL_INVENT,
			(char *) &inv, &len, 0) == 0
			&& len == sizeof (inv)
			&& inv.ic_gen.ig_invclass == INV_PROCESSOR) {
		val = inv.ic_cpu_info.cpufq;
		speed_cycletime = 1e-6 / val;
		ret = 1;
	}
#endif
	return ret;
}

/* FreeBSD on i386 gives a line like the following at bootup, and which can
   be read back from /var/run/dmesg.boot.

       CPU: AMD Athlon(tm) Processor (755.29-MHz 686-class CPU)
       CPU: Pentium 4 (1707.56-MHz 686-class CPU)
       CPU: i486 DX4 (486-class CPU)

   This is useful on FreeBSD 4.x, where there's no sysctl machdep.tsc_freq
   or machdep.i586_freq.

   It's better to use /var/run/dmesg.boot than to run /sbin/dmesg, since the
   latter prints the current system message buffer, which is a limited size
   and can wrap around if the system is up for a long time.  */
static int
freq_bsd_dmesg (int help) {
	FILE    *fp;
	char    buf[256], *p;
	double  val;
	int     ret = 0;
	int     end;

	HELP ("BSD /var/run/dmesg.boot file");

	if ((fp = fopen ("/var/run/dmesg.boot", "r")) != NULL) {
		while (fgets (buf, sizeof (buf), fp) != NULL) {
			if (memcmp (buf, "CPU:", 4) == 0) {
				for (p = buf; *p != '\0'; p++) {
					end = 0;
					if (sscanf (p, "(%lf-MHz%n", &val, &end) == 1 && end != 0) {
						speed_cycletime = 1e-6 / val;
						ret = 1;
						break;
					}
				}
			}
		}
		fclose (fp);
	}
	return ret;
}

/* "hinv -c processor" for IRIX.  The following lines have been seen,

              1 150 MHZ IP20 Processor
              2 195 MHZ IP27 Processors
              Processor 0: 500 MHZ IP35

   This information is available from attr_get() on IRIX 6.5 (see above),
   but on IRIX 6.2 it's not clear where to look, so fall back on
   parsing.  */
static int
freq_irix_hinv (int help) {
	int     ret = 0;
#if HAVE_POPEN
	FILE    *fp;
	char    buf[128];
	double  val;
	int     nproc, end;

	HELP ("IRIX \"hinv -c processor\" output");

	/* Error messages are sent to /dev/null in case hinv doesn't exist.  The
     brackets are necessary for some shells. */
	if ((fp = popen ("(hinv -c processor) 2>/dev/null", "r")) != NULL) {
		while (fgets (buf, sizeof (buf), fp) != NULL) {
			end = 0;
			if (sscanf (buf, "Processor 0: %lf MHZ%n", &val, &end) == 1
					&& end != 0) {
			found:
				speed_cycletime = 1e-6 / val;
				ret = 1;
				break;
			}
			end = 0;
			if (sscanf (buf, "%d %lf MHZ%n", &nproc, &val, &end) == 2
					&& end != 0) {
				goto found;
			}
		}
		pclose (fp);
	}
#endif
	return ret;
}

/* processor_info() for Solaris.  "psrinfo" is the command-line interface to
   this.  "prtconf -vp" gives similar information.

   Apple Darwin has a processor_info, but in an incompatible style.  It
   doesn't have <sys/processor.h>, so test for that.  */
static int
freq_processor_info (int help) {
#if HAVE_PROCESSOR_INFO && HAVE_SYS_PROCESSOR_H
	processor_info_t  p;
	int  i, n, mhz = 0;

	HELP ("processor_info() pi_clock");

	n = sysconf (_SC_NPROCESSORS_CONF);
	for (i = 0; i < n; i++) {
		if (processor_info (i, &p) != 0) {
			continue;
		}
		if (p.pi_state != P_ONLINE) {
			continue;
		}

		if (mhz != 0 && p.pi_clock != mhz) {
			fprintf (stderr,
					"freq_processor_info(): There's more than one CPU and they have different clock speeds\n");
			return 0;
		}

		mhz = p.pi_clock;
	}

	speed_cycletime = 1.0e-6 / (double) mhz;

	return 1;

#else
	return 0;
#endif
}



#if HAVE_SPEED_CYCLECOUNTER && HAVE_GETTIMEOFDAY
static double
freq_measure_gettimeofday_one (void) {
#define call_gettimeofday(t)   gettimeofday (&(t), NULL)
#define timeval_tv_sec(t)      ((t).tv_sec)
#define timeval_tv_usec(t)     ((t).tv_usec)
	FREQ_MEASURE_ONE ("gettimeofday", struct timeval,
			call_gettimeofday, speed_cyclecounter,
			timeval_tv_sec, timeval_tv_usec);
}
#endif

#if HAVE_SPEED_CYCLECOUNTER && HAVE_GETRUSAGE
static double
freq_measure_getrusage_one (void) {
#define call_getrusage(t)   getrusage (0, &(t))
#define rusage_tv_sec(t)    ((t).ru_utime.tv_sec)
#define rusage_tv_usec(t)   ((t).ru_utime.tv_usec)
	FREQ_MEASURE_ONE ("getrusage", struct rusage,
			call_getrusage, speed_cyclecounter,
			rusage_tv_sec, rusage_tv_usec);
}
#endif


/* MEASURE_MATCH is how many readings within MEASURE_TOLERANCE of each other
   are required.  This must be at least 2.  */
#define MEASURE_MAX_ATTEMPTS   20
#define MEASURE_TOLERANCE      1.005  /* 0.5% */
#define MEASURE_MATCH          3

int
double_cmp_ptr (const double *p, const double *q) {
	if (*p > *q)  return 1;
	if (*p < *q)  return -1;
	return 0;
}

double
freq_measure (const char *name, double (*one) (void)) {
	double  t[MEASURE_MAX_ATTEMPTS];
	int     i, j;

	for (i = 0; i < numberof (t); i++) {
		t[i] = (*one) ();

		qsort (t, i+1, sizeof(t[0]), (qsort_function_t) double_cmp_ptr);

		for (j = 0; j+MEASURE_MATCH-1 <= i; j++) {
			if (t[j+MEASURE_MATCH-1] <= t[j] * MEASURE_TOLERANCE) {
				/* use the average of the range found */
				return (t[j+MEASURE_MATCH-1] + t[j]) / 2.0;
			}
		}
	}
	return -1.0;
}

static int
freq_measure_getrusage (void) {
#if HAVE_SPEED_CYCLECOUNTER && HAVE_GETRUSAGE
	double  cycletime;

	if (! getrusage_microseconds_p ()) {
		return 0;
	}
	if (! cycles_works_p ()) {
		return 0;
	}

	cycletime = freq_measure ("getrusage", freq_measure_getrusage_one);
	if (cycletime == -1.0) {
		return 0;
	}

	speed_cycletime = cycletime;
	return 1;
#else
	return 0;
#endif
}

static int
freq_measure_gettimeofday (void) {
#if HAVE_SPEED_CYCLECOUNTER && HAVE_GETTIMEOFDAY
	double  cycletime;

	if (! gettimeofday_microseconds_p ()) {
		return 0;
	}
	if (! cycles_works_p ()) {
		return 0;
	}

	cycletime = freq_measure ("gettimeofday", freq_measure_gettimeofday_one);
	if (cycletime == -1.0) {
		return 0;
	}

	speed_cycletime = cycletime;
	return 1;
#else
	return 0;
#endif
}


/* Each function returns 1 if it succeeds in setting speed_cycletime, or 0
   if not.

   In general system call tests are first since they're fast, then file
   tests, then tests running programs.  Necessary exceptions to this rule
   are noted.  The measuring is last since it's time consuming, and rather
   wasteful of cpu.  */
static int
freq_all (int help) {
	return
		/* This should be first, so an environment variable can override
		 * anything the system gives. */
		freq_environment (help)
		|| freq_attr_get_invent (help)
		|| freq_getsysinfo (help)
		|| freq_pstat_getprocessor (help)
		|| freq_sysctl_hw_model (help)
		|| freq_sysctl_hw_cpufrequency (help)
		|| freq_sysctlbyname_i586_freq (help)
		|| freq_sysctlbyname_tsc_freq (help)
		/* SCO openunix 8 puts a dummy pi_clock==16 in processor_info, so be
		 * sure to check /etc/hw before that function. */
		|| freq_sco_etchw (help)
		|| freq_processor_info (help)
		|| freq_proc_cpuinfo (help)
		|| freq_bsd_dmesg (help)
		|| freq_irix_hinv (help)
		|| freq_sunos_sysinfo (help)
		|| freq_measure_getrusage (help)
		|| freq_measure_gettimeofday (help);
}


void
speed_cycletime_init (void) {
	static int  attempted = 0;

	if (attempted) {
		return;
	}
	attempted = 1;

	if (freq_all (0))
		return;

	fprintf (stderr, "CPU frequency couldn't be determined\n");
}

void
speed_cycletime_fail (const char *str) {
	fprintf (stderr, "Measuring with: %s\n", speed_time_string);
	fprintf (stderr, "%s,\n", str);
	fprintf (stderr, "but none of the following are available,\n");
	freq_all (1);
	abort ();
}

/* speed_time_init leaves speed_cycletime set to either 0.0 or 1.0 when the
   CPU frequency is unknown.  0.0 is when the time base is in seconds, so
   that's no good if cycles are wanted.  1.0 is when the time base is in
   cycles, which conversely is no good if seconds are wanted.  */
void
speed_cycletime_need_cycles (void) {
	speed_time_init ();
	if (speed_cycletime == 0.0) {
		speed_cycletime_fail
		("Need to know CPU frequency to give times in cycles");
	}
}
void
speed_cycletime_need_seconds (void) {
	speed_time_init ();
	if (speed_cycletime == 1.0) {
		speed_cycletime_fail
		("Need to know CPU frequency to convert cycles to seconds");
	}
}

