//------------------------------------------------------------------------------

/// @file usart.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "usart.h"

#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>

//------------------------------------------------------------------------------

#if USART_FIXED_BAUDRATE_DOUBLE_SPEED
#define USART_FIXED_BAUDRATE_MUL 8UL
#else
#define USART_FIXED_BAUDRATE_MUL 16UL
#endif

#define USART_UBRR_VALUE ((F_CPU / (USART_FIXED_BAUDRATE_MUL * USART_FIXED_BAUDRATE)) - 1)

//------------------------------------------------------------------------------

struct usart_context
{
    usart_udre_cb udre_cb;
    usart_rxc_cb rxc_cb;
    usart_txc_cb txc_cb;
};

#if USART_USE_IRQ
static struct usart_context ctx;
#endif

//------------------------------------------------------------------------------

bool usart_init(struct usart_cfg *cfg)
{
    if (!cfg || cfg->data_size == USART_DATASIZE_9_BIT)
        return false;

#if USART_USE_FIXED_BAUDRATE
    UBRR0H = (uint8_t)(USART_UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)USART_UBRR_VALUE;
#else
    /* Set baud rate - alternative formula from mirekk36: (F_CPU + cfg->baudrate * 8) / (16UL * cfg->baudrate) - 1 */
    uint16_t ubrr = F_CPU / ((cfg->double_speed ? 8 : 16) * cfg->baudrate) - 1;

    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
#endif

    /* Set mode */
    UCSR0C &= ~(1 << UMSEL01) & ~(1 << UMSEL00);
    UCSR0C |= cfg->mode << UMSEL00;

#if USART_USE_SYNC_MODE
    if (cfg->mode == USART_MODE_SYNC)
    {
        UCSR0C = (UCSR0C & ~(1 << UCPOL0)) | (!!cfg->async_pol_txd_changed_on_xck_falling_edge << UCPOL0);
        DDRB = (DDRB & ~(1 << PD4)) | !!cfg->async_master << PD4;

#if !USART_USE_FIXED_BAUDRATE
        ubrr = F_CPU / (2 * cfg->baudrate) - 1;

        UBRR0H = (uint8_t)(ubrr >> 8);
        UBRR0L = (uint8_t)ubrr;
#endif
    }
#endif

    /* Double speed */
#if USART_USE_FIXED_BAUDRATE
    UCSR0A = (UCSR0A & ~(1 << U2X0)) | (USART_FIXED_BAUDRATE_DOUBLE_SPEED << U2X0);
#else 
    UCSR0A = (UCSR0A & ~(1 << U2X0)) | (!!cfg->double_speed << U2X0);
#endif

    /* Set data size */
    UCSR0C = (UCSR0C & ~(1 << UCSZ01) & ~(1 << UCSZ00)) | ((cfg->data_size & 0b11) << UCSZ00);
    UCSR0B = (UCSR0B & ~(1 << UCSZ02)) | (((cfg->data_size & 0b100) >> 2) << UCSZ02);

    /* Set parity */
    UCSR0C = (UCSR0C & ~(1 << UPM01) & ~(1 << UPM00)) | (cfg->parity << UPM00);

    /* Set stop bits */
    UCSR0C = (UCSR0C & ~(1 << USBS0)) | (!!cfg->stop_bits << USBS0); 

#if USART_USE_IRQ
    /* Store interrupts callbacks */
    ctx.udre_cb = cfg->udre_cb;
    ctx.rxc_cb = cfg->rxc_cb;
    ctx.txc_cb = cfg->txc_cb;

    /* Enable TXC interrupt */
    if (cfg->txc_cb)
        UCSR0B |= 1 << TXCIE0;
 #endif

    /* Enable receiver and / or transmitter */
    UCSR0B = (UCSR0B & ~(1 << RXEN0) & ~(1 << TXEN0)) | ((!!(cfg->rx_tx & USART_RX_ENABLE) << RXEN0) | (!!(cfg->rx_tx & USART_TX_ENABLE) << TXEN0));

#if USART_USE_SPI_MODE
    /* Configure SPI mode */
    if (cfg->mode == USART_MODE_SPI)
    {
        UBRR0H = 0;
        UBRR0L = 0;

        DDRD |= (1 << PD4);

        UCSR0C &= ~(1 << UCPOL0) & ~(1 << UCPHA0) & ~(1 << UDORD0);
        UCSR0C |= (!!cfg->spi_cpol << UCPOL0) | (!!cfg->spi_cpha << UCPHA0) | (!!cfg->spi_lsb_first << UDORD0); 

        ubrr = F_CPU / ((cfg->double_speed ? 8 : 16) * cfg->baudrate) - 1;

        UBRR0H = (uint8_t)(ubrr >> 8);
        UBRR0L = (uint8_t)ubrr;
    }
#endif

    return true;
}

void usart_send(uint8_t * data, uint8_t len)
{
#if USART_USE_IRQ

    /* For interrupt transport model, enable UDRE interrupt only */
    if (ctx.udre_cb)
    {
        UCSR0B |= 1 << UDRIE0;
        return;
    }
#endif

    for (uint32_t i = 0; i < len; i++)
    {
        /* Wait for empty transmit buffer */
        while (!(UCSR0A & (1 << UDRE0)));

        /* Put data into buffer, sends the data */
        UDR0 = *data++;
    }
}

bool usart_receive(uint8_t *data, uint32_t len)
{
#if USART_USE_IRQ
    /* For interrupt transport model, enable RXC interrupt only */
    if (ctx.rxc_cb)
    {
        UCSR0B |= 1 << RXCIE0;
        return true;
    }
#endif

    for (uint32_t i = 0; i < len; i++)
    {
        /* Wait for empty receive complete flag */
        while (!(UCSR0A & (1 << RXC0)));

        /* Check for Frame / Overrun / Parity errors */
        if (UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))
            return false;

        /* Put data into buffer, sends the data */
        *data++ = UDR0;
    }

    return true;
}

void usart_print(char *str)
{
    usart_send((uint8_t*)str, strlen(str));
}

void usart_deinit(void)
{
    UCSR0A &= ~(1 << U2X0) & ~(1 << MPCM0);
    UCSR0B &= ~(1 << RXCIE0) & ~(1 << TXCIE0) & ~(1 << UDRIE0) & ~(1 << RXEN0) & ~(1 << TXEN0) & ~(1 << UCSZ02);
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
    UCSR0C &= ~(1 << UMSEL01) & ~(1 << UMSEL00) & ~(1 << UPM01) & ~(1 << UPM00) & ~(1 << USBS0) & ~(1 << UCPOL0);
    UBRR0H = 0;
    UBRR0L = 0;

#if USART_USE_IRQ
    memset(&ctx, 0x00, sizeof(ctx));
#endif
}

//------------------------------------------------------------------------------

#if USART_USE_IRQ
ISR(USART_TX_vect)
{ 
    if (ctx.txc_cb)
        ctx.txc_cb();
};

ISR(USART_RX_vect)
{
    volatile uint8_t data_received = UDR0;
    volatile bool receive = false;
    volatile bool error = (UCSR0A & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)));

    if (ctx.rxc_cb)
        receive = ctx.rxc_cb(data_received, error);

    if (!receive)
        UCSR0B &= ~(1 << RXCIE0);
};

ISR(USART_UDRE_vect)
{
    volatile uint8_t data_to_send = 0;
    volatile bool send = false;

    if (ctx.udre_cb)
        send = ctx.udre_cb(&data_to_send);

    if (send)
        UDR0 = data_to_send;
    else
        UCSR0B &= ~(1 << UDRIE0);
};
#endif

//------------------------------------------------------------------------------
