/******************************************************************************
 * 
 * Relay card control utility: Driver for <XYZ> relay card
 *   <Link to relay cards product web page>
 * 
 * Description:
 *   <Short description of what is implemented in this file>
 * 
 * Build instructions:
 *   gcc -c relay_drv_sample.c
 * 
 * Last modified:
 *   <date>
 * 
 * Copyright 2015, <your name>
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

/******************************************************************************
 * Communication protocol description
 * ==================================
 * 
 * < Describe here how the communication between the host and the card
 *   works, how the relay states are read and how the relays are set >
 * 
 *****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "relay_drv.h"


/**********************************************************
 * Function detect_relay_card_sample()
 * 
 * Description: Detect the port used for communicating 
 *              with the sample relay card
 * 
 * Parameters: portname (out) - pointer to a string where
 *                              the detected com port will
 *                              be stored
 *             num_relays(out)- pointer to number of relays
 * 
 * Return:  0 - success
 *         -1 - fail, no relay card found
 *********************************************************/
int detect_relay_card_sample(char* portname, uint8_t* num_relays, char* serial)
{
   return 0;
}


/**********************************************************
 * Function get_relay_sample()
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
int get_relay_sample(char* portname, uint8_t relay, relay_state_t* relay_state, char* serial)
{
   return 0;
}


/**********************************************************
 * Function set_relay_sample()
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
int set_relay_sample(char* portname, uint8_t relay, relay_state_t relay_state, char* serial)
{ 
   return 0;
}
 
