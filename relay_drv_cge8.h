/******************************************************************************
 * 
 * Relay card control utility: Driver for CGE USB 8-relay card
 * 
 * Description:
 *   This software is used to control the CGE USB 8-relay card.
 *   This file contains the declaration of the specific functions.
 * 
 * Author:
 *   Jean-Louis PREZUT
 *
 * Last modified:
 *   10/07/2020
 *
 * Copyright 2020, Jean-Louis PREZUT 
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

#ifndef relay_drv_cge8_usb_h
#define relay_drv_cge8_usb_h

/**********************************************************
 * Function detect_relay_card_cge_usb_8chan()
 * 
 * Description: Detect the CGE USB 8 relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 *             num_relays(out)- pointer to number of relays
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_relay_card_cge_usb_8chan(char* portname, uint8_t* num_relays, char* serial, relay_info_t** relay_info);

/**********************************************************
 * Function get_relay_cge_usb_8chan()
 * 
 * Description: Get the current relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (out) - current relay state
 * 
 * Return:   0 - success
 *          -1 - fail
 *********************************************************/
int get_relay_cge_usb_8chan(char* portname, uint8_t relay, relay_state_t* relay_state, char* serial);

/**********************************************************
 * Function set_relay_cge_usb_8chan()
 * 
 * Description: Set new relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (in)  - current relay state
 * 
 * Return:   0 - success
 *          -1 - fail
 *********************************************************/
int set_relay_cge_usb_8chan(char* portname, uint8_t relay, relay_state_t relay_state, char* serial);

int close_cge_usb_8chan() ;

int free_static_mem_cge_usb_8chan() ;

#endif
