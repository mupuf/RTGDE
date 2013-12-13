#ifndef MODEL_H
#define MODEL_H

#include "prediction.h"
#include "decision.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

} model_t;

typedef decision_input_model_t *(*model_exec_t)(model_t *m, prediction_list_t *prediction);
typedef void (*model_delete_t)(model_t *m);

decision_input_model_t *model_exec(model_t *m, prediction_list_t * predictions);
void model_delete(model_t *m);

#ifdef __cplusplus
}
#endif

#endif // MODEL_H
