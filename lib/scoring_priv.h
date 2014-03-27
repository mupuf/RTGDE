/*Copyright (c) 2014 Martin Peres
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SCORE_PRIV_H
#define SCORE_PRIV_H

#include "scoring.h"
#include "prediction.h"

struct csv_output_file_t {
	FILE *csv_file;
	char *filename;
};

typedef struct {
	scoring_t base;

	scoring_calc_t calc;
	scoring_dtor_t dtor;

	const char *name;
	void *user;

	char *csv_file_format;
	struct timeval csv_time_first;
	scoring_output_csv_cb_t csv_cb;
	struct csv_output_file_t csv_models;

	struct list_head metrics;
} scoring_priv_t;

typedef struct {
	scoring_metric_t base;

	/* private declarations */
	struct list_head list;
	char *name;
	int weight;

	struct csv_output_file_t csv;

} scoring_metric_priv_t;

#endif // SCORE_PRIV_H
