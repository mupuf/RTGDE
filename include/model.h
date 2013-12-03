#ifndef MODEL_H
#define MODEL_H

#include "prediction.h"
#include "decision.h"

typedef struct {

} model_t;

typedef decision_input_model_t *(*model_exec_t)(model_t *m, prediction_list_t *prediction);
typedef void (*model_delete_t)(model_t *m);

decision_input_model_t *model_exec(model_t *m, prediction_list_t * predictions);
void model_delete(model_t *m);

#endif // MODEL_H
