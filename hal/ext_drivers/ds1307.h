//------------------------------------------------------------------------------

/*
 * Copyright 2025 Michal Lokcewicz
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//------------------------------------------------------------------------------

#ifndef DS1307_H_
#define DS1307_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

enum ds1307_rate_select
{
    DS1307_RS_1HZ = 0,
    DS1307_RS_4096HZ,
    DS1307_RS_8192HZ,
    DS1307_RS_32768HZ,
};

//------------------------------------------------------------------------------

typedef bool (*ds1307_io_init_cb)(void);
typedef bool (*ds1307_io_deinit_cb)(void);
typedef bool (*ds1307_serial_send_cb)(uint8_t device_addr, uint8_t *data, uint16_t len);
typedef bool (*ds1307_serial_receive_cb)(uint8_t device_addr, uint8_t *data, uint16_t len);

//------------------------------------------------------------------------------

struct ds1307_time 
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;
};

//------------------------------------------------------------------------------

struct ds1307_cfg
{
    ds1307_io_init_cb io_init;
    ds1307_serial_send_cb serial_send;
    ds1307_serial_receive_cb serial_receive;

    bool sqw_en;
    enum ds1307_rate_select rs;
};

struct ds1307_obj
{
    ds1307_io_init_cb io_init;
    ds1307_io_deinit_cb io_deinit;
    ds1307_serial_send_cb serial_send;
    ds1307_serial_receive_cb serial_receive;
};

//------------------------------------------------------------------------------

/// @brief Initializes DS1307 module accroding to given configuration @ref struct ds1307_cfg
/// @param obj given DS1307 object pointer
/// @param cfg given configuration structure pointer
/// @return true if initialized correctly, otherwiste false
bool ds1307_init(struct ds1307_obj *obj, struct ds1307_cfg *cfg);

/// @brief Chekcs if DS1307 is running or is halted (according to power down)
/// @param obj given DS1307 object pointer
/// @return true if is running 
bool ds1307_is_running(struct ds1307_obj *obj);

/// @brief Sets given unix time 
/// @param obj given DS1307 object pointer
/// @param unix_time unix timestamp 
/// @return true if set correctly, otherwiste false
bool ds1307_set_time(struct ds1307_obj *obj, struct ds1307_time *unix_time);

/// @brief Gets unix time
/// @param obj given DS1307 object pointer
/// @param unix_time unix timestamp 
/// @return true if got correctly, otherwiste false
bool ds1307_get_time(struct ds1307_obj *obj, struct ds1307_time *unix_time);

/// @brief Saves data under given memory address (starting from 0 to 55)
/// @param obj given DS1307 object pointer
/// @param addr memory address (0 to 55)
/// @param data data pointer
/// @param len data length
/// @return true if saved correctly, otherwiste false
bool ds1307_save_to_ram(struct ds1307_obj *obj, uint8_t addr, uint8_t *data, uint8_t len);

/// @brief Reads data from given memory address (starting from 0 to 55)
/// @param obj given DS1307 object pointer
/// @param addr memory address (0 to 55)
/// @param data data pointer
/// @param len data length
/// @return true if read correctly, otherwiste false
bool ds1307_read_from_ram(struct ds1307_obj *obj, uint8_t addr, uint8_t *data, uint8_t len);

/// @brief Deinitializes DS1307 module and reset object
/// @param obj given DS1307 object pointer
/// @return true if deinitialized correctly, otherwiste false
bool ds1307_deinit(struct ds1307_obj *obj);

//------------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif /* DS1307_H_ */

//------------------------------------------------------------------------------
