#ifndef MODEL_SIMPLE_RADIO_H
#define MODEL_SIMPLE_RADIO_H

#include <model.h>

model_t * model_simple_radio_create(const char *name, uint32_t bitrate,
				    float energy_rx_to_tx,
				    float energy_tx_to_rx,
				    uint32_t delay_rx_to_tx_us,
				    float delay_tx_to_rx_us,
				    float pwr_rx_idle, float pwr_tx);

#endif
