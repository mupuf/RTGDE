#ifndef HISTORY_H
#define HISTORY_H

#include <stdint.h>

#include "sample.h"

typedef struct {

} history_t;

typedef uint16_t history_size_t;
typedef history_size_t history_index_t;

history_t * history_create(history_size_t size);
void history_free(history_t *h);

void history_push(sample_t *s);
void history_get_sample_at(history_index_t i);

#endif // HISTORY_H
