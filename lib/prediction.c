#include "prediction_priv.h"
#include <string.h>

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
	free((char *)pmr->name);
	graph_delete((graph_t *)pmr->high);
	graph_delete((graph_t *)pmr->average);
	graph_delete((graph_t *)pmr->low);
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

void prediction_output_append(prediction_list_t *pl, prediction_metric_result_t *pmr)
{
	list_add(pl, &pmr->list);
}

int prediction_output_append_list_copy(prediction_list_t *po, const prediction_list_t * npl)
{
	prediction_metric_result_t *pmr;

	list_for_each_entry(pmr, npl, list) {
		prediction_metric_result_t *npmr = prediction_metric_result_copy(pmr);
		if (!npmr)
			return 1;
		prediction_output_append(po, npmr);
	}
	return 0;
}

void prediction_output_delete(prediction_list_t *pl)
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

prediction_t * prediction_create(prediction_check_t check,
				 prediction_exec_t exec,
				 prediction_delete_t dtor,
				 void *user)
{
	prediction_priv_t *p_priv = malloc(sizeof(prediction_priv_t));
	if (!p_priv)
		return NULL;

	INIT_LIST_HEAD(&p_priv->metrics);
	p_priv->check = check;
	p_priv->exec = exec;
	p_priv->dtor = dtor;
	p_priv->base.user = user;

	return (prediction_t *)p_priv;
}

prediction_list_t *prediction_exec(prediction_t *p)
{
	prediction_priv_t *p_priv = prediction_priv(p);

	int ret = p_priv->check(p);
	if (ret)
		return NULL;

	prediction_list_t *pl = prediction_list_create();
	if (!pl)
		return NULL;

	return p_priv->exec(p, pl);
}

void prediction_delete(prediction_t *p)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	prediction_metric_t *pos, *n;

	p_priv->dtor(p);

	/* free the metrics list */
	list_for_each_entry_safe(pos, n, &p_priv->metrics, list) {
		list_del(&(pos->list));
		free(pos);
	}

	free(p);
}

int prediction_attach_metric(prediction_t *p, metric_t *m)
{
	prediction_metric_t *pm = malloc(sizeof(prediction_metric_t));
	if (!pm)
		return -1;

	pm->base = m;

	prediction_priv_t *p_priv = prediction_priv(p);
	list_add_tail(&pm->list, &p_priv->metrics);

	return 0;
}
