//------------------------------------------------------------------------------

/// @file timer.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "timer.h"

#include <stddef.h>

#include <util/atomic.h>

#include <avr/io.h>
#include <avr/interrupt.h>

//------------------------------------------------------------------------------

struct timer_ctx
{
    struct timer_obj *timer_obj[TIMER_ID_2 + 1];
};

static struct timer_ctx ctx;

//------------------------------------------------------------------------------

/* Code redundancy here is intentional - any optimizations by creating arrays of pointers to registers
   and selection of register by timer ID causes unnecessary FLASH consumption */

static void timer0_init(struct timer_obj *obj, struct timer_cfg *cfg)
{
    /* Set mode */
    TCCR0A &= ~(1 << WGM01) & ~(1 << WGM00);
    TCCR0B &= ~(1 << WGM02);
    TCCR0A |= (cfg->mode & 0b11) << WGM00;
    TCCR0B |= !!(cfg->mode & 0b100) << WGM02;

    /* Set output pin behaviour */
    TCCR0A &= ~(1 << COM0A1) & ~(1 << COM0A0) & ~(1 << COM0B1) & ~(1 << COM0B0);
    TCCR0A |= cfg->com_a_cfg << COM0A0;
    TCCR0A |= cfg->com_b_cfg << COM0B0;

    /* Configure ouput pins */
    if (cfg->com_a_cfg != TIMER_CM_DISABLED)
        DDRD |= (1 << PD6);

    if (cfg->com_b_cfg != TIMER_CM_DISABLED)
        PORTD &= ~(1 << PD5);

    /* Configure interrupts handling */
    if (cfg->ovrfv_cb) // Overflow
    {
        obj->ovrfv_cb = cfg->ovrfv_cb;
        TIMSK0 |= (1 << TOIE0);
    }

    if (cfg->out_comp_a_cb) // Compare
    {
        obj->out_comp_a_cb = cfg->out_comp_a_cb;
        TIMSK0 |= 1 << OCIE0A;
    }

    if (cfg->out_comp_b_cb) // Compare
    {
        obj->comp_b_cb = cfg->out_comp_b_cb;
        TIMSK0 |= 1 << OCIE0B;
    }

    /* Set counter value */
    TCNT0 = cfg->counter_val;

    /* Set compare values */
    OCR0A = cfg->out_comp_a_val;
    OCR0B = cfg->out_comp_b_val;
}

static void timer1_init(struct timer_obj *obj, struct timer_cfg *cfg)
{
    /* Set mode */
    enum timer_mode mode = (cfg->mode >> 4) - 1;

    TCCR1A &= ~(1 << WGM11) & ~(1 << WGM10);
    TCCR1B &= ~(1 << WGM12) & ~(1 << WGM13);
    TCCR1A |= (mode & 0b11) << WGM10;
    TCCR1B |= (mode & 0b1100) << (WGM12 - 2);

    /* Set output pin behaviour */
    TCCR1A &= ~(1 << COM1A1) & ~(1 << COM1A0) & ~(1 << COM1B1) & ~(1 << COM1B0);
    TCCR1A |= cfg->com_a_cfg << COM1A0;
    TCCR1A |= cfg->com_b_cfg << COM1B0;

    /* Configure ouput pins */
    if (cfg->com_a_cfg != TIMER_CM_DISABLED)
        DDRB |= (1 << PB1);

    if (cfg->com_b_cfg != TIMER_CM_DISABLED)
        DDRB |= (1 << PB2);

    /* Configure interrupts handling */
    if (cfg->ovrfv_cb) // Overflow
    {
        obj->ovrfv_cb = cfg->ovrfv_cb;
        TIMSK1 |= (1 << TOIE1);
    }

    if (cfg->out_comp_a_cb) // Compare
    {
        obj->out_comp_a_cb = cfg->out_comp_a_cb;
        TIMSK1 |= 1 << OCIE1A;
    }

    if (cfg->out_comp_b_cb) // Compare
    {
        obj->comp_b_cb = cfg->out_comp_b_cb;
        TIMSK1 |= 1 << OCIE1B;
    }

    if (cfg->in_capt_cb) // Capture
    {
        obj->in_capt_cb = cfg->in_capt_cb;
        TIMSK1 |= 1 << ICIE1;

        /* Configure ICP1 pin */
        DDRB &= ~(1 << PB0);
        PORTB = (PORTB & ~(1 << PB0)) | (cfg->input_capture_pullup << PB0);

    }

    /* Input capture options (only Timer1) */
    TCCR1B &= ~(1 << ICNC1) & ~(1 << ICES1);
    TCCR1B |= (cfg->input_capture_noise_canceler << ICNC1) | (cfg->input_capture_rising_edge << ICES1);

    /* Set counter value */
    TCNT1 = cfg->counter_val;

    /* Set compare values */
    OCR1A = cfg->out_comp_a_val;
    OCR1B = cfg->out_comp_b_val;

    /* Set input capture val */
    ICR1 = cfg->input_capture_val;
}

