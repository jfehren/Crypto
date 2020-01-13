/*
 * crypto-buffer.h
 *
 *  Created on: Oct 24, 2019, 11:04:12 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef CRYPTO_BUFFER_H_
#define CRYPTO_BUFFER_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

struct crypto_buffer {
	uint8_t *contents;
	/* Allocated size */
	size_t alloc;

	void *realloc_ctx;
	crypto_realloc_func *realloc;

	/* Current size */
	size_t size;
};

/* Initializes a buffer that uses plain realloc */
__CRYPTO_DECLSPEC void
crypto_buffer_init(struct crypto_buffer *buffer);

/* Initialize a buffer with user-defined reallocation function */
__CRYPTO_DECLSPEC void
crypto_buffer_init_realloc(struct crypto_buffer *buffer,
		void *realloc_ctx, crypto_realloc_func *realloc);

/* Initializes a buffer with fixed allocation */
__CRYPTO_DECLSPEC void
crypto_buffer_init_size(struct crypto_buffer *buffer, uint8_t *space, size_t length);

/* Releases the memory allocated to this buffer */
__CRYPTO_DECLSPEC void
crypto_buffer_clear(struct crypto_buffer *buffer);

/* Resets the buffer, without freeing the buffer space. */
__CRYPTO_DECLSPEC void
crypto_buffer_reset(struct crypto_buffer *buffer);

/* Increase the size of the buffer by length bytes */
__CRYPTO_DECLSPEC int
crypto_buffer_grow(struct crypto_buffer *buffer, size_t length);

/* Insert a single uint8_t value at the end of the buffer,
 * increasing the buffer's allocated size if necessary.
 * Returns 1 on success, 0 on failure. */
#define CRYPTO_BUFFER_PUTC(buffer, c) \
	( (((buffer)->size < (buffer)->alloc) || crypto_buffer_grow((buffer), 1)) \
			&& ((buffer)->contents[(buffer)->size++] = (c), 1) )

/* Write length bytes from data to the end of the buffer,
 * increasing the buffer's allocated size if necessary.
 * Returns 1 on success, 0 on failure. */
__CRYPTO_DECLSPEC int
crypto_buffer_write(struct crypto_buffer *buffer, const uint8_t *data, size_t length);

/* Like crypto_buffer_write, but instead of copying data to the
 * buffer, it returns a pointer to the area where the caller can copy
 * the data. The pointer is valid only until the next call that can
 * reallocate the buffer.
 * Returns NULL if the needed space could not be obtained. */
__CRYPTO_DECLSPEC uint8_t *
crypto_buffer_space(struct crypto_buffer *buffer, size_t length);

/* Copy the contents of SRC to the end of DST.
 * Returns 1 on success, 0 on failure. */
__CRYPTO_DECLSPEC int
crypto_buffer_copy(struct crypto_buffer *dst, const struct crypto_buffer *src);

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_BUFFER_H_ */
