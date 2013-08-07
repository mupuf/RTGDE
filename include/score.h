#ifndef SCORE_H
#define SCORE_H

typedef struct {

} score_metric_t;

typedef enum {
	SCORE_INTEGRAL_DIFFERENCE = 0
} score_type_t;

score_metric_t *
score_new_metric(const char *name, int weight, score_type_t type);

#endif // SCORE_H
