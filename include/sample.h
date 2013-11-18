#ifndef SAMPLE_H
#define SAMPLE_H

typedef uint64_t sample_time_t;
typedef uint32_t sample_value_t;

typedef struct {
	/* public declarations */
	sample_time_t time;
	sample_value_t value;
} sample_t;

#endif // SAMPLE_H
