//------------------------------------------------------------------------------

/// @file twi.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef TWI_H_
#define TWI_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

struct twi_cfg
{
    bool pull_up_en;
    uint16_t frequency;
    bool irq_mode;
};

//------------------------------------------------------------------------------

/// @brief Initializes and enables TWI with given configuration
/// @param cfg configuration structure pointer
/// @return true if initialized successfully, otherwise false
bool twi_init(struct twi_cfg *cfg);

/// @brief Sends given amount of bytes under given address
/// @param addr device address (r/w bit is set inside function)
/// @param data bytes to be transmitted
/// @param size size of data to send
/// @param generate_stop generate stop if no following read operation is expected
/// @return true if sent successfully, otherwise false
bool twi_send(uint8_t addr, uint8_t *data, uint8_t size, bool generate_stop_cond);

/// @brief Receives given amount of bytes under given address
/// @param addr device address (r/w bit is set inside function)
/// @param data bytes to be received
/// @param size size of data to received
/// @return true if sent successfully, otherwise false
bool twi_receive(uint8_t addr, uint8_t *data, uint8_t size);

/// @brief Checks if transmission is finished (for interrupt mode)
/// @param error true if error occurred
/// @return true if transmission is finished or error occurred
bool twi_is_finished(bool *error);

/// @brief Deinitializes and disables TWI  
void twi_deinit(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* TWI_H_ */

//------------------------------------------------------------------------------
