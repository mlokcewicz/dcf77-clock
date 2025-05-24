//------------------------------------------------------------------------------

/// @file usart.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef USART_H_
#define USART_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

#ifndef USART_USE_IRQ
#define USART_USE_IRQ 0
#endif

#ifndef USART_USE_SPI_MODE
#define USART_USE_SPI_MODE 0
#endif

#ifndef USART_USE_SYNC_MODE
#define USART_USE_SYNC_MODE 0
#endif

#ifndef USART_USE_FIXED_BAUDRATE
#define USART_USE_FIXED_BAUDRATE 0
#endif

#ifndef USART_FIXED_BAUDRATE_DOUBLE_SPEED 
#define USART_FIXED_BAUDRATE_DOUBLE_SPEED 0
#endif

#ifndef USART_FIXED_BAUDRATE 
#define USART_FIXED_BAUDRATE 9600
#endif


//------------------------------------------------------------------------------

typedef bool (*usart_udre_cb)(volatile uint8_t *data_to_send); // Return true to continue transmission
typedef bool (*usart_rxc_cb)(volatile uint8_t data_received, volatile bool error); // Return true to continue reception
typedef void (*usart_txc_cb)(void);

//------------------------------------------------------------------------------

enum usart_mode
{
    USART_MODE_ASYMC = 0b00,
    USART_MODE_SYNC = 0b01,
    USART_MODE_SPI = 0b11,
};

enum usart_rx_tx
{
    USART_TX_ENABLE = 1 << 0,
    USART_RX_ENABLE = 1 << 1,
};

enum usart_data_size
{
    USART_DATASIZE_5_BIT = 0b000,
    USART_DATASIZE_6_BIT = 0b001,
    USART_DATASIZE_7_BIT = 0b010,
    USART_DATASIZE_8_BIT = 0b011,
    USART_DATASIZE_9_BIT = 0b111, // Currently unsupported
};

enum usart_parity
{
    USART_PARITY_DISABLED = 0b00,
    USART_PARITY_EVEN = 0b10,
    USART_PARITY_ODD = 0b11,
};

enum usart_stop_bits
{
    USART_STOP_BITS_1 = 0,
    USART_STOP_BITS_2 = 1,
};

//------------------------------------------------------------------------------

struct usart_cfg
{
    enum usart_mode mode;
    enum usart_rx_tx rx_tx;
    enum usart_data_size data_size;
    enum usart_parity parity;
    enum usart_stop_bits stop_bits;

#if !USART_USE_FIXED_BAUDRETE
    uint32_t baudrate;
    bool double_speed;
#endif

#if USART_USE_SYNC_MODE
    bool async_master;
    bool async_pol_txd_changed_on_xck_falling_edge;
#endif

#if USART_USE_SPI_MODE
    bool spi_lsb_first;
    bool spi_cpol;
    bool spi_cpha;
#endif

#if USART_USE_IRQ
    usart_udre_cb udre_cb;
    usart_rxc_cb rxc_cb;
    usart_txc_cb txc_cb;
#endif
};

//------------------------------------------------------------------------------

/// @brief Initializes USART module according to given configuration
/// @note If RXC or TXC callbacks are provided, send and receive functions only enables suitable inerrupts
/// @param cfg configuration structure pointer @ref struct usart_cfg
/// @return true if initialized succesfully, otherwise false
bool usart_init(struct usart_cfg *cfg);

/// @brief Sends given data if polling mode is chosen
/// @note If TXC callback is provided, this function only enables TXC interrupt
/// @param data data to send pointer
/// @param len data to send length 
void usart_send(uint8_t *data, uint8_t len);

/// @brief Receives given amount of data and stores in buffer pointed by data
/// @param data pointer to input buffer
/// @param len data to receive length 
/// @return true if received successfully, false in case of parity / overrun / frame errors
bool usart_receive(uint8_t *data, uint32_t len);

/// @brief Sends given data if polling mode is chosen
/// @note If TXC callback is provided, this function only enables TXC interrupt
/// @param str - null-terminated string to send pointer
void usart_print(char *str);

/// @brief Disables USART module, resets registers to default state and resets internal context
void usart_deinit(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* USART_H_ */

//------------------------------------------------------------------------------