static void timer2_init(struct timer_obj *obj, struct timer_cfg *cfg)
{
    /* Configure external clock */
    ASSR &= ~(1 << EXCLK) & ~(1 << AS2);
    ASSR |= (cfg->async_clock << AS2);
    
    /* Set mode */
    TCCR2A &= ~(1 << WGM21) & ~(1 << WGM20);
    TCCR2B &= ~(1 << WGM22);
    TCCR2A |= (cfg->mode & 0b11) << WGM20;
    TCCR2B |= !!(cfg->mode & 0b100) << WGM22;

    /* Set output pin behaviour */
    TCCR2A &= ~(1 << COM2A1) & ~(1 << COM2A0) & ~(1 << COM2B1) & ~(1 << COM2B0);
    TCCR2A |= cfg->com_a_cfg << COM2A0;
    TCCR2A |= cfg->com_b_cfg << COM2B0;

    /* Configure ouput pins */
    if (cfg->com_a_cfg != TIMER_CM_DISABLED)
        DDRB |= (1 << PB3);

    if (cfg->com_b_cfg != TIMER_CM_DISABLED)
        DDRD |= (1 << PD3);

    /* Configure interrupts handling */
    if (cfg->ovrfv_cb) // Overflow
    {
        obj->ovrfv_cb = cfg->ovrfv_cb;
        TIMSK2 |= (1 << TOIE2);
    }

    if (cfg->out_comp_a_cb) // Compare
    {
        obj->out_comp_a_cb = cfg->out_comp_a_cb;
        TIMSK2 |= 1 << OCIE2A;
    }

    if (cfg->out_comp_b_cb) // Compare
    {
        obj->comp_b_cb = cfg->out_comp_b_cb;
        TIMSK2 |= 1 << OCIE2B;
    }

    /* Set counter value */
    TCNT2 = cfg->counter_val;

    /* Set compare values */
    OCR2A = cfg->out_comp_a_val;
    OCR2B = cfg->out_comp_b_val;
}

static void timer0_start(struct timer_obj *obj, bool start)
{
    TCCR0B &= ~(1 << CS02) & ~(1 << CS01) & ~(1 << CS00);

    if (start)
        TCCR0B |= obj->clock;
}

static void timer1_start(struct timer_obj *obj, bool start)
{
    TCCR1B &= ~(1 << CS12) & ~(1 << CS11) & ~(1 << CS10);

    if (start)
        TCCR1B |= obj->clock;
}

static void timer2_start(struct timer_obj *obj, bool start)
{
    TCCR2B &= ~(1 << CS22) & ~(1 << CS21) & ~(1 << CS20);

    if (start)
        TCCR2B |= obj->clock;
}

static void timer0_deinit(void)
{
    TCCR0A = 0;
    TCCR0B = 0;
    TIMSK0 = 0;
    TCNT0 = 0;
    OCR0A = 0;
    OCR0B = 0;
}

static void timer1_deinit(void)
{
    TCCR1A = 0;
    TCCR1B = 0;
    TIMSK1 = 0;
    TCNT1 = 0;
    OCR1A = 0;
    OCR1B = 0;
    ICR1 = 0;
}

static void timer2_deinit(void)
{
    TCCR2A = 0;
    TCCR2B = 0;
    TIMSK2 = 0;
    TCNT2 = 0;
    OCR2A = 0;
    OCR2B = 0;
}

//------------------------------------------------------------------------------

bool timer_init(struct timer_obj *obj, struct timer_cfg *cfg)
{
    if (!obj || !cfg || cfg->id > TIMER_ID_2)
        return false;

    /* Configure source clock */
    obj->clock = cfg->clock;

    if (cfg->id == TIMER_ID_0)
        timer0_init(obj, cfg);
    else if (cfg->id == TIMER_ID_1)
        timer1_init(obj, cfg);
    else if (cfg->id == TIMER_ID_2)
        timer2_init(obj, cfg);

    /* Store pointer to object for interrupts handling */
    ctx.timer_obj[cfg->id] = obj;
    obj->id = cfg->id;

    return true;
}

void timer_start(struct timer_obj *obj, bool start)
{
    if (obj->id == TIMER_ID_0)
        timer0_start(obj, start);
    else if (obj->id == TIMER_ID_1)
        timer1_start(obj, start);
    else if (obj->id == TIMER_ID_2)
        timer2_start(obj, start);
}

uint16_t timer_get_val(struct timer_obj *obj)
{
    volatile uint16_t val;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        /* Get counter value */
        if (obj->id == TIMER_ID_0)
            val = TCNT0;
        else if (obj->id == TIMER_ID_1)
            val = TCNT1;
        else if (obj->id == TIMER_ID_2)
            val = TCNT2;
    };

    return val;
}

