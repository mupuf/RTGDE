#ifndef FSM_H
#define FSM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sample.h"

typedef struct {
} fsm_t;

typedef struct {
	char *name;
} fsm_state_t;

typedef fsm_state_t *(*fsm_next_state_t)(fsm_t *fsm, const char *metric, sample_value_t value);
typedef void (*fsm_dtor_t)(fsm_t *fsm);

fsm_t *fsm_create(fsm_next_state_t next_state, fsm_dtor_t dtor, void *user);
fsm_state_t *fsm_add_state(fsm_t *fsm, char *name, void *user);
fsm_state_t *fsm_update_state(fsm_t *fsm, const char *metric, sample_value_t value);
void *fsm_get_user(fsm_t *fsm);
void *fsm_state_get_user(fsm_state_t *fsm_state);
void fsm_delete(fsm_t *fsm);

#ifdef __cplusplus
}
#endif

#endif // FSM_H
