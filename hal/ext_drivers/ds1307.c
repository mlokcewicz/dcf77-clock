//------------------------------------------------------------------------------

/// @file ds1307.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "ds1307.h"

//------------------------------------------------------------------------------

#define DS1307_ADDR 0b11010000

//------------------------------------------------------------------------------

enum ds1307_reg_addr
{
    DS1307_REG_ADDR_SECONDS = 0x00,
    DS1307_REG_ADDR_MINUTES = 0x01,
    DS1307_REG_ADDR_HOURS = 0x02,
    DS1307_REG_ADDR_DAY = 0x03,
    DS1307_REG_ADDR_DATE = 0x04,
    DS1307_REG_ADDR_MONTH = 0x05,
    DS1307_REG_ADDR_YEAR = 0x06,
    DS1307_REG_ADDR_CONTROL = 0x07,
    DS1307_REG_ADDR_RAM_START = 0x08,
    DS1307_REG_ADDR_RAM_END = 0x3F,
};

//------------------------------------------------------------------------------

bool ds1307_init(struct ds1307_obj *obj, struct ds1307_cfg *cfg)
{
    if (!obj || !cfg || !cfg->serial_send || !cfg->serial_receive)
        return false;

    obj->io_init = cfg->io_init;
    obj->serial_send = cfg->serial_send;
    obj->serial_receive = cfg->serial_receive;

    /* Set Rate and SQW */
    uint8_t msg[] = {DS1307_REG_ADDR_CONTROL, (cfg->rs & 0x03) | (cfg->sqw_en << 4)};

    if (!obj->serial_send(DS1307_ADDR, msg, sizeof(msg)))
        return false;

    return true;
}

bool ds1307_is_running(struct ds1307_obj *obj)
{
    if (!obj)
        return false;

    /* Get Clock Halt bit */
    uint8_t msg[] = {DS1307_REG_ADDR_SECONDS};

    if (!obj->serial_send(DS1307_ADDR, msg, sizeof(msg)))
        return false;

    if (!obj->serial_receive(DS1307_ADDR, msg, sizeof(msg)))
        return false;

    return (msg[0] & (1 << 7)) == 0;
}

bool ds1307_set_date(struct ds1307_obj *obj, uint32_t unix_time)
{
    if (!obj)
        return false;

    return true;
}

bool ds1307_get_date(struct ds1307_obj *obj, uint32_t *unix_time)
{   
    if (!obj || !unix_time)
        return false;

    return true;
}

bool ds1307_save_to_ram(struct ds1307_obj *obj, uint8_t addr, uint8_t *data, uint8_t len)
{
    if (!obj)
        return false;

    return true;
}

bool ds1307_read_from_ram(struct ds1307_obj *obj, uint8_t addr, uint8_t *data, uint8_t len)
{
    if (!obj)
        return false;

    return true;
}

bool ds1307_deinit(struct ds1307_obj *obj)
{
    if (!obj)
        return false;
    
    return true;
}

//------------------------------------------------------------------------------