void timer_deinit(struct timer_obj *obj)
{
    if (obj->id == TIMER_ID_0)
        timer0_deinit();
    else if (obj->id == TIMER_ID_1)
        timer1_deinit();
    else if (obj->id == TIMER_ID_2)
        timer2_deinit();

    ctx.timer_obj[obj->id] = NULL;
}

//------------------------------------------------------------------------------

ISR(TIMER0_OVF_vect)
{
    if (ctx.timer_obj[TIMER_ID_0]->ovrfv_cb)
    {
        ctx.timer_obj[TIMER_ID_0]->ovrfv_cb();
    }
}

ISR(TIMER0_COMPA_vect)
{
    if (ctx.timer_obj[TIMER_ID_0]->out_comp_a_cb)
    {
        ctx.timer_obj[TIMER_ID_0]->out_comp_a_cb();
    }
}

ISR(TIMER0_COMPB_vect)
{
    if (ctx.timer_obj[TIMER_ID_0]->comp_b_cb)
    {
        ctx.timer_obj[TIMER_ID_0]->comp_b_cb();
    }
}

//------------------------------------------------------------------------------

ISR(TIMER1_OVF_vect)
{
    if (ctx.timer_obj[TIMER_ID_1]->ovrfv_cb)
    {
        ctx.timer_obj[TIMER_ID_1]->ovrfv_cb();
    }
}

ISR(TIMER1_COMPA_vect)
{
    if (ctx.timer_obj[TIMER_ID_1]->out_comp_a_cb)
    {
        ctx.timer_obj[TIMER_ID_1]->out_comp_a_cb();
    }
}

ISR(TIMER1_COMPB_vect)
{
    if (ctx.timer_obj[TIMER_ID_1]->comp_b_cb)
    {
        ctx.timer_obj[TIMER_ID_1]->comp_b_cb();
    }
}

ISR(TIMER1_CAPT_vect)
{
    if (ctx.timer_obj[TIMER_ID_1]->in_capt_cb)
    {
        ctx.timer_obj[TIMER_ID_1]->in_capt_cb(ICR1);
    }
}

//------------------------------------------------------------------------------

ISR(TIMER2_OVF_vect)
{
    if (ctx.timer_obj[TIMER_ID_2]->ovrfv_cb)
    {
        ctx.timer_obj[TIMER_ID_2]->ovrfv_cb();
    }
}

ISR(TIMER2_COMPA_vect)
{
    if (ctx.timer_obj[TIMER_ID_2]->out_comp_a_cb)
    {
        ctx.timer_obj[TIMER_ID_2]->out_comp_a_cb();
    }
}

ISR(TIMER2_COMPB_vect)
{
    if (ctx.timer_obj[TIMER_ID_2]->comp_b_cb)
    {
        ctx.timer_obj[TIMER_ID_2]->comp_b_cb();
    }
}

//------------------------------------------------------------------------------

struct system_timer_ctx
{
    volatile uint16_t current_ms;
};

static struct system_timer_ctx system_timer_ctx;

static void timer0_comp_a_cb(void)
{
    system_timer_ctx.current_ms++; 
}

//------------------------------------------------------------------------------

void system_timer_init(void)
{
    system_timer_ctx.current_ms = 0;

    static struct timer_cfg timer0_cfg = 
    {  
        .id = TIMER_ID_0,
        .clock = TIMER_CLOCK_PRESC_8,
        .async_clock = TIMER_ASYNC_CLOCK_DISABLED,
        .mode = TIMER_MODE_CTC,
        .com_a_cfg = TIMER_CM_DISABLED,
        .com_b_cfg = TIMER_CM_DISABLED,

        .counter_val = 0,
        .ovrfv_cb = NULL,

        .out_comp_a_val = MS_TO_TICKS(1, 8) - 1,
        .out_comp_b_val = 0,
        .out_comp_a_cb = timer0_comp_a_cb,
        .out_comp_b_cb = NULL,
        
        .input_capture_val = 0,
        .input_capture_pullup = false,
        .input_capture_noise_canceler = false,
        .input_capture_rising_edge = false,
        .in_capt_cb = NULL,
    };

    static struct timer_obj timer0_obj;

    timer_init(&timer0_obj, &timer0_cfg);
    timer_start(&timer0_obj, true);
}

uint32_t system_timer_get(void)
{
    volatile uint32_t val = 0;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        val = system_timer_ctx.current_ms;
    }

    return val;
}

bool system_timer_timeout_passed(uint16_t tickstamp, uint16_t timeout)
{
    return tickstamp + timeout < system_timer_get();
}

//------------------------------------------------------------------------------
