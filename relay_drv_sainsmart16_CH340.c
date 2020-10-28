/******************************************************************************
 * 
 * Relay card control utility: Driver for Sainsmart 16-channel USB CH340G relay
 * control module:
 * 
 * Note:
 *   libusb
 * 
 * Description:
 *   This 16-channel module is used for USB control of the Sainsmart 16-channel
 *   relays.
 *   https://www.sainsmart.com/collections/internet-of-things/products/16-channel-9-36v-usb-relay-module
 * 
 * Author:
 *   Jean-Louis PREZUT
 *
 *   https://github.com/ldnelso2/sainsmart
 *   * https://github.com/obdev/v-usb/tree/master/examples/usbtool
 * 
 * Build instructions:
 *   gcc -c relay_drv_sainsmart16USB.c
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

/******************************************************************************
 * Communication protocol description
 * ==================================
 * 
 * Read command
 * ------------
 * 
 * 
 * Write command
 * -------------
 * 
 *****************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <usb.h>

#include "relay_drv.h"

#ifndef __OPENDEVICE_H_INCLUDED__
#define __OPENDEVICE_H_INCLUDED__

/* usbOpenDevice() error codes: */
#define USBOPEN_SUCCESS         0   /* no error */
#define USBOPEN_ERR_ACCESS      1   /* not enough permissions to open device */
#define USBOPEN_ERR_IO          2   /* I/O error */
#define USBOPEN_ERR_NOTFOUND    3   /* device not found */


/* Obdev's free USB IDs, see USB-IDs-for-free.txt for details */

#define USB_VID_OBDEV_SHARED        5824    /* obdev's shared vendor ID */
#define USB_PID_OBDEV_SHARED_CUSTOM 1500    /* shared PID for custom class devices */
#define USB_PID_OBDEV_SHARED_HID    1503    /* shared PID for HIDs except mice & keyboards */
#define USB_PID_OBDEV_SHARED_CDCACM 1505    /* shared PID for CDC Modem devices */
#define USB_PID_OBDEV_SHARED_MIDI   1508    /* shared PID for MIDI class devices */

#endif /* __OPENDEVICE_H_INCLUDED__ */


#define VENDOR_ID 0x1A86
#define DEVICE_ID 0x7523

static uint8_t g_num_relays=SAINSMART16_CH340_NUM_RELAYS ;

typedef struct mem_state {
    char * serial ;
    relay_state_t state[16] ;
    struct mem_state *next ;
} mem_state_t ; 

static mem_state_t *all_states = NULL ;

static char l_command[34][17] = {
 {58, 70, 69, 48, 53, 48, 48, 48, 48, 70, 70, 48, 48, 70, 69, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 48, 48, 48, 48, 48, 70, 68, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 49, 70, 70, 48, 48, 70, 68, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 49, 48, 48, 48, 48, 70, 67, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 50, 70, 70, 48, 48, 70, 67, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 50, 48, 48, 48, 48, 70, 66, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 51, 70, 70, 48, 48, 70, 66, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 51, 48, 48, 48, 48, 70, 65, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 52, 70, 70, 48, 48, 70, 65, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 52, 48, 48, 48, 48, 70, 57, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 53, 70, 70, 48, 48, 70, 57, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 53, 48, 48, 48, 48, 70, 56, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 54, 70, 70, 48, 48, 70, 56, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 54, 48, 48, 48, 48, 70, 55, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 55, 70, 70, 48, 48, 70, 55, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 55, 48, 48, 48, 48, 70, 54, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 56, 70, 70, 48, 48, 70, 54, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 56, 48, 48, 48, 48, 70, 53, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 57, 70, 70, 48, 48, 70, 53, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 57, 48, 48, 48, 48, 70, 52, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 65, 70, 70, 48, 48, 70, 52, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 65, 48, 48, 48, 48, 70, 51, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 66, 70, 70, 48, 48, 70, 51, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 66, 48, 48, 48, 48, 70, 50, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 67, 70, 70, 48, 48, 70, 50, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 67, 48, 48, 48, 48, 70, 49, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 68, 70, 70, 48, 48, 70, 49, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 68, 48, 48, 48, 48, 70, 48, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 69, 70, 70, 48, 48, 70, 48, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 69, 48, 48, 48, 48, 70, 70, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 70, 70, 70, 48, 48, 70, 70, 13, 10},
 {58, 70, 69, 48, 53, 48, 48, 48, 70, 48, 48, 48, 48, 70, 69, 13, 10},
 {58, 70, 69, 48, 49, 48, 48, 48, 48, 48, 48, 49, 48, 70, 49, 13, 10},
 {58, 70, 69, 48, 49, 48, 48, 50, 48, 48, 48, 48, 48, 70, 70, 13, 10}
} ;


