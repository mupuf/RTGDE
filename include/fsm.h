#ifndef FSM_H
#define FSM_H

typedef struct {
} fsm_t;

typedef struct {
} fsm_state_t;

typedef fsm_state_t *(*fsm_next_state_t)(const fsm_t *fsm, void *user);
typedef void (*fsm_dtor_t)(fsm_t *fsm);

fsm_t *fsm_create(fsm_next_state_t next_state, fsm_dtor_t dtor, void *user);
fsm_state_t *fsm_add_state(fsm_t *fsm, void *user);
fsm_state_t *fsm_update_state(fsm_t *fsm);
void *fsm_get_user(fsm_t *fsm);
void fsm_delete(fsm_t *fsm);

#endif // FSM_H
