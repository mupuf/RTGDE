#ifndef RTGDE_H
#define RTGDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "metric.h"
#include "prediction.h"
#include "model.h"
#include "scoring.h"
#include "decision.h"

typedef struct {

} flowgraph_t;

typedef void (*flowgraph_callback_t)(flowgraph_t *f, decision_input_t *di,
				     decision_input_model_t *dim, void *user);

typedef void (*flowgraph_prediction_output_csv_cb_t)(flowgraph_t *f,
					  prediction_t *p,
					  prediction_metric_result_t *pmr,
					  const char *csv_filename);

typedef void (*flowgraph_model_output_csv_cb_t)(flowgraph_t *f,
					  decision_input_metric_t* m,
					  const char *csv_filename);

int64_t clock_read_us();

flowgraph_t *flowgraph_create(const char *name, scoring_t *s, decision_t *d,
			      flowgraph_callback_t user_cb, void *user_cb_data,
			      uint64_t update_period_ns);
void flowgraph_teardown(flowgraph_t *f);

const char * flowgraph_name(flowgraph_t *f);

int flowgraph_attach_prediction(flowgraph_t *f, prediction_t * p);
int flowgraph_detach_prediction(flowgraph_t *f, prediction_t * p);

int flowgraph_attach_model(flowgraph_t *f, model_t * m);
int flowgraph_detach_model(flowgraph_t *f, model_t * m);

/* csv_filename_format have to have 3 parameters, the first is one is the number
 * of time a prediction has been made (%i), then the prediction's name (%s) and
 * finally the metrics' name (%s).
 */
void flowgraph_prediction_output_csv(flowgraph_t *f, const char *csv_filename_format,
			  flowgraph_prediction_output_csv_cb_t output_cb);

/* csv_filename_format have to have 3 parameters, the first is one is the number
 * of time the model has been executed (%i), then the model's name (%s) and
 * finally the metrics' name (%s).
 */
void flowgraph_model_output_csv(flowgraph_t *f, const char *csv_filename_format,
			  flowgraph_model_output_csv_cb_t output_cb);

int rtgde_start(flowgraph_t *f, int one_time);
int rtgde_stop(flowgraph_t *f);

#ifdef __cplusplus
}
#endif

#endif // RTGDE_H
