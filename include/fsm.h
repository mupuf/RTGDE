#ifndef FSM_H
#define FSM_H

typedef struct {
} fsm_t;

typedef struct {
} fsm_state_t;

typedef struct fsm_state *(*next_state_t)(const fsm_t *fsm, void *user);

fsm_t *fsm_create(next_state_t next_state, void *user);
fsm_state_t *fsm_add_state(fsm_t *fsm, void *user);

#endif // FSM_H
