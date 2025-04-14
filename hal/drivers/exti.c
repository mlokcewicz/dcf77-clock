//------------------------------------------------------------------------------

/// @file exti.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "exti.h"

#include <stddef.h>

#include <avr/interrupt.h>

//------------------------------------------------------------------------------

struct exti_context
{
    exti_cb int0_cb;
    exti_cb int1_cb;
    exti_cb pcint0_cb;
    exti_cb pcint1_cb;
    exti_cb pcint2_cb;
};

static struct exti_context ctx;

//------------------------------------------------------------------------------

bool exti_init(enum exti_id id, enum exti_trigger trigger, exti_cb cb)
{
    if (id > EXTI_ID_PCINT23)
        return false;

    if (id >= EXTI_ID_PCINT16)
    {
        ctx.pcint2_cb = cb;
    }
    else if (id >= EXTI_ID_PCINT8)
    {
        ctx.pcint1_cb = cb;
    }
    else if (id >= EXTI_ID_PCINT0)
    {
        ctx.pcint0_cb = cb;
    }
    else if (id == EXTI_ID_INT1)
    {
        ctx.int1_cb = cb;
        EICRA = (EICRA & ~(1 << ISC11) & ~(1 << ISC10)) | (trigger << ISC10);
    }
    else if (id == EXTI_ID_INT0)
    {
        ctx.int0_cb = cb;
        EICRA = (EICRA & ~(1 << ISC01) & ~(1 << ISC00)) | (trigger << ISC00);
    }

    return true;
}

void exti_enable(enum exti_id id, bool enable)
{
    if (id > EXTI_ID_PCINT23)
        return;

    if (id >= EXTI_ID_PCINT16)
    {
        id -= EXTI_ID_PCINT16;
        PCICR |= (enable << PCIE2);
        PCMSK2 = (PCMSK2 & ~(1 << id)) | (enable << id);
    }
    else if (id >= EXTI_ID_PCINT8)
    {
        id -= EXTI_ID_PCINT8;
        PCICR |= (enable << PCIE1);
        PCMSK1 = (PCMSK1 & ~(1 << id)) | (enable << id);
    }
    else if (id >= EXTI_ID_PCINT0)
    {
        id -= EXTI_ID_PCINT0;
        PCICR |= (enable << PCIE0);
        PCMSK0 = (PCMSK0 & ~(1 << id)) | (enable << id);
    }
    else
    {
        EIMSK = (EIMSK & ~(1 << id)) | (enable << id);
    }
}

bool exti_deinit(enum exti_id id)
{
    if (id > EXTI_ID_PCINT23)
        return false;

    if (id >= EXTI_ID_PCINT16)
    {
        id -= EXTI_ID_PCINT16;
        PCMSK2 &= ~(1 << id);
        ctx.pcint2_cb = NULL;
    }
    else if (id >= EXTI_ID_PCINT8)
    {
        id -= EXTI_ID_PCINT8;
        PCMSK1 &= ~(1 << id);
        ctx.pcint1_cb = NULL;
    }
    else if (id >= EXTI_ID_PCINT0)
    {
        id -= EXTI_ID_PCINT0;
        PCMSK0 &= ~(1 << id);
        ctx.pcint0_cb = NULL;
    }
    else if (id == EXTI_ID_INT1)
    {
        EIMSK &= ~(1 << id);
        EICRA &= ~(1 << ISC11) & ~(1 << ISC10);
        ctx.int1_cb = NULL;
    }
    else if (id == EXTI_ID_INT0)
    {
        EIMSK &= ~(1 << id);
        EICRA &= ~(1 << ISC01) & ~(1 << ISC00);
        ctx.int0_cb = NULL;
    }

    return true;
}

//------------------------------------------------------------------------------

ISR(INT0_vect)
{
    if (ctx.int0_cb)
        ctx.int0_cb();
}

ISR(INT1_vect)
{
    if (ctx.int1_cb)
        ctx.int1_cb();
}

ISR(PCINT0_vect) // PCINT[7:0]
{
    if (ctx.pcint0_cb)
        ctx.pcint0_cb();
}

ISR(PCINT1_vect) // PCINT[14:8]
{
    if (ctx.pcint1_cb)
        ctx.pcint1_cb();
}

ISR(PCINT2_vect) // PCINT[23:16]
{
    if (ctx.pcint2_cb)
        ctx.pcint2_cb();
}

//------------------------------------------------------------------------------
