#ifndef SCORE_H
#define SCORE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

} score_metric_t;

typedef struct {
	const char *name;
} score_t;

score_metric_t * score_metric_create(score_t *s, int weight);

#ifdef __cplusplus
}
#endif

#endif // SCORE_H
