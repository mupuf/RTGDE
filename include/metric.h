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

metric_t * metric_create(const char *name, history_size_t history_size);
const char *metric_name(metric_t* m);
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
