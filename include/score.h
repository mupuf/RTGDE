#ifndef SCORE_H
#define SCORE_H

typedef struct {

} score_metric_t;

typedef struct {
	const char *name;
} score_t;

score_metric_t * score_metric_create(score_t *s, int weight);

#endif // SCORE_H
