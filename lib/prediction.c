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

static void prediction_metric_result_delete(prediction_metric_result_t *pmr)
{
	free((char *)pmr->name);
	graph_delete((graph_t *)pmr->high);
	graph_delete((graph_t *)pmr->average);
	graph_delete((graph_t *)pmr->low);
}

static prediction_output_t *prediction_output_create()
{
	prediction_output_t *po = malloc(sizeof(prediction_output_t));
	if (!po)
		return NULL;

	INIT_LIST_HEAD(&po->metrics);
	return po;
}

void prediction_output_delete(prediction_output_t *po)
{
	prediction_metric_result_t *pos, *n;

	/* free the metrics list */
	list_for_each_entry_safe(pos, n, &po->metrics, list) {
		list_del(&(pos->list));
		prediction_metric_result_delete(pos);
	}
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

prediction_output_t *prediction_exec(prediction_t *p)
{
	prediction_priv_t *p_priv = prediction_priv(p);

	int ret = p_priv->check(p);
	if (ret)
		return NULL;

	prediction_output_t *po = prediction_output_create();
	if (!po)
		return NULL;

	return p_priv->exec(p, po);
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
