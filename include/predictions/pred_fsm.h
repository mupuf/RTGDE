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

#ifndef PREDICTION_FSM_H
#define PREDICTION_FSM_H

#include "../prediction.h"
#include "../fsm.h"

typedef int (*prediction_fsm_metric_from_state_t)(fsm_state_t *state,
						  const char *metric_name,
						  sample_value_t *value);

prediction_t *prediction_fsm_create(fsm_t *fsm,
				    prediction_fsm_metric_from_state_t metric_state,
				    sample_time_t prediction_length,
				    sample_time_t transition_resolution_us);

/* metrics should be added before running the code */
int prediction_fsm_add_output_metric(prediction_t *pred_fsm, const char *metric_name);
void prediction_fsm_dump_probability_density(prediction_t *pred_fsm,
					    const char *filename_pattern);

#endif
