//------------------------------------------------------------------------------

/// @file hal.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "hal.h"

#include <avr/fuse.h>
#include <avr/interrupt.h>

//------------------------------------------------------------------------------

/* Fuse and lock bits are active-low so to program given bit use & operator */

LOCKBITS = 0xFF;

FUSES =
{
    .low = LFUSE_DEFAULT,
    .high = HFUSE_DEFAULT,
    .extended = EFUSE_DEFAULT,
};

//------------------------------------------------------------------------------

/* Non implemented ISR handling */

ISR(BADISR_vect)
{
    /* Add error handling */
}

//------------------------------------------------------------------------------
