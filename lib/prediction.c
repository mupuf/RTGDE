#include "prediction_priv.h"
#include <inttypes.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

prediction_metric_result_t *prediction_metric_result_create(const char *name)
{
	prediction_metric_result_t *pmr = malloc(sizeof(prediction_metric_result_t));
	if (!pmr)
		return NULL;

	INIT_LIST_HEAD(&pmr->list);
	pmr->name = strdup(name);
	pmr->high = graph_create();
	pmr->average = graph_create();
	pmr->low = graph_create();

	return pmr;
}

void prediction_metric_result_delete(prediction_metric_result_t *pmr)
{
	if (!pmr)
		return;

	if (pmr->name == (char *)42) {
		fprintf(stderr,
			"possible double free of prediction_metric_result_t %p\n",
			pmr);
	}

	free((char *)pmr->name);
	graph_delete((graph_t *)pmr->high);
	graph_delete((graph_t *)pmr->average);
	graph_delete((graph_t *)pmr->low);

	pmr->name = (char *)42; // POISON!

	free(pmr);
}

prediction_metric_result_t *prediction_metric_result_copy(prediction_metric_result_t *pmr)
{
	prediction_metric_result_t *new_pmr = malloc(sizeof(prediction_metric_result_t));
	if (!pmr)
		return NULL;

	INIT_LIST_HEAD(&new_pmr->list);
	new_pmr->name = strdup(pmr->name);
	new_pmr->high = graph_copy(pmr->high);
	new_pmr->average = graph_copy(pmr->average);
	new_pmr->low = graph_copy(pmr->low);

	return new_pmr;
}

prediction_list_t *prediction_list_create()
{
	prediction_list_t *pl = malloc(sizeof(prediction_list_t));
	if (!pl)
		return NULL;

	INIT_LIST_HEAD(pl);
	return pl;
}
void prediction_list_append(prediction_list_t *pl, prediction_metric_result_t *pmr)
{
	list_add_tail(&pmr->list, pl);
}

int prediction_list_append_list_copy(prediction_list_t *po, const prediction_list_t * npl)
{
	prediction_metric_result_t *pmr;

	list_for_each_entry(pmr, npl, list) {
		prediction_metric_result_t *npmr = prediction_metric_result_copy(pmr);
		if (!npmr)
			return 1;
		prediction_list_append(po, npmr);
	}
	return 0;
}

prediction_metric_result_t * prediction_list_find(prediction_list_t *pl, const char *metric_name)
{
	prediction_metric_result_t *pos, *n;

	list_for_each_entry_safe(pos, n, pl, list) {
		if (strcmp(pos->name, metric_name) == 0)
			return pos;
	}

	return NULL;
}

prediction_metric_result_t * prediction_list_extract_by_name(prediction_list_t *pl, const char *metric_name)
{
	prediction_metric_result_t *ret = prediction_list_find(pl, metric_name);
	if (ret)
		list_del(&(ret->list));
	return ret;
}

prediction_metric_result_t * prediction_list_extract_head(prediction_list_t *input)
{
	prediction_metric_result_t *ret = list_entry(input->next, prediction_metric_result_t, list);
	if (!list_empty(input)) {
		list_del(&(ret->list));
		return ret;
	} else
		return NULL;
}

void prediction_list_delete(prediction_list_t *pl)
{
	prediction_metric_result_t *pos, *n;

	if (!pl)
		return;

	/* free the metrics list */
	list_for_each_entry_safe(pos, n, pl, list) {
		list_del(&(pos->list));
		prediction_metric_result_delete(pos);
	}
	free(pl);
}

prediction_priv_t * prediction_priv(prediction_t* p)
{
	return (prediction_priv_t*) p;
}

uint32_t prediction_metrics_count(prediction_t* p)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	return p_priv->metrics_count;
}


prediction_t * prediction_create(prediction_check_t check,
				 prediction_exec_t exec,
				 prediction_delete_t dtor,
				 void *user)
{
	prediction_priv_t *p_priv = malloc(sizeof(prediction_priv_t));
	if (!p_priv)
		return NULL;

	INIT_LIST_HEAD(&p_priv->metrics);
	p_priv->metrics_count = 0;
	p_priv->check = check;
	p_priv->exec = exec;
	p_priv->dtor = dtor;
	p_priv->base.user = user;
	p_priv->csv_filename_format = NULL;
	p_priv->prediction_count = 0;

	return (prediction_t *)p_priv;
}

