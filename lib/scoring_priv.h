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
