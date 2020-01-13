/*
 * tuneup.c
 *
 *  Created on: Jan 7, 2020, 9:10:57 AM
 *      Author: Joshua Fehrenbach
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "speed.h"
#include "tmp-alloc.h"
#include "yarrow.h"

#define DEFAULT_MAX_SIZE   10000   /* bytes */

struct dat_t {
  size_t  size;
  double     d;
};

static struct dat_t *dat = NULL;
static int  ndat = 0;
static int  allocdat = 0;

struct speed_params s;

void
add_dat(size_t size, double d) {
#define ALLOCDAT_STEP  500
	ASSERT(ndat <= allocdat);

	if (ndat == allocdat) {
		allocdat += ALLOCDAT_STEP;
		dat = (struct dat_t*) (dat == NULL
				? malloc(allocdat * sizeof(struct dat_t))
				: realloc(dat, allocdat * sizeof(struct dat_t)));
	}
	dat[ndat].size = size;
	dat[ndat].d = d;
	ndat++;
}

/* Return the threshold size based on the data accumulated. */
size_t
analyze_dat() {
	double  x, min_x;
	int     j, min_j;

	/* If the threshold is set at dat[0].size, any positive values are bad. */
	x = 0.0;
	for (j = 0; j < ndat; j++) {
		if (dat[j].d > 0.0) {
			x += dat[j].d;
		}
	}

	min_x = x;
	min_j = 0;

	/* When stepping to the next dat[j].size, positive values are no longer
	 * bad (so subtracted), negative values become bad (so add the absolute
	 * value, meaning subtract). */
	for (j = 0; j < ndat; x -= dat[j].d, j++) {
		if (x < min_x) {
			min_x = x;
			min_j = j;
		}
	}

	return min_j;
}


double
tuneup_measure(speed_function_t fun, const struct param_t *param, struct speed_params *s) {
	double t;
	TMP_DECL(s_xp, uint8_t);
	TMP_DECL(s_yp, uint8_t);

	s->size += param->size_extra;
	SPEED_TMP_ALLOC(s_xp, s->xp, s->size, 0);
	SPEED_TMP_ALLOC(s_yp, s->yp, s->size, 0);

	(*s->random)(s->rand_ctx, s->xp, s->size);
	(*s->random)(s->rand_ctx, s->yp, s->size);

	t = speed_measure(fun, s);

	s->size -= param->size_extra;

	TMP_FREE(s_yp);
	TMP_FREE(s_xp);

	return t;
}



#define PRINT_WIDTH  31

void
print_define_start(const char *name) {
	printf("#define %-*s  ", PRINT_WIDTH, name);
}

void
print_define_end_remark(size_t value, const char *remark) {
	if (value >= SIZE_MAX) {
		printf("SIZE_MAX");
	} else {
		printf("%5ld", (value > 0 ? (long) value : 0L));
	}

	if (remark != NULL) {
		printf("  /* %s */", remark);
	}
	printf("\n");
	fflush(stdout);
}

void
print_define_end(size_t value) {
	const char *remark;
	if (value >= SIZE_MAX) {
		remark = "never";
	} else if (value <= 0) {
		remark = "always";
	} else {
		remark = NULL;
	}
}

void
print_define(const char *name, size_t value) {
	print_define_start(name);
	print_define_end(value);
}

void
print_define_remark(const char *name, size_t value, const char *remark) {
	print_define_start(name);
	print_define_end_remark(value, remark);
}