prediction_list_t *prediction_exec(prediction_t *p)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	prediction_metric_t *pos;

	int ret = p_priv->check(p);
	if (ret)
		return NULL;

	p_priv->prediction_count++;

	prediction_list_t *pl = p_priv->exec(p);
	if (!pl)
		return NULL;

	if (!p_priv->csv_filename_format)
		return pl;

	/* list all metrics */
	list_for_each_entry(pos, &p_priv->metrics, list) {
		prediction_metric_result_t *pred;
		sample_time_t last_sample_time;
		char filename[PATH_MAX];
		history_size_t mh_size;
		sample_t *bh;
		int i;

		snprintf(filename, sizeof(filename), p_priv->csv_filename_format,
			 metric_name(pos->base), p_priv->prediction_count);

		pred = prediction_list_find(pl, metric_name(pos->base));
		if (!pred) {
			fprintf(stderr,
				"Error, can't find '%s' in the prediction list\n",
				metric_name(pos->base));
		}

		printf("filename = '%s'\n", filename);
		FILE *f = fopen(filename, "w");
		if (!f) {
			perror("prediction csv output, fopen");
			continue;
		}

		fprintf(f, "Time, %s, %s-low prediction, %s-average prediction, %s-high prediction\n",
			metric_name(pos->base),
			metric_name(pos->base),
			metric_name(pos->base),
			metric_name(pos->base));

		/* dump the metrics */
		mh_size = metric_history_size(pos->base);
		bh = (sample_t *) malloc(mh_size * sizeof(sample_t));
		mh_size = metric_dump_history(pos->base, bh, mh_size);

		for (i = 0; i < mh_size; i++) {
			fprintf(f, "%" PRIu64 ", %u, 0, 0, 0\n", bh[i].time, bh[i].value);
			last_sample_time = bh[i].time;
		}

		free(bh);

		/* dump the predicted values */
		const sample_t *s_low = graph_read_first(pred->low);
		const sample_t *s_avg = graph_read_first(pred->average);
		const sample_t *s_high = graph_read_first(pred->high);

		while (s_low && s_avg && s_high) {
			fprintf(f, "%" PRIu64 ", %u, %u, %u\n",
				last_sample_time + s_low->time, s_low->value,
				s_avg->value, s_high->value);

			s_low = graph_read_next(pred->low, s_low);
			s_avg = graph_read_next(pred->average, s_avg);
			s_high = graph_read_next(pred->high, s_high);
		}

		fclose(f);
	}

	return pl;
}

void prediction_delete(prediction_t *p)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	prediction_metric_t *pos, *n;

	p_priv->dtor(p);

	/* free the metrics list */
	list_for_each_entry_safe(pos, n, &p_priv->metrics, list) {
		list_del(&(pos->list));
		p_priv->metrics_count--;
		free(pos);
	}

	if (p_priv->csv_filename_format)
		free(p_priv->csv_filename_format);

	/* POISON: detect use after free or double-delete */
	p_priv->csv_filename_format = (char *)42;

	free(p);
}

void prediction_output_csv(prediction_t *p, const char *csv_filename_format)
{
	prediction_priv_t *p_priv = prediction_priv(p);

	if (p_priv->csv_filename_format)
		free(p_priv->csv_filename_format);

	p_priv->csv_filename_format = strdup(csv_filename_format);
}

int prediction_attach_metric(prediction_t *p, metric_t *m)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	prediction_metric_t *pos;
	prediction_metric_t *pm;

	list_for_each_entry(pos, &p_priv->metrics, list) {
		if (strcmp(metric_name(pos->base), metric_name(m)) == 0) {
			fprintf(stderr,
				"prediction_attach_metric: Metric '%s' is already attached\n",
				metric_name(m));
			return 1;
		}
	}

	pm = malloc(sizeof(prediction_metric_t));
	if (!pm)
		return -1;

	INIT_LIST_HEAD(&pm->list);
	pm->base = m;

	list_add(&pm->list, &p_priv->metrics);
	p_priv->metrics_count++;

	return 0;
}

#ifdef __cplusplus
}
#endif
