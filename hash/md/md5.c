/*
 * md5.c
 *
 *  Created on: Aug 26, 2019, 1:58:08 PM
 *      Author: Joshua Fehrenbach
 */

#include "md5-internal.h"
#include "crypto-internal.h"
#include "memwrite.h"
#include "macros.h"
#include <assert.h>
#include <string.h>

void
md5_init(struct md5_ctx *ctx) {
	const uint32_t iv[_MD5_DIGEST_LENGTH] = {
		0x67452301,
		0xefcdab89,
		0x98badcfe,
		0x10325476
	};
	memcpy(ctx->state, iv, sizeof(ctx->state));
	memset(ctx->block, 0, sizeof(ctx->block));
	ctx->count = 0;
	ctx->index = 0;
}

void
md5_update(struct md5_ctx *ctx, const uint8_t *data, size_t length) {
	if (ctx->index) {
		/* Fill partial block */
		unsigned left = sizeof(ctx->block) - ctx->index;
		if (length < left) {
			memcpy(ctx->block + ctx->index, data, length);
			ctx->index += length;
			return;		/* Cannot fill partial block, so return here */
		} else {
			memcpy(ctx->block + ctx->index, data, left);
			_md5_compress(ctx->state, ctx->block);
			ctx->count++;

			data += left;
			length -= left;
		}
	}
	/* Process individual block */
	while (length >= sizeof(ctx->block)) {
		_md5_compress(ctx->state, data);
		ctx->count++;

		data += sizeof(ctx->block);
		length -= sizeof(ctx->block);
	}
	/* Copy any leftover data into internal buffer */
	memcpy(ctx->block, data, length);
	ctx->index = length;
}

void
md5_digest(struct md5_ctx *ctx, uint8_t *digest, size_t length) {
	uint64_t bit_count;
	unsigned i;
	assert (length <= MD5_DIGEST_SIZE);

	/* Apply padding */
	i = ctx->index;
	ctx->block[i++] = 0x80;
	if (i > (sizeof(ctx->block) - 8)) {
		/* No room for length in this block. Process it and pad with another one */					\
		memset(ctx->block + i, 0, sizeof(ctx->block) - i);

		_md5_compress(ctx->state, ctx->block);
		i = 0;
	}
	memset(ctx->block + i, 0, sizeof(ctx->block) - 8 - i);

	/* There are 512 = 2^9 bits in one block */
	bit_count = (ctx->count << 9) | (ctx->index << 3);

	LE_WRITE_UINT64(ctx->block + (MD5_BLOCK_SIZE - 8), bit_count);
	_md5_compress(ctx->state, ctx->block);

	memwrite_le32(digest, ctx->state, length);
	md5_init(ctx);
}

