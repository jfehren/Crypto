/*
 * noop.c
 *
 *  Created on: Jan 6, 2020, 5:01:21 PM
 *      Author: Joshua Fehrenbach
 *
 * Code adapted from GMP 6.1.2's tune/noop.c, although it would be
 * difficult to claim plagiarism on functions that literally do nothing.
 * These functions are in a separate file to prevent compilers from
 * recognizing these functions as no-ops and optimizing calls to them out.
 */

#include "speed.h"

void noop(void) { }

void noop_1(int) { }

