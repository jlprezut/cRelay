/******************************************************************************
 * 
 * Relay card control utility: Driver for Sainsmart 16-channel USB CH340-G relay
 * control module:
 * 
 * Description:
 *   This 16-channel module is used for USB control of the Sainsmart 16-channel
 *   relays.
 *   https://www.sainsmart.com/collections/internet-of-things/products/16-channel-9-36v-usb-relay-module
 * 
 * Author:
 *   Jean-Louis PREZUT
 *
 *   Based on :
 *   * https://github.com/ldnelso2/sainsmart
 *   * https://github.com/obdev/v-usb/tree/master/examples/usbtool
 * 
 * Last modified:
 *   19/04/2020
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

#ifndef relay_drv_sainsmart16_CH340_h
#define relay_drv_sainsmart16_CH340_h

/**********************************************************
 * Function detect_relay_card_sainsmart_16chan_CH340()
 * 
 * Description: Detect the Saintsmart 16 channel relay card CH340
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 *             num_relays(out)- pointer to number of relays
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_relay_card_sainsmart_16chan_CH340(char* portname, uint8_t* num_relays, char* serial, relay_info_t** relay_info);


/**********************************************************
 * Function get_relay_sainsmart_16chan_CH340()
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
int get_relay_sainsmart_16chan_CH340(char* portname, uint8_t relay, relay_state_t* relay_state, char* serial);


/**********************************************************
 * Function set_relay_sainsmart_16chan_CH340()
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
int set_relay_sainsmart_16chan_CH340(char* portname, uint8_t relay, relay_state_t relay_state, char* serial);

int close_sainsmart_16chan_CH340() ;

int free_static_mem_sainsmart_16chan_CH340() ;

#endif
