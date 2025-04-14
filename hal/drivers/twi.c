//------------------------------------------------------------------------------

/// @file twi.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "twi.h"

#include <string.h>

#include <util/twi.h>
#include <avr/interrupt.h>

//------------------------------------------------------------------------------

enum twi_state
{
    TWI_STATE_IDLE,
    TWI_STATE_START_GENERATED,
    TWI_STATE_ADDR_SENT,
};

struct twi_ctx
{
    bool irq_mode;
    volatile uint8_t addr;
    volatile uint8_t *data;
    volatile uint8_t data_size;
    volatile uint8_t data_idx;
    volatile bool read;
    volatile bool error;
    volatile enum twi_state state;
};

static struct twi_ctx ctx;

//------------------------------------------------------------------------------

static void wait_for_transfer_complete(void)
{
    while (!(TWCR & (1 << TWINT)));
}

static bool generate_start(void)
{
    /* Generate START condition */
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    wait_for_transfer_complete();

    if (TW_STATUS != TW_START) // START error
        return false;
    
    return true;
}

static bool send_slave_addr(uint8_t addr, bool read)
{
    TWDR = (addr & (read ? 0xFF : 0xFE));

    TWCR = (1 << TWINT) | (1 << TWEN);

    wait_for_transfer_complete();

    if (TW_STATUS != (read ? TW_MR_SLA_ACK : TW_MT_SLA_ACK)) // NACK error
        return false;   

    return true;
}

static void generate_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); 
    
    while (TWCR & (1 << TWSTO));
}

static bool handle_error(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); 

    ctx.error = true;
    ctx.state = TWI_STATE_IDLE;

    return false;
}
    
//------------------------------------------------------------------------------

bool twi_init(struct twi_cfg *cfg)
{
    if (!cfg || cfg->frequency == 0 || F_CPU < (16UL * cfg->frequency))
        return false;

    ctx.irq_mode = cfg->irq_mode;
    ctx.data = NULL;
    ctx.data_idx = 0;
    ctx.data_size = 0;
    ctx.read = false;
    ctx.state = TWI_STATE_IDLE;

    /* Enable internal pull-ups */
    if (cfg->pull_up_en)
        PORTC |= (1 << PC4) | (1 << PC5);

    /* Enable TWI and set ACK bit generation */
    TWCR = (1 << TWEN) | (1 << TWEA);

    /* Set bus speed - calculate bitrate and prescaler */
    uint8_t presc = 0;
    uint16_t bitrate = F_CPU / (2 * cfg->frequency) - 8;

    while (bitrate > 255)
    {
        presc++;
        bitrate >>= 2;
    };

    TWBR = bitrate;

    TWSR &= ~(1 << TWPS1) & ~(1 << TWPS1);
    TWSR |= (presc & 0b11);

    return true;
}

bool twi_send(uint8_t addr, uint8_t *data, uint8_t size, bool generate_stop_cond)
{
    if (!data || size == 0)
        return false;

    /* Handle interrupt mode */
    if (ctx.irq_mode)
    {
        ctx.addr = addr;
        ctx.data = data;
        ctx.data_size = size;
        ctx.data_idx = 0;
        ctx.read = false;
        ctx.error = false;

        /* Generate START condition */
        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE);

        return true;
    }

    /* Generate START condition */
    if (!generate_start())
        return false;

    /* Send slave address */
    if (!send_slave_addr(addr, false))
        return false;

    /* Send data */
    for (uint8_t i = 0; i < size; i++)
    {
        TWDR = data[i];

        if (TWCR & (1 << TWWC)) // Write collision error
            return false;

        TWCR = (1 << TWINT) | (1 << TWEN);
        
        wait_for_transfer_complete();

        if (TW_STATUS != TW_MT_DATA_ACK) // NoACK error
            return false;    
    }
    
    /* Generate STOP condition */
    if (generate_stop_cond)
        generate_stop();

    return true;
}

