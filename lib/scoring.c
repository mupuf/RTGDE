#include "scoring_priv.h"

#include <inttypes.h>
#include <string.h>

static scoring_metric_priv_t *score_metric_priv(scoring_metric_t *m)
{
	return (scoring_metric_priv_t *)m;
}

static scoring_priv_t *scoring_priv(scoring_t *s)
{
	return (scoring_priv_t *)s;
}

scoring_t *scoring_create(scoring_calc_t calc, scoring_dtor_t dtor, const char *name, void *user)
{
	scoring_priv_t *s_priv = malloc(sizeof(scoring_priv_t));
	if (!s_priv)
		return NULL;

	INIT_LIST_HEAD(&s_priv->metrics);
	s_priv->calc = calc;
	s_priv->dtor = dtor;
	s_priv->name = strdup(name);
	s_priv->user = user;
	s_priv->csv_file_format = NULL;
	s_priv->time_first.tv_sec = 0;
	s_priv->time_first.tv_usec = 0;
	s_priv->score_csv = NULL;

	return (scoring_t *)s_priv;
}

void scoring_delete(scoring_t *s)
{
	scoring_priv_t *s_priv = scoring_priv(s);
	scoring_metric_priv_t *pos, *n;
	if (s_priv->dtor)
		s_priv->dtor(s);

	list_for_each_entry_safe(pos, n, &s_priv->metrics, list)
		scoring_metric_delete((scoring_metric_t*)pos);

	free((char *)s_priv->name);
	free(s);
}

scoring_metric_t * scoring_metric_create(scoring_t *s, const char *name, int weight)
{
	scoring_priv_t *s_priv = scoring_priv(s);
	scoring_metric_priv_t *sm_priv;

	if (scoring_metric_by_name(s, name)) {
		fprintf(stderr,
			"score_metric_create: Metric '%s' is already defined",
			name);
		return NULL;
	}

	sm_priv = malloc(sizeof(scoring_metric_priv_t));
	if (!sm_priv)
		return NULL;

	INIT_LIST_HEAD(&sm_priv->list);
	sm_priv->name = strdup(name);
	sm_priv->weight = weight;

	list_add(&sm_priv->list, &s_priv->metrics);

	return (scoring_metric_t *)sm_priv;
}

scoring_metric_t *scoring_metric_by_name(scoring_t *s, const char *name)
{
	scoring_priv_t *s_priv = scoring_priv(s);
	scoring_metric_priv_t *pos;

	list_for_each_entry(pos, &s_priv->metrics, list) {
		if (strcmp(pos->name, name) == 0) {
			return (scoring_metric_t*) pos;
		}
	}
	return NULL;
}

int scoring_metric_weight(scoring_metric_t *metric)
{
	return score_metric_priv(metric)->weight;
}

void scoring_metric_delete(scoring_metric_t *metric)
{
	scoring_metric_priv_t *sm_priv = score_metric_priv(metric);
	list_del(&(sm_priv->list));
	free(sm_priv->name);
	free(sm_priv);
}

const char *scoring_name(scoring_t *s)
{
	return scoring_priv(s)->name;
}

void *scoring_user(scoring_t *s)
{
	return scoring_priv(s)->user;
}

static FILE *csv_write_header_file(scoring_priv_t *s_priv, decision_input_t *di,
				  const char *score_metric)
{
	decision_input_model_t *dim;
	char filename[PATH_MAX];
	FILE *f;

	snprintf(filename, sizeof(filename), s_priv->csv_file_format,
		 s_priv->name, score_metric);

	f = fopen(filename, "w");
	if (!f) {
		perror("scoring csv output, fopen");
		return NULL;
	}

	fprintf(f, "Time (µs)");
	dim = decision_input_model_get_first(di);
	while (dim) {
		fprintf(f, ", %s's score", model_name(dim->model));
		dim = decision_input_model_get_next(dim);
	}
	fprintf(f, "\n");
	fflush(f);

	return f;
}

static void csv_write_header(scoring_priv_t *s_priv, decision_input_t *di)
{
	scoring_metric_priv_t *pos;

	if (!s_priv->csv_file_format || s_priv->score_csv != NULL)
		return;

	s_priv->score_csv = csv_write_header_file(s_priv, di, "global");
	list_for_each_entry(pos, &s_priv->metrics, list) {
		pos->f_csv = csv_write_header_file(s_priv, di, pos->name);
	}
}

uint64_t time_diff(struct timeval a, struct timeval b)
{
	uint64_t a_us = a.tv_sec * 1000000 + a.tv_usec;
	uint64_t b_us = b.tv_sec * 1000000 + b.tv_usec;

	return b_us - a_us;
}

static void csv_write_metric(scoring_priv_t *s_priv, FILE *f,
				  decision_input_t *di, const char *score_metric)
{
	decision_input_model_t *dim;
	decision_input_metric_t *di_metric;

	fprintf(f, "%" PRIu64, time_diff(s_priv->time_first, di->tv));
	dim = decision_input_model_get_first(di);
	while (dim) {
		di_metric = decision_input_metric_from_name(dim, score_metric);
		fprintf(f, ", %f", di_metric->score);
		dim = decision_input_model_get_next(dim);
	}
	fprintf(f, "\n");
	fflush(f);
}

static void csv_write_global_score(scoring_priv_t *s_priv, decision_input_t *di)
{
	decision_input_model_t *dim;

	fprintf(s_priv->score_csv, "%" PRIu64, time_diff(s_priv->time_first, di->tv));
	dim = decision_input_model_get_first(di);
	while (dim) {
		fprintf(s_priv->score_csv, ", %f", dim->score);
		dim = decision_input_model_get_next(dim);
	}
	fprintf(s_priv->score_csv, "\n");
	fflush(s_priv->score_csv);
}

int scoring_exec(scoring_t *s, decision_input_t *di)
{
	scoring_priv_t *s_priv = scoring_priv(s);
	decision_input_model_t *dim;
	scoring_metric_priv_t *pos;
	decision_input_metric_t* m;

	/* first run? */
	csv_write_header(s_priv, di);
	if (s_priv->time_first.tv_sec == 0)
		s_priv->time_first = di->tv;

	/* for all models of dim */
	dim = decision_input_model_get_first(di);
	while (dim) {
		int weights = 0;
		/* for all metrics registered */
		list_for_each_entry(pos, &s_priv->metrics, list) {
			/* fetch the corresponding entry from the decision input */
			m = decision_input_metric_from_name(dim, pos->name);
			if (!m)
				continue;

			m->score = s_priv->calc(s, m->prediction, m->output);
			dim->score += m->score * pos->weight;

			weights += pos->weight;
		}
		dim->score = dim->score / weights;
		dim = decision_input_model_get_next(dim);
	}

	/* print all the metrics */
	if (s_priv->csv_file_format) {
		list_for_each_entry(pos, &s_priv->metrics, list) {
			csv_write_metric(s_priv, pos->f_csv, di, pos->name);
		}
		csv_write_global_score(s_priv, di);
	}

	return 0;
}

void scoring_output_csv(scoring_t *s, const char *csv_filename_format)
{
	scoring_priv_t *s_priv = scoring_priv(s);

	if (s_priv->csv_file_format)
		free(s_priv->csv_file_format);

	s_priv->csv_file_format = strdup(csv_filename_format);
}
