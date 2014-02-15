#include "prediction_priv.h"
#include <inttypes.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *pmr_usage_hint_to_str(pmr_usage_hint_t hint)
{
	switch(hint)
	{
	case pmr_usage_prediction:
		return "prediction";
	case pmr_usage_constraint:
		return "constraint";
	}
	return "<not_a_valid_hint>";
}

prediction_metric_result_t *prediction_metric_result_create(const char *name,
						score_simple_style_t scoring_style)
{
	prediction_metric_result_t *pmr = malloc(sizeof(prediction_metric_result_t));
	if (!pmr)
		return NULL;

	INIT_LIST_HEAD(&pmr->list);
	pmr->name = strdup(name);
	pmr->scoring_style = scoring_style;
	pmr->usage_hint = pmr_usage_prediction;
	pmr->metric = NULL;
	pmr->hsize = 0;
	pmr->history = NULL;
	pmr->history_start = 0;
	pmr->history_stop = 0;
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
	if (pmr->history)
		free(pmr->history);
	graph_delete((graph_t *)pmr->high);
	graph_delete((graph_t *)pmr->average);
	graph_delete((graph_t *)pmr->low);

	pmr->name = (char *)42; // POISON!

	free(pmr);
}

prediction_metric_result_t *
prediction_metric_result_copy(prediction_metric_result_t *pmr)
{
	prediction_metric_result_t *new_pmr;
	new_pmr = malloc(sizeof(prediction_metric_result_t));
	if (!pmr)
		return NULL;

	INIT_LIST_HEAD(&new_pmr->list);
	new_pmr->name = strdup(pmr->name);
	new_pmr->scoring_style = pmr->scoring_style;
	new_pmr->hsize = pmr->hsize;
	if (pmr->history) {
		new_pmr->history = (sample_t *) malloc(pmr->hsize * sizeof(sample_t));
		memcpy(new_pmr->history, pmr->history, pmr->hsize * sizeof(sample_t));
	} else
		new_pmr->history = NULL;
	new_pmr->history_start = pmr->history_start;
	new_pmr->history_stop = pmr->history_stop;
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

void
prediction_list_append(prediction_list_t *pl, prediction_metric_result_t *pmr)
{
	list_add_tail(&pmr->list, pl);
}

int
prediction_list_append_list_copy(prediction_list_t *po, const prediction_list_t * npl)
{
	prediction_metric_result_t *pmr;

	list_for_each_entry(pmr, npl, list) {
		prediction_metric_result_t *npmr;

		npmr = prediction_metric_result_copy(pmr);
		if (!npmr)
			return 1;
		prediction_list_append(po, npmr);
	}
	return 0;
}

prediction_metric_result_t *
prediction_list_find(prediction_list_t *pl, const char *metric_name)
{
	prediction_metric_result_t *pos, *n;

	list_for_each_entry_safe(pos, n, pl, list) {
		if (strcmp(pos->name, metric_name) == 0)
			return pos;
	}

	return NULL;
}

prediction_metric_result_t * prediction_list_get_first(prediction_list_t *input)
{
	if (list_empty(input))
		return NULL;
	else
		return list_entry(input->next, prediction_metric_result_t, list);
}

prediction_metric_result_t * prediction_list_get_next(prediction_list_t *input,
						  prediction_metric_result_t *pmr)
{
	struct list_head * next = pmr->list.next;
	if (next == input)
		return NULL;
	else
		return list_entry(next, prediction_metric_result_t, list);
}

prediction_metric_result_t *
prediction_list_extract_by_name(prediction_list_t *pl, const char *metric_name)
{
	prediction_metric_result_t *ret = prediction_list_find(pl, metric_name);
	if (ret)
		list_del(&(ret->list));
	return ret;
}

prediction_metric_result_t * prediction_list_extract_head(prediction_list_t *input)
{
	prediction_metric_result_t *ret;
	ret = list_entry(input->next, prediction_metric_result_t, list);

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

static prediction_priv_t * prediction_priv(prediction_t* p)
{
	return (prediction_priv_t*) p;
}

uint32_t prediction_metrics_count(prediction_t* p)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	return p_priv->metrics_count;
}

struct list_head *prediction_metrics(prediction_t *p)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	return &p_priv->metrics;
}

prediction_t * prediction_create(prediction_check_t check,
				 prediction_exec_t exec,
				 prediction_delete_t dtor, const char *name,
				 void *user)
{
	prediction_priv_t *p_priv = malloc(sizeof(prediction_priv_t));
	if (!p_priv)
		return NULL;

	INIT_LIST_HEAD(&p_priv->metrics);
	p_priv->name = strdup(name);
	p_priv->metrics_count = 0;
	p_priv->check = check;
	p_priv->exec = exec;
	p_priv->dtor = dtor;
	p_priv->base.user = user;

	return (prediction_t *)p_priv;
}

const char *prediction_name(prediction_t* p)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	return p_priv->name;
}

prediction_list_t *prediction_exec(prediction_t *p)
{
	prediction_priv_t *p_priv = prediction_priv(p);

	int ret = p_priv->check(p);
	if (ret)
		return NULL;

	prediction_list_t *pl = p_priv->exec(p);
	if (!pl)
		return NULL;

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

	free(p_priv->name);
	free(p);

	/* POISON: detect use after free or double-delete */
	p_priv->name = (char *)42;
}

metric_t *prediction_find_metric(prediction_t *p, const char *name)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	prediction_metric_t *pos;

	list_for_each_entry(pos, &p_priv->metrics, list) {
		if (strcmp(metric_name(pos->base), name) == 0) {
			return pos->base;
		}
	}

	return NULL;
}

int prediction_attach_metric(prediction_t *p, metric_t *m)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	prediction_metric_t *pm;

	if (prediction_find_metric(p, metric_name(m)))
		return 1;

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