int free_static_mem_sainsmart_16chan_CH340()
{
    mem_state_t * current ;
    struct usb_bus      *bus, *next_bus;
    struct usb_device   *dev, *next_dev;

    while ( all_states != NULL)
    {
        current = all_states ;
        all_states = (mem_state_t *)all_states->next ;
        
        free((char*)current->serial) ;
        free(current) ;
    }
    
    bus = usb_get_busses();
    while (bus != NULL){
        dev = bus->devices; 
        while (dev != NULL){
            next_dev = dev->next ;
            usb_free_dev(dev) ;
            dev = next_dev ;
        }
        next_bus = bus->next ;
        usb_free_bus(bus) ;
        bus = next_bus ;
    }
    
    return 0 ;
}

int close_sainsmart_16chan_CH340() 
{
   return 0 ;
}

int usbGetStringAscii(usb_dev_handle *dev, int index, char *buf, int buflen)
{
char    buffer[256];
int     rval, i;

    if((rval = usb_get_string_simple(dev, index, buf, buflen)) >= 0) /* use libusb version if it works */
        return rval;
    if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, 0x0409, buffer, sizeof(buffer), 5000)) < 0)
        return rval;
    if(buffer[1] != USB_DT_STRING){
        *buf = 0;
        return 0; 
    }
    if((unsigned char)buffer[0] < rval)
        rval = (unsigned char)buffer[0];
    rval /= 2;
    /* lossy conversion to ISO Latin1: */
    for(i=1;i<rval;i++){
        if(i > buflen)              /* destination buffer overflow */
            break;
        buf[i-1] = buffer[2 * i];
        if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
            buf[i-1] = '?';
    }
    buf[i-1] = 0;
    return i-1;
}

static void save_serial_in_state(char *serial)
{
    mem_state_t **mystate ;
    mystate = &all_states ;
    while ( (*mystate) != NULL)
    {
        if (!strcmp((*mystate)->serial, serial))
        {
            break ;
        }
        mystate = (mem_state_t **)&((*mystate)->next) ;
    }
    if ((*mystate) == NULL)
    {
        (*mystate) = malloc(sizeof(mem_state_t));
        (*mystate)->serial = strdup(serial) ;
        for (int k=0; k<16; k++) (*mystate)->state[k] = OFF ;
        (*mystate)->next = NULL ;
    }
}

static relay_state_t get_state(char *serial, uint8_t n_relay)
{
    mem_state_t *mystate ;
    
    mystate = all_states ;
    while ( mystate != NULL)
    {
        if (!strcmp(mystate->serial, serial))
        {
            return mystate->state[n_relay-1] ;
            break ;
        }
        mystate = (mem_state_t *)mystate->next ;
    }
    return INVALID ;
}

static void set_state(char *serial, uint8_t n_relay, relay_state_t state)
{
    mem_state_t *mystate ;
    
    mystate = all_states ;
    while ( mystate != NULL)
    {
        if (!strcmp(mystate->serial, serial))
        {
            mystate->state[n_relay-1] = state ;
            break ;
        }
        mystate = (mem_state_t *)mystate->next ;
    }
}

