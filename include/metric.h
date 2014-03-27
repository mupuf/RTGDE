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

#ifndef METRIC_H
#define METRIC_H

#include <stdio.h>
#include "graph.h"
#include "history.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
} metric_t;

metric_t * metric_create(const char *name, const char *unit, history_size_t history_size);
const char *metric_name(metric_t* m);
const char *metric_unit(metric_t* m);
void metric_update(metric_t *m, sample_time_t timestamp, sample_value_t value);
history_size_t metric_dump_history(metric_t *m, sample_t *buffer, history_size_t size);
sample_t metric_get_last(metric_t *m); /* will return garbage if no sample was fed to it before */
history_size_t metric_history_size(metric_t *m);
int metric_is_empty(metric_t *m);
void metric_print_history(metric_t *m);
void metric_set_csv_output_file(metric_t *m, const char *time_unit, const char *unit, FILE *of);
void metric_delete(metric_t *m);

#ifdef __cplusplus
}
#endif

#endif // METRIC_H
