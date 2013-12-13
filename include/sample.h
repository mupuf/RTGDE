#ifndef SAMPLE_H
#define SAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t sample_time_t;
typedef uint32_t sample_value_t;

typedef struct {
	/* public declarations */
	sample_time_t time;
	sample_value_t value;
} sample_t;

#ifdef __cplusplus
}
#endif

#endif // SAMPLE_H