bool twi_receive(uint8_t addr, uint8_t *data, uint8_t size)
{
    if (!data || size == 0)
        return false;

    if (ctx.irq_mode)
    {
        ctx.addr = addr;
        ctx.data = data;
        ctx.data_size = size;
        ctx.data_idx = 0;
        ctx.read = true;
        ctx.error = false;

        /* Generate START condition */
        TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1 << TWIE);

        return true;
    }

    /* Generate START condition */
    if (!generate_start())
        return false;

    /* Send slave address */
    if (!send_slave_addr(addr, true))
        return false;          

    /* Receive data */
    for (uint8_t i = 0; i < size; i++)
    {
        if (i < size - 1)
        {
            TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN);
            
            wait_for_transfer_complete();

            if (TW_STATUS != TW_MR_DATA_ACK) // NoACK error
                return false;

        }
        else /* Last byte */
        {
            TWCR = (1 << TWINT) | (1 << TWEN);

            wait_for_transfer_complete();

            if (TW_STATUS != TW_MR_DATA_NACK) // NoNACK error
                return false;
        }

        data[i] = TWDR;
    }
    
    /* Generate STOP condition */
    generate_stop();

    return true;
}

bool twi_is_finished(bool *error)
{
    if (error)
        *error = ctx.error;

    return ctx.error || !(TWCR & (1 << TWSTO));
}

void twi_deinit(void)
{
    /* Disable TWI */
    TWCR &= ~(1 << TWEN);

    /* Clear interrupt flag */
    TWCR |= (1 << TWINT);
    
    /* Disable internal pull-ups */
    PORTC &= ~(1 << PC4) & ~(1 << PC5);

    memset(&ctx, 0x00, sizeof(ctx));
}

//------------------------------------------------------------------------------

ISR(TWI_vect)
{
    switch (ctx.state)
    {
    case TWI_STATE_IDLE:
        
        if (TW_STATUS != TW_START) // START error
        {
            handle_error();
            return;
        } 

        ctx.state = TWI_STATE_START_GENERATED;
        
        /* Send address */
        TWDR = (ctx.addr & (ctx.read ? 0xFF : 0xFE));

        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
        
        break;

    case TWI_STATE_START_GENERATED:

        if (TW_STATUS != (ctx.read ? TW_MR_SLA_ACK : TW_MT_SLA_ACK)) // NACK error
        {
            handle_error();
            return;
        } 

        ctx.state = TWI_STATE_ADDR_SENT;

        /* Send first byte */
        if (!ctx.read)
        {
            TWDR = ctx.data[ctx.data_idx++];

            if (TWCR & (1 << TWWC)) // Write collision error
            {
                handle_error();
                return;
            }

            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
        }
        else /* Read first byte */
        {
            if (ctx.data_idx < ctx.data_size - 1) /* Not last byte */
                TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            else /* Last byte */
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
        }

        break;

    case TWI_STATE_ADDR_SENT:

        if (ctx.read && (ctx.data_idx == (ctx.data_size - 1))) // NoNACK error while reading last byte
        {
            if (TW_STATUS != TW_MR_DATA_NACK)
            {
                handle_error();
                return;
            }
        } 
        else if (TW_STATUS != (ctx.read ? TW_MR_DATA_ACK : TW_MT_DATA_ACK)) // NoACK error
        {
            handle_error();
            return;
        } 

        /* Read */
        if (ctx.read) 
        {
            ctx.data[ctx.data_idx++] = TWDR;

            if (ctx.data_idx < ctx.data_size - 1) /* Not last byte */
                TWCR = (1 << TWEA) | (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
            else if (ctx.data_idx == (ctx.data_size - 1)) /* Last byte */
                TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);                
        }

        /* Send STOP if data ends */
        if (ctx.data_idx == ctx.data_size)
        {
            ctx.state = TWI_STATE_IDLE;
            TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
            
            return;
        }   

        /* Write */
        if (!ctx.read)
        {
            TWDR = ctx.data[ctx.data_idx++];

            if (TWCR & (1 << TWWC)) // Write collision error
            {
                handle_error();
                return;
            }

            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE);
        }

        break;

    default:
        break;
    }
}

//------------------------------------------------------------------------------
