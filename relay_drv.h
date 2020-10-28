/******************************************************************************
 * 
 * Relay card control utility: Relay driver generic file
 * 
 * Description:
 *   This software is used to controls different type of relays cards.
 *   This file contains the declaration of the generic functions.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with crelay.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *****************************************************************************/ 
 
#ifndef relay_drv_h
#define relay_drv_h

/* Conrad 4 channel USB relay card */
#define CONRAD_4CHANNEL_USB_NAME       "Conrad USB 4-channel relay card"
#define CONRAD_4CHANNEL_USB_NUM_RELAYS 4

/* Sainsmart 4/8 channel USB relay card */
#define SAINSMART_USB_NAME             "Sainsmart USB 4/8-channel relay card"
#define SAINSMART_USB_NUM_RELAYS       8

/* HID API compatibe x channel relay card */
#define HID_API_RELAY_NAME             "HID API compatible relay card"
#define HID_API_NUM_RELAYS             8

/* Sainsmart HID API compatibe 16-channel relay card */
#define SAINSMART16_USB_NAME           "Sainsmart USB-HID 16-channel relay card"
#define SAINSMART16_USB_NUM_RELAYS     16

/* Sainsmart USB CH340G 16-channel relay card */
#define SAINSMART16_CH340_NAME         "Sainsmart USB CH340G 16-channel relay card"
#define SAINSMART16_CH340_NUM_RELAYS     16

/* Sainsmart USB CGE 8-channel relay card */
#define CGE8_USB_NAME                  "CGE USB 8-channel relay card"
#define CGE8_USB_NUM_RELAYS     8

/* Generic GPIO connected relay cards */
#define GENERIC_GPIO_NAME              "Generic GPIO relays"
#define GENERIC_GPIO_NUM_RELAYS        8


#define FIRST_RELAY    1
#define MAX_NUM_RELAYS 16
#define MAX_RELAY_CARD_NAME_LEN 40
#define MAX_COM_PORT_NAME_LEN 32
#define MAX_SERIAL_LEN 32


typedef enum
{
   NO_RELAY_TYPE=0,
   
   CONRAD_4CHANNEL_USB_RELAY_TYPE = 1, /* Conrad usb 4-channel relay card */
   SAINSMART_USB_RELAY_TYPE = 2,       /* Sainsmart usb 4/8-channel relay card */
   HID_API_RELAY_TYPE = 3,             /* HID API compatible relay card */
   SAINSMART16_USB_RELAY_TYPE = 4,     /* Sainsmart USB-HID relay card */
   SAINSMART16_CH340_RELAY_TYPE = 5,     /* Sainsmart USB-HID relay card */
   CGE8_USB_RELAY_TYPE = 6,              /* CGE USB 8-channel relay card */
   GENERIC_GPIO_RELAY_TYPE = 7,        /* Relays connected directly via GPIO pins */
   LAST_RELAY_TYPE = 8

} relay_type_t;

typedef enum 
{
   OFF=0,
   ON,
   PULSE,
   INVALID
}
relay_state_t;

typedef struct relay_info
{
   relay_type_t relay_type;
   uint8_t      num_relays;
   char         serial[MAX_SERIAL_LEN]; 
   struct relay_info *next;
} 
relay_info_t;

typedef struct
{
   int (*detect_relay_card_fun)(char*, uint8_t*, char*, relay_info_t **); /* function to detect the relay card */
   int (*get_relay_fun)(char*, uint8_t, relay_state_t*, char*); /* function to get the current relay state */
   int (*set_relay_fun)(char*, uint8_t, relay_state_t, char*);  /* function to set the new relay state */
   int (*close_fun)();  /* function to set the new relay state */
   int (*free_static_mem_fun)();  /* function to set the new relay state */
   char *card_name;                                           /* card name string */
}
relay_data_t;



/**********************************************************
 * Function crelay_detect_all_relay_cards()

 * 
 * Description: Detect all relay cards
 * 
 * Parameters: relay_info(out)- pointer to list of 
 *                              relays info struct
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int crelay_detect_all_relay_cards(relay_info_t** relay_info);

/**********************************************************
 * Function crelay_detect_relay_card()
 * 
 * Description: Detect the relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 *             num_relays(out)- pointer to number of relays
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int crelay_detect_relay_card(char* portname, uint8_t* num_relays, char* serial, relay_info_t** relay_info, relay_type_t model);

/**********************************************************
 * Function crelay_get_relay()
 * 
 * Description: Get the current relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (out) - current relay state
 * 
 * Return:   o - success
 *          -1 - fail
 *********************************************************/
int crelay_get_relay(char* portname, uint8_t relay, relay_state_t* relay_state, char* serial);

/**********************************************************
 * Function crelay_set_relay()
 * 
 * Description: Set new relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (in)  - current relay state
 * 
 * Return:   o - success
 *          -1 - fail
 *********************************************************/
int crelay_set_relay(char* portname, uint8_t relay, relay_state_t relay_state, char* serial);

/**********************************************************
 * Function crelay_get_relay_card_type()
 * 
 * Description: Get the detected relay type
 * 
 * Parameters: none
 * 
 * Return: relay type
 *********************************************************/
relay_type_t crelay_get_relay_card_type();

/**********************************************************
 * Function crelay_get_relay_card_name()
 * 
 * Description: Get the detected relay card name
 * 
 * Parameters: none
 * 
 * Return: relay card name
 *********************************************************/
int crelay_get_relay_card_name(relay_type_t rtype, char* card_name);

int crelay_close();

int crelay_free_static_mem() ;

#endif

