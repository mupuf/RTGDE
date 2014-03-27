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

#ifndef SCORE_H
#define SCORE_H

#include "decision.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

} scoring_metric_t;

typedef struct {

} scoring_t;

typedef void (*scoring_output_csv_cb_t)(scoring_t *s, const char *name,
					  const char *csv_filename);

scoring_metric_t * scoring_metric_create(scoring_t *s, const char *name, int weight);
scoring_metric_t * scoring_metric_by_name(scoring_t *s, const char *name);
int scoring_metric_weight(scoring_metric_t *metric);
void scoring_metric_delete(scoring_t *s, scoring_metric_t *metric);

typedef float (*scoring_calc_t)(scoring_t *s, const prediction_metric_result_t *pmr,
				const graph_t *model_out);
typedef void (*scoring_dtor_t)(scoring_t *s);

scoring_t *scoring_create(scoring_calc_t calc, scoring_dtor_t dtor,
			  const char *name, void *user);
void *scoring_user(scoring_t *s);
const char *scoring_name(scoring_t *s);
int scoring_exec(scoring_t *s, decision_input_t *di);
void scoring_delete(scoring_t *s);

/** csv_filename_format have to have 2 parameters, the first is one is scoring's
 * name (%s) and the second one is the metrics' name (%s).
 */
void scoring_output_csv(scoring_t *s, const char *csv_filename_format,
			scoring_output_csv_cb_t output_cb);


#ifdef __cplusplus
}
#endif

#endif // SCORE_H
