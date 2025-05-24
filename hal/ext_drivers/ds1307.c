//------------------------------------------------------------------------------

/// @file ds1307.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "ds1307.h"

#include <string.h>

//------------------------------------------------------------------------------

#define DS1307_ADDR 0b11010001

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

static uint8_t to_bcd(uint8_t val) 
{
    return ((val / 10) << 4) | (val % 10);
}

static uint8_t from_bcd(uint8_t bcd) 
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

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

bool ds1307_set_time(struct ds1307_obj *obj, struct ds1307_time *time)
{
    if (!obj || !time)
        return false;

    uint8_t msg[8];
    msg[0] = DS1307_REG_ADDR_SECONDS;
    msg[1] = to_bcd(time->seconds) & 0x7F; // Bit 7 = CH (Clock Halt)
    msg[2] = to_bcd(time->minutes);
    msg[3] = to_bcd(time->hours); // 24h mode
    msg[4] = to_bcd(time->day);
    msg[5] = to_bcd(time->date);
    msg[6] = to_bcd(time->month);
    msg[7] = to_bcd(time->year);

    if (!obj->serial_send(DS1307_ADDR, msg, sizeof(msg)))
        return false;

    return true;
}

bool ds1307_get_time(struct ds1307_obj *obj, struct ds1307_time *time)
{
    if (!obj || !time)
        return false;

    uint8_t reg = DS1307_REG_ADDR_SECONDS;
    uint8_t data[7];

    if (!obj->serial_send(DS1307_ADDR, &reg, 1))
        return false;

    if (!obj->serial_receive(DS1307_ADDR, data, sizeof(data)))
        return false;

    time->seconds = from_bcd(data[0] & 0x7F); // Bit 7 = CH
    time->minutes = from_bcd(data[1]);
    time->hours   = from_bcd(data[2] & 0x3F); // 24h mode
    time->day     = from_bcd(data[3]);
    time->date    = from_bcd(data[4]);
    time->month   = from_bcd(data[5]);
    time->year    = from_bcd(data[6]);

    return true;
}

bool ds1307_save_to_ram(struct ds1307_obj *obj, uint8_t addr, uint8_t *data, uint8_t len)
{
    if (!obj || DS1307_REG_ADDR_RAM_START + addr + len > DS1307_REG_ADDR_RAM_END)
        return false;

    uint8_t msg[len + 1];
    msg[0] = DS1307_REG_ADDR_RAM_START + addr;
    memcpy(&msg[1], data, len);

    if (!obj->serial_send(DS1307_ADDR, msg, sizeof(msg)))
        return false;

    return true;
}

bool ds1307_read_from_ram(struct ds1307_obj *obj, uint8_t addr, uint8_t *data, uint8_t len)
{
    if (!obj || DS1307_REG_ADDR_RAM_START + addr + len > DS1307_REG_ADDR_RAM_END)
        return false;

    uint8_t msg[] = {DS1307_REG_ADDR_RAM_START + addr};
    
    if (!obj->serial_send(DS1307_ADDR, msg, sizeof(msg)))
        return false;

    if (!obj->serial_receive(DS1307_ADDR, data, len))
        return false;

    return true;
}

bool ds1307_deinit(struct ds1307_obj *obj)
{
    if (!obj)
        return false;

    /* Set Clock Halt bit */
    uint8_t msg[] = {DS1307_REG_ADDR_SECONDS, 1 << 7};

    if (!obj->serial_send(DS1307_ADDR, msg, sizeof(msg)))
        return false;
    
    return true;
}

//------------------------------------------------------------------------------
