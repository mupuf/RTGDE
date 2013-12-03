#ifndef FSM_H
#define FSM_H

struct fsm {
	struct fsm_state *cur;
};

struct fsm_state {
	struct fsm_state (*func)(const struct fsm *fsm);
};

#endif // FSM_H
