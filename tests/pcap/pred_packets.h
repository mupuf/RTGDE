#ifndef PRED_PACKETS_H
#define PRED_PACKETS_H

#include <prediction.h>

#ifdef __cplusplus
extern "C" {
#endif

enum pred_avr_confidence_t {
	pred_avr_68_confidence = 1,
	pred_avr_95_confidence = 2,
	pred_avr_99_confidence = 3
};

prediction_t * pred_packets_create(sample_time_t prediction_length,
				enum pred_avr_confidence_t confidence_factor);

#ifdef __cplusplus
}
#endif

#endif
