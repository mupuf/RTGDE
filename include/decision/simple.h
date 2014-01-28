#ifndef DECISION_SIMPLE_H
#define DECISION_SIMPLE_H

#include "../decision.h"

#ifdef __cplusplus
extern "C" {
#endif

decision_t * decision_simple_create(decision_callback_t user_cb, void *user_cb_data);

#ifdef __cplusplus
}
#endif

#endif