int usbOpenDevice(usb_dev_handle **device, int vendorID, int productID, char *my_serial, relay_info_t** relay_info)
{
    struct usb_bus      *bus;
    struct usb_device   *dev;
    usb_dev_handle      *handle = NULL;
    //int                 errorCode = USBOPEN_ERR_NOTFOUND;
    char                s_value[64] ; 
    relay_info_t        *rinfo ;
                    
    usb_find_busses();
    usb_find_devices();
    for(bus = usb_get_busses(); bus; bus = bus->next)
    {
        for(dev = bus->devices; dev; dev = dev->next)
        {  /* iterate over all devices on all busses */
            if(dev->descriptor.idVendor == vendorID && dev->descriptor.idProduct == productID)
            {
                char    vendor[256], product[256], serial[256] ;
                int     len;
                
                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if(!handle)
                {
//                    errorCode = USBOPEN_ERR_ACCESS;
                    fprintf(stderr, "Warning: cannot open VID=0x%04x PID=0x%04x: %s\n", dev->descriptor.idVendor, dev->descriptor.idProduct, usb_strerror());
                    continue;
                }
                /* now check whether the names match: */
                len = vendor[0] = 0;
                if(dev->descriptor.iManufacturer > 0)
                {
                    len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, vendor, sizeof(vendor));
                }
                if(len < 0)
                {
//                    errorCode = USBOPEN_ERR_ACCESS;
                    fprintf(stderr, "Warning: cannot query manufacturer for VID=0x%04x PID=0x%04x: %s\n", dev->descriptor.idVendor, dev->descriptor.idProduct, usb_strerror());
                }
                else
                {
//                    errorCode = USBOPEN_ERR_NOTFOUND;
                    len = product[0] = 0;
                    if(dev->descriptor.iProduct > 0)
                    {
                        len = usbGetStringAscii(handle, dev->descriptor.iProduct, product, sizeof(product));
                    }
                    if(len < 0)
                    {
    //                    errorCode = USBOPEN_ERR_ACCESS;
                        fprintf(stderr, "Warning: cannot query product for VID=0x%04x PID=0x%04x: %s\n", dev->descriptor.idVendor, dev->descriptor.idProduct, usb_strerror());
                    }
                    else
                    {
    //                        errorCode = USBOPEN_ERR_NOTFOUND;                          
                        s_value[0] = '\0' ;
                        sprintf((char *)s_value,"%04x",vendorID) ;
                        strcpy((char *)serial, (char *)s_value);
                        strcat((char *)serial, ":");
                        sprintf((char *)s_value,"%04x",productID) ;
                        strcat((char *)serial, (char *)s_value);
                        strcat((char *)serial, ":");
                        sprintf((char *)s_value,"%d",dev->devnum) ;
                        strcat((char *)serial, s_value) ;
                        
                        if (relay_info != NULL)
                        {
                            //printf("Device %d; save serial\n", i);
                            // Save serial number and type in current relay info struct
                            (*relay_info)->relay_type = SAINSMART16_CH340_RELAY_TYPE;
                            (*relay_info)->num_relays = g_num_relays ;
                            strcpy((*relay_info)->serial, (char *)serial) ;
                            // Allocate new struct
                            rinfo = malloc(sizeof(relay_info_t));
                            rinfo->next = NULL;
                            // Link current to new struct
                            (*relay_info)->next = rinfo;
                            // Move pointer to new struct
                            *relay_info = rinfo;
                              
                            save_serial_in_state((char *)serial) ;
                        }
                        else
                        {
                            if (!strcmp(my_serial,serial))
                            {
                                if (device != NULL)
                                {
                                    *device = handle;
                                    return 1 ;
                                }
                                else
                                {
                                    usb_close(handle);
                                    handle = NULL;
                                    return 1 ;
                                }
                            }
                        }
                    }
                }
                usb_close(handle);
                handle = NULL;
            }
        }
    }

    return 0;
}


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
int detect_relay_card_sainsmart_16chan_CH340(char* portname, uint8_t* num_relays, char* serial, relay_info_t** relay_info)
{
   /* Find all connected devices, if requested */
   if (relay_info != NULL)
   { 
      usb_init();
      usbOpenDevice(NULL, VENDOR_ID,DEVICE_ID, NULL, relay_info) ;
      
      return -1;
   }
   
   close_sainsmart_16chan_CH340() ; 

   usb_init();
   if (usbOpenDevice(NULL, VENDOR_ID,DEVICE_ID, serial, NULL) == 1)
   {
       save_serial_in_state(serial) ;
       
      /* Return parameters */
      if (num_relays) 
         *num_relays = g_num_relays;
      if (portname)
         sprintf(portname, "CH340G");
      //printf("DBG: portname %s\n", portname);
      return 0 ;   
   }
   
   return -1;
}


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
 *          <0 - fail
 *********************************************************/
int get_relay_sainsmart_16chan_CH340(char* portname, uint8_t relay, relay_state_t* relay_state, char* serial)
{
   *relay_state = get_state(serial, relay) ;
   return 0;
}


/**********************************************************
 * Function set_relay_sainsmart_16chan_CH340()
 * 
 * Description: Set new relay state
 * 
 * Parameters: portname (in)     - communication port
 *             relay (in)        - relay number
 *             relay_state (in)  - current relay state
 * 
 * Return:   0 - success
 *          <0 - fail
 *********************************************************/
int set_relay_sainsmart_16chan_CH340(char* portname, uint8_t relay, relay_state_t relay_state, char* serial)
{ 
   usb_dev_handle  *handle = NULL;
   int retries = 1;
   int len ;
   int  usbTimeout = 5000;
   int  usbConfiguration = 1;
   int  usbInterface = 0;

   if (relay<FIRST_RELAY || relay>(FIRST_RELAY+g_num_relays-1))
   {  
      fprintf(stderr, "ERROR: Relay number out of range\n");
      return -1;      
   }
   
   /* Open FTDI USB device */
   usb_init();
   if (usbOpenDevice(&handle, VENDOR_ID,DEVICE_ID, serial, NULL) != 1)
   {
      fprintf(stderr, "unable to open device\n") ;
      return -2;
   }

   if(usb_set_configuration(handle, usbConfiguration)){
            fprintf(stderr, "Warning: could not set configuration: %s\n", usb_strerror());
        }

   while((len = usb_claim_interface(handle, usbInterface)) != 0 && retries-- > 0){
#ifdef LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
            if(usb_detach_kernel_driver_np(handle, 0) < 0 ){
                fprintf(stderr, "Warning: could not detach kernel driver: %s\n", usb_strerror());
            }
#endif
   }
   if(len != 0)
            fprintf(stderr, "Warning: could not claim interface: %s\n", usb_strerror());
   
   if (relay_state == OFF)
   {
      len = usb_bulk_write(handle, 2, (char *)l_command[(relay-1)*2+1], 17, usbTimeout);
   }
   else
   {
      len = usb_bulk_write(handle, 2, (char *)l_command[(relay-1)*2], 17, usbTimeout);
   }
   
   set_state(serial,relay,relay_state) ;
   
   usb_release_interface(handle, usbInterface);
   usb_close(handle);
   return 0;
}
