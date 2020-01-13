/*
 * yarrow-keysource.h
 *
 *  Created on: Jan 9, 2020, 11:56:40 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef YARROW_KEYSOURCE_H_
#define YARROW_KEYSOURCE_H_

#include "yarrow.h"

#ifdef __cplusplus
extern "C" {
#endif

struct yarrow_keysource_ctx;

extern void yarrow_init_keysource(struct yarrow_keysource_ctx **);
extern void yarrow_close_keysource(struct yarrow_keysource_ctx *);

#ifdef __cplusplus
}
#endif

#endif /* YARROW_KEYSOURCE_H_ */
