/*
 * rsa-sign.c
 *
 *  Created on: Nov 4, 2019, 6:24:44 PM
 *      Author: Joshua Fehrenbach
 */

#include "rsa.h"

void
rsa_compute_root(const struct rsa_private_key *key, mpz_ptr x, mpz_srcptr m) {
	mpz_t xp, xq;	/* modulus p and q */

	mpz_init(xp);
	mpz_init(xq);

	/* Compute xq = m^d (mod q) = (m (mod q))^b (mod q) */
	mpz_fdiv_r(xq, m, key->q);
	mpz_powm_sec(xq, xq, key->b, key->q);

	/* Compute xp = m^d (mod p) = (m (mod p))^a (mod p) */
	mpz_fdiv_r(xp, m, key->p);
	mpz_powm_sec(xp, xp, key->a, key->p);

	/* Set xp' = (xp - xq) * c (mod p) */
	mpz_sub(xp, xp, xq);
	mpz_mul(xp, xp, key->c);
	mpz_fdiv_r(xp, xp, key->p);

	/* Compute x = xq + q*xp
	 *
	 * Proof:
	 *
	 * Note that
	 *
	 *   xp  = x + i*p,
	 *   xq  = x + j*q,
	 *   c*q = 1 + k*p
	 *
	 * for some integers i, j and k. Now, for some integer l,
	 *
	 *   xp' = (xp - xq)*c + l*p
	 *       = (x + i*p - (x + j*q))*c + l*p
	 *       = (i*p - j*q)*c + l*p
	 *       = (i*c + l)*p - j*(c*q)
	 *       = (i*c + l)*p - j*(1 + k*p)
	 *       = (i*c + l - j*k)*p - j
	 *
	 * which shows that xp' = -j (mod p). We get
	 *
	 *   xq + q*xp' = x + j*q + (i*c + l - j*k)*p*q - j*q
	 *              = x + (i*c + l - j*k)*p*q
	 *
	 * so that
	 *
	 *   xq + q*xp' = x (mod p*q)
	 *
	 * We also get
	 *   0 <= xq + q*xp' < p*q,
	 * because
	 *   0 <= xq < q and 0 <= xp' < p.
	 */
	mpz_mul(x, key->q, xp);
	mpz_add(x, x, xq);

	mpz_clear(xq);
	mpz_clear(xp);
}

/* Blinds m by computing c = m(r^e) (mod n), for a random r. Also
 * returns the inverse (ri), for use by rsa_unblind. */
static void
rsa_blind(const struct rsa_public_key *pub,
		mpz_ptr c, mpz_ptr ri, mpz_srcptr m,
		void *random_ctx, crypto_random_func *random) {
	mpz_t r;

	mpz_init(r);

	/* c = m*(r^e)
	 * ri = r^(-1) */
	do {
		crypto_mpz_random(r, pub->n, random_ctx, random);
	} while (!mpz_invert(ri, r, pub->n));

	/* c = c*(r^e) mod n */
	mpz_powm_sec(r, r, pub->e, pub->n);
	mpz_mul(c, m, r);
	mpz_fdiv_r(c, c, pub->n);

	mpz_clear(r);
}

/* m = c*ri (mod n) */
static void
rsa_unblind(const struct rsa_public_key *pub,
		mpz_ptr m, mpz_srcptr ri, mpz_srcptr c) {
	mpz_mul(m, c, ri);
	mpz_fdiv_r(m, m, pub->n);
}

/* Checks for any error in the RSA computation. That avoids
 * attacks which rely on faults on hardware, or even software
 * MPI implementation. */
int
rsa_compute_root_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr x, mpz_srcptr m,
		void *random_ctx, crypto_random_func *random) {
	int res;
	mpz_t t, mb, xb, ri;

	/* mpz_powm_sec handles only odd moduli. If p, q, or n are even, the
	 * key is invalid and rejected by rsa_private_key_prepare. However,
	 * some applications, notably gnutls, don't use this function, and
	 * we don't want an invalid key to lead to a crash down inside
	 * mpz_powm_sec. So do an additional check here. */
	if (mpz_even_p(pub->n) || mpz_even_p(key->p) || mpz_even_p(key->q)) {
		return 0;
	}

	mpz_init(mb);
	mpz_init(xb);
	mpz_init(ri);
	mpz_init(t);

	rsa_blind(pub, mb, ri, m, random_ctx, random);

	rsa_compute_root(key, xb, mb);

	mpz_powm_sec(t, xb, pub->e, pub->n);
	res = (mpz_cmp(mb, t) == 0);

	if (res) {
		rsa_unblind(pub, x, ri, xb);
	}

	mpz_clear(t);
	mpz_clear(ri);
	mpz_clear(xb);
	mpz_clear(mb);

	return res;
}
