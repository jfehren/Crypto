/*
 * buffer.c
 *
 *  Created on: Oct 24, 2019, 11:50:31 AM
 *      Author: Joshua Fehrenbach
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "crypto-buffer.h"
#include "crypto-realloc.h"

void
crypto_buffer_init(struct crypto_buffer *buffer) {
	buffer->contents = NULL;
	buffer->alloc = 0;
	buffer->realloc_ctx = NULL;
	buffer->realloc = &crypto_realloc;
	buffer->size = 0;
}

void
crypto_buffer_init_realloc(struct crypto_buffer *buffer,
		void *realloc_ctx, crypto_realloc_func *reallocp) {
	buffer->contents = NULL;
	buffer->alloc = 0;
	buffer->realloc_ctx = realloc_ctx;
	buffer->realloc = reallocp;
	buffer->size = 0;
}

void
crypto_buffer_init_size(struct crypto_buffer *buffer, uint8_t *space, size_t length) {
	buffer->contents = space;
	buffer->alloc = length;
	buffer->realloc_ctx = NULL;
	buffer->realloc = NULL;
	buffer->size = 0;
}

void
crypto_buffer_clear(struct crypto_buffer *buffer) {
	/* free buffer, if it was not allocated at a fixed size */
	buffer->contents = (buffer->realloc != NULL
			? buffer->realloc(buffer->realloc_ctx, buffer->contents, 0) : NULL);
	buffer->alloc = buffer->size = 0;
}

void
crypto_buffer_reset(struct crypto_buffer *buffer) {
	buffer->size = 0;
}

int
crypto_buffer_grow(struct crypto_buffer *buffer, size_t length) {
	assert (buffer->size <= buffer->alloc);
	/* Check if buffer is large enough for requested space */
	if (buffer->size + length > buffer->alloc) {
		/* Buffer not large enough, so attempt to grow */
		size_t alloc;
		uint8_t *p;

		if (buffer->realloc == NULL) {
			/* Buffer has fixed size */
			return 0;
		}

		/* Attempt to allocate increased buffer */
		alloc = buffer->alloc*2 + length + 100;
		p = buffer->realloc(buffer->realloc_ctx, buffer->contents, alloc);
		if (p == NULL) {
			/* Could not increase by factor of 2, so try to increase
			 * only as much as is needed */
			alloc = buffer->size + length;
			p = buffer->realloc(buffer->realloc_ctx, buffer->contents, alloc);
			if (p == NULL) {
				/* Still could not increase buffer size, so return failure */
				return 0;
			}
		}
		/* Buffer size increased */
		buffer->contents = p;
		buffer->alloc = alloc;
	}
	return 1;
}

int
crypto_buffer_write(struct crypto_buffer *buffer, const uint8_t *data, size_t length) {
	/* Get the needed buffer space */
	uint8_t *p = crypto_buffer_space(buffer, length);
	if (p == NULL) {
		/* Could not get the needed space */
		return 0;
	}
	/* Copy the data into the buffer */
	memcpy(p, data, length);
	return 1;
}

uint8_t*
crypto_buffer_space(struct crypto_buffer *buffer, size_t length) {
	uint8_t *p;
	/* Ensure we have the needed space */
	if (!crypto_buffer_grow(buffer, length)) {
		/* Could not ensure the needed space */
		return NULL;
	}
	p = buffer->contents + buffer->size;
	buffer->size += length;		/* mark the returned space as used */
	return p;
}

int
crypto_buffer_copy(struct crypto_buffer *dst, const struct crypto_buffer *src) {
	/* Simply write the contents of src to dst */
	return crypto_buffer_write(dst, src->contents, src->size);
}

