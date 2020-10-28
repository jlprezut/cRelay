/******************************************************************************
 * 
 * Relay card control utility: Generic type definitions
 * 
 * Description:
 *   This software is used to controls different type of relays cards.
 *   There are 3 ways to control the relais:
 *    1. via command line
 *    2. via web interface using a browser
 *    3. via HTTP API using a client application
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Last modified:
 *   19/08/2015
 *
 * Copyright 2015, Ondrej Wisniewski 
 * 
 * This file is part of crelay.
 * 
 * crelay is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABI
LITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with crelay.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *****************************************************************************/ 
 
#ifndef data_types_h
#define data_types_h

#define MAX_SERIAL_LEN 32

/* Config data struct */
typedef struct
{
    /* [HTTP server] */
    const char*  server_iface;
    uint16_t server_port;
    const char* relay_label[16] ;
    uint8_t pulse_duration;
    
    /* [GPIO drv] */
    uint8_t gpio_num_relays;
    uint8_t gpio_active_value;
    uint8_t relay1_gpio_pin;
    uint8_t relay2_gpio_pin;
    uint8_t relay3_gpio_pin;
    uint8_t relay4_gpio_pin;
    uint8_t relay5_gpio_pin;
    uint8_t relay6_gpio_pin;
    uint8_t relay7_gpio_pin;
    uint8_t relay8_gpio_pin;
    
    /* [Sainsmart drv] */
    uint8_t sainsmart_num_relays;
    
    /* [Boards] */
    uint8_t number;
    
    /* [Board list] */
    struct card_info *card_list;
    
} config_t;

typedef enum
{
   SERIAL_FIXE=0,
   SERIAL_AUTO=1,
   SERIAL_FIRST=2
} serial_type_t;

typedef struct card_info
{
    uint8_t card_id;
    const char* serial;
    serial_type_t serial_type;
    uint8_t num_relays;
    const char* relay_label[16] ;
    const char* comment;
    uint8_t model ;
    struct card_info *next;
} 
card_info_t;

#endif
