#ifndef SCORE_SIMPLE_H
#define SCORE_SIMPLE_H

#include "../scoring.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	scoring_normal = 0,
	scoring_inverted = 1
} score_simple_style_t;

scoring_t * score_simple_create(score_simple_style_t inverted);

#ifdef __cplusplus
}
#endif

#endif