void
one(size_t *threshold, struct param_t *param) {
	int since_positive, since_thresh_change;
	int thresh_idx, new_thresh_idx;

#define DEFAULT(x,n)  do { if (! (x)) { (x) = (n); } } while (0)

	DEFAULT(param->function_fudge, 1.0);
	DEFAULT(param->function2, param->function);
	DEFAULT(param->step_factor, 0.01);	/* small steps by default */
	DEFAULT(param->step, 1);			/* small steps by default */
	DEFAULT(param->stop_since_change, 80);
	DEFAULT(param->stop_factor, 1.2);
	DEFAULT(param->min_size, 10);
	DEFAULT(param->max_size, DEFAULT_MAX_SIZE);



	if (param->check_size != 0) {
		double t1, t2;
		s.size = param->check_size;

		*threshold = s.size + 1;
		t1 = tuneup_measure(param->function, param, &s);

		*threshold = s.size;
		t2 = tuneup_measure(param->function, param, &s);

		if (t1 == -1.0 || t2 == -1.0) {
			printf("Oops, can't run the function as size %ld\n", (long) s.size);
			abort();
		}
		t1 *= param->function_fudge;

		/* check that t2 is at least 4% below t1 */
		if (t1 < t2*1.04) {
			fprintf(stderr, "function2 never enough faster: t1=%0.9f t2=%0.9f\n", t1, t2);
			*threshold = SIZE_MAX;
			if (!param->noprint) {
				print_define(param->name, *threshold);
			}
			return;
		}
	}

	if (!param->noprint) {
		print_define_start(param->name);
	}

	ndat = 0;
	since_positive = 0;
	since_thresh_change = 0;
	thresh_idx = 0;

	for (s.size = param->min_size; s.size < param->max_size;
			s.size += MAX((size_t) floor(s.size * param->step_factor), param->step)) {
		double ti, tip1, d;

		*threshold = s.size + 1;
		ti = tuneup_measure(param->function, param, &s);
		if (ti == -1.0) {
			if (!param->noprint) {
				printf("-1\n");
				fflush(stdout);
			}
			fprintf(stderr, "function failed with threshold=%ld\n", *threshold);
			abort();
		}
		ti *= param->function_fudge;

		*threshold = s.size;
		tip1 = tuneup_measure(param->function2, param, &s);
		if (tip1 == -1.0) {
			if (!param->noprint) {
				printf("-1\n");
				fflush(stdout);
			}
			fprintf(stderr, "function2 failed with threshold=%ld\n", *threshold);
			abort();
		}

		/* calculate the fraction by which one or the other routine is slower */
		if (tip1 >= ti) {
			d = (tip1 - ti) / tip1;		/* negative */
		} else {
			d = (tip1 - ti) / ti;		/* positive */
		}

		add_dat(s.size, d);
		new_thresh_idx = analyze_dat();

		/* stop if the last time method i was faster was more than a
		 * certain number of measurements ago */
#define STOP_SINCE_POSITIVE  200
		if (d >= 0) {
			since_positive = 0;
		} else if (++since_positive > STOP_SINCE_POSITIVE) {
			break;
		}

		/* stop of function has become slower by a certain factor */
		if (ti >= tip1*param->stop_factor) {
			break;
		}

		/* Stop if the threshold implied hasn't changed in a certain
		 * number of measurements. (It's this condition that usually
		 * stops the loop) */
		if (thresh_idx != new_thresh_idx) {
			since_thresh_change = 0;
			thresh_idx = new_thresh_idx;
		} else if (++since_thresh_change > param->stop_since_change) {
			break;
		}

		/* Stop if the threshold implied is more than a certain number of
		 * measurements ago. */
#define STOP_SINCE_AFTER  500
		if (ndat - thresh_idx > STOP_SINCE_AFTER) {
			break;
		}

		/* Stop when the size limit is reached before the end of the
		 * crossover, but only show this as an error for >= the default
		 * max size. */
		if (s.size >= param->max_size && param->max_size >= DEFAULT_MAX_SIZE) {
			fprintf(stderr, "%s\n", param->name);
			fprintf(stderr, "sizes %ld to %ld total %d measurements\n",
					(long) dat[0].size, (long) dat[ndat-1].size, ndat);
			fprintf(stderr, "    max size reached before end of crossover\n");
			break;
		}
	}

	*threshold = dat[analyze_dat()].size;

	if (param->min_is_always && *threshold == param->min_size) {
		*threshold = 0;
	}

	if (!param->noprint) {
		print_define_end(*threshold);
	}
}

























random_function_t default_random = NULL;
random_close_function_t default_random_close = NULL;

void
gmp_random(void *ctx, mpz_ptr ptr, mp_size_t size) {
	mpz_random(ptr, size);
}

void
unix_random(void *ctx, mpz_ptr ptr, mp_size_t size) {
	size_t read;
	FILE *fp = fopen("/dev/urandom", "rb");

	ASSERT (fp != NULL);

	do {
		read = fread(ptr, 1, size, fp);
		ptr += read;
		size -= read;
	} while (size > 0);

	fclose(fp);
}

#if defined(_WIN32) || defined(__CYGWIN__)

#include <bcrypt.h>

static BCRYPT_ALG_HANDLE windows_random_handle = NULL;
static int register_exit_handle = 1;

#define BCRAND(handle, ptr, size, flags) BCryptGenRandom((BCRYPT_ALG_HANDLE) (handle), \
		(PUCHAR) (ptr), (ULONG) (size), (ULONG) flags)
#define BCOPEN(handle, alg, prov, flags) BCryptOpenAlgorithmProvider( \
		(BCRYPT_ALG_HANDLE*) &(handle), (LPCWSTR) (alg), (LPCWSTR) (prov), (ULONG) (flags))


__cdecl void
windows_random_cleanup(void) {
	if (windows_random_handle != NULL) {
		BCryptCloseAlgorithmProvider(windows_random_handle, (ULONG) 0);
		windows_random_handle = NULL;
	}
}

void
windows_random(void *ctx, mpz_ptr ptr, mp_size_t size) {
	/* asume that, if ctx is non-null, then it is a
	 * BCRYPT_ALG_HANDLE object for an RNG */
	if (ctx != NULL && BCRYPT_SUCCESS(BCRAND(ctx, ptr, size, 0))) {
		/* ctx is a valid BCRYPT_ALG_HANDLER, and BCryptGenRandom
		 * succeeded, so return here */
		return;
	}
	/* either ctx is not a BCRYPT_ALG_HANDLE, or ctx is null, so
	 * try to use the system-preferred RNG, if the static
	 * BCRYPT_ALG_HANDLE has not yet been initialized */
	if (windows_random_handle == NULL) {
		/* do not have static BCRYPT_ALG_HANDLE, so try the system-preferred */
		if (BCRYPT_SUCCESS(BCRAND(NULL, ptr, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG))) {
			/* success, so return */
			return;
		}
		/* default failed, so attempt to initialize and use
		 * the static BCRYPT_ALG_HANDLE */
		if (!BCRYPT_SUCCESS(BCOPEN(windows_random_handle, BCRYPT_RNG_ALGORITHM, NULL, 0))) {
			/* could not create a BCRYPT_ALG_HANDLE, so skip to libc fallback */
			goto fail;
		}
		/* register exit handler, if necessary */
		if (register_exit_handle) {
			register_exit_handle = atexit(&windows_random_cleanup);
		}
	}
	/* use static BCRYPT_ALG_HANDLE */
	if (BCRYPT_SUCCESS(BCRAND(windows_random_handle, ptr, size, 0))) {
		/* success, so return */
		return;
	}
	/* static failed, so clean up the static BCRYPT_ALG_HANDLE and switch
	 * to the libc fallback function */
	fail:
	windows_random_cleanup();
	default_random = &gmp_random;
	gmp_random(ctx, ptr, size);
}

#endif

