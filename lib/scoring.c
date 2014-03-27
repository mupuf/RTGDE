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
	s_priv->csv_time_first.tv_sec = 0;
	s_priv->csv_time_first.tv_usec = 0;
	s_priv->csv_models.csv_file = NULL;
	s_priv->csv_models.filename = NULL;

	return (scoring_t *)s_priv;
}

static void scoring_csv_close(scoring_t *s, struct csv_output_file_t *cof, const char *name)
{
	scoring_priv_t *s_priv = scoring_priv(s);
	if (!cof->csv_file)
		return;

	/* close the file */
	fflush(cof->csv_file);
	fclose(cof->csv_file);

	/* call the user callback (TODO) */
	if (s_priv->csv_cb)
		s_priv->csv_cb(s, name, cof->filename);


	if (cof->filename)
		free(cof->filename);

}

void scoring_delete(scoring_t *s)
{
	scoring_priv_t *s_priv = scoring_priv(s);
	scoring_metric_priv_t *pos, *n;
	if (s_priv->dtor)
		s_priv->dtor(s);

	if (s_priv->csv_file_format)
		free(s_priv->csv_file_format);

	scoring_csv_close(s, &s_priv->csv_models, "global");
	list_for_each_entry_safe(pos, n, &s_priv->metrics, list) {
		scoring_metric_delete(s, (scoring_metric_t*)pos);
	}

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
	sm_priv->csv.csv_file = NULL;
	sm_priv->csv.filename = NULL;

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

void scoring_metric_delete(scoring_t *s, scoring_metric_t *metric)
{
	scoring_metric_priv_t *sm_priv = score_metric_priv(metric);

	scoring_csv_close(s, &sm_priv->csv, sm_priv->name);

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

static int csv_write_header_file(scoring_priv_t *s_priv, decision_input_t *di,
				 struct csv_output_file_t *cof,
				 const char *score_metric)
{
	decision_input_model_t *dim;

	cof->filename = malloc(PATH_MAX);
	if (!cof->filename)
		return 0;

	snprintf(cof->filename, PATH_MAX, s_priv->csv_file_format,
		 s_priv->name, score_metric);

	cof->csv_file = fopen(cof->filename, "w");
	if (!cof->csv_file) {
		perror("scoring csv output, fopen");
		return 0;
	}

	fprintf(cof->csv_file, "Time (µs)");
	dim = decision_input_model_get_first(di);
	while (dim) {
		fprintf(cof->csv_file, ", %s's score", model_name(dim->model));
		dim = decision_input_model_get_next(dim);
	}
	fprintf(cof->csv_file, "\n");
	fflush(cof->csv_file);

	return 1;
}

static void csv_write_header(scoring_priv_t *s_priv, decision_input_t *di)
{
	scoring_metric_priv_t *pos;

	if (!s_priv->csv_file_format || s_priv->csv_models.csv_file != NULL)
		return;

	csv_write_header_file(s_priv, di, &s_priv->csv_models, "global");
	list_for_each_entry(pos, &s_priv->metrics, list) {
		csv_write_header_file(s_priv, di, &pos->csv, pos->name);
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

	fprintf(f, "%" PRIu64, time_diff(s_priv->csv_time_first, di->tv));
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
	FILE *f = s_priv->csv_models.csv_file;

	fprintf(f, "%" PRIu64, time_diff(s_priv->csv_time_first, di->tv));
	dim = decision_input_model_get_first(di);
	while (dim) {
		fprintf(f, ", %f", dim->score);
		dim = decision_input_model_get_next(dim);
	}
	fprintf(f, "\n");
	fflush(f);
}

int scoring_exec(scoring_t *s, decision_input_t *di)
{
	scoring_priv_t *s_priv = scoring_priv(s);
	decision_input_model_t *dim;
	scoring_metric_priv_t *pos;
	decision_input_metric_t* m;

	/* first run? */
	csv_write_header(s_priv, di);
	if (s_priv->csv_time_first.tv_sec == 0)
		s_priv->csv_time_first = di->tv;

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
			csv_write_metric(s_priv, pos->csv.csv_file, di, pos->name);
		}
		csv_write_global_score(s_priv, di);
	}

	return 0;
}

void scoring_output_csv(scoring_t *s, const char *csv_filename_format,
			scoring_output_csv_cb_t output_cb)
{
	scoring_priv_t *s_priv = scoring_priv(s);

	if (s_priv->csv_file_format)
		free(s_priv->csv_file_format);

	s_priv->csv_file_format = strdup(csv_filename_format);
	s_priv->csv_cb = output_cb;
}
