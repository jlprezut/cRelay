/******************************************************************************
 * 
 * Relay card control utility: Main module
 * 
 * Description:
 *   This software is used to controls different type of relays cards.
 *   There are 3 ways to control the relays:
 *    1. via command line
 *    2. via web interface using a browser
 *    3. via HTTP API using a client application
 * 
 * Author:
 *   Ondrej Wisniewski (ondrej.wisniewski *at* gmail.com)
 *
 * Build instructions:
 *   make
 *   sudo make install
 * 
 * Last modified:
 *   14/01/2019
 *
 * Copyright 2015-2019, Ondrej Wisniewski 
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
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <syslog.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "data_types.h"
#include "config.h"
#include "relay_drv.h"

#define VERSION "0.20"
#define DATE "2020"

/* HTTP server defines */
#define SERVER "crelay/"VERSION
#define PROTOCOL "HTTP/1.1"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define DEFAULT_SERVER_PORT 8000

/* HTML tag definitions */
#define RELAY_TAG "pin"
#define STATE_TAG "status"
#define SERIAL_TAG "serial"
#define CARDID_TAG "cardid"

#define CONFIG_FILE "/etc/crelay.conf"

/* Global variables */
config_t config;
int portHttp;
int global_s = -1 ;

FILE *fin = NULL ;
FILE *fout = NULL ;

/**********************************************************
 * Function: config_cb()
 * 
 * Description:
 *           Callback function for handling the name=value
 *           pairs returned by the conf_parse() function
 * 
 * Returns:  0 on success, <0 otherwise
 *********************************************************/
static int config_cb(void* user, const char* section, const char* name, const char* value)
{
   config_t* pconfig = (config_t*)user;
   char buf[256];
   char template[256] ;
   card_info_t * current;
   
   #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
   
   if (MATCH("HTTP server", "server_iface")) 
   {
      pconfig->server_iface = strdup(value);
   } 
   else if (MATCH("HTTP server", "server_port")) 
   {
      pconfig->server_port = atoi(value);
   } 
   else if (MATCH("HTTP server", "pulse_duration")) 
   {
      pconfig->pulse_duration = atoi(value);
   }
   else if (MATCH("GPIO drv", "num_relays")) 
   {
      pconfig->gpio_num_relays = atoi(value);
   } 
   else if (MATCH("GPIO drv", "active_value")) 
   {
      pconfig->gpio_active_value = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay1_gpio_pin")) 
   {
      pconfig->relay1_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay2_gpio_pin")) 
   {
      pconfig->relay2_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay3_gpio_pin")) 
   {
      pconfig->relay3_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay4_gpio_pin")) 
   {
      pconfig->relay4_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay5_gpio_pin")) 
   {
      pconfig->relay5_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay6_gpio_pin")) 
   {
      pconfig->relay6_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay7_gpio_pin")) 
   {
      pconfig->relay7_gpio_pin = atoi(value);
   } 
   else if (MATCH("GPIO drv", "relay8_gpio_pin")) 
   {
      pconfig->relay8_gpio_pin = atoi(value);
   } 
   else if (MATCH("Sainsmart drv", "num_relays")) 
   {
      pconfig->sainsmart_num_relays = atoi(value);
   } 
   else if (MATCH("Boards","number"))
   {
      pconfig->number = atoi(value);
   }
   else
   {
      /* traitement Board list */
      int match_found = 0 ;
      
      for (int k=0 ; k<16; k++)
      {
         sprintf(template,"relay%d_label",k+1) ;
         if (MATCH("HTTP server", template))
         {
            pconfig->relay_label[k] = strdup(value) ;
            match_found = 1 ;
         }
      }
      
      if (match_found == 0 && pconfig->number > 0)
      {
         for (int i=1;i<=pconfig->number;i++)
         {
            sprintf((char *)&buf,"Board %d",i) ;
            if ((MATCH(buf,"serial")) || (MATCH(buf,"num_relays")) || (MATCH(buf,"comment")) || (MATCH(buf,"relay1_label")) || (MATCH(buf,"relay2_label")) || (MATCH(buf,"relay3_label")) || (MATCH(buf,"relay4_label")) || (MATCH(buf,"relay5_label")) || (MATCH(buf,"relay6_label")) || (MATCH(buf,"relay7_label")) || (MATCH(buf,"relay8_label")) || (MATCH(buf,"relay9_label")) || (MATCH(buf,"relay10_label")) || (MATCH(buf,"relay11_label")) || (MATCH(buf,"relay12_label")) || (MATCH(buf,"relay13_label")) || (MATCH(buf,"relay14_label")) || (MATCH(buf,"relay15_label")) || (MATCH(buf,"relay16_label")))
            {
               if (pconfig->card_list == NULL)
               {
                  pconfig->card_list = malloc(sizeof(card_info_t)) ;
                  pconfig->card_list->next = NULL ;
                  pconfig->card_list->card_id = i ;
                  pconfig->card_list->serial_type = SERIAL_FIRST;
                  pconfig->card_list->serial = NULL ;
                  for (int k=0 ; k<16; k++)
                  {
                     pconfig->card_list->relay_label[k] = NULL ;
                  }
                  current = pconfig->card_list ;
               }
               else
               {
                  current = pconfig->card_list ;
                  while ( current->card_id != i ) 
                  {
                     if (current->next == NULL)
                     {
                        current->next = malloc(sizeof(card_info_t)) ;
                        current->next->next = NULL ;
                        current->next->card_id = i ;
                        current->next->serial_type = SERIAL_FIRST;
                        current->next->serial = NULL ;
                        for (int k=0 ; k<16; k++)
                        {
                           current->next->relay_label[k] = NULL ;
                        }
                        current = current->next ;
                     }
                     else
                     {
                        current = current->next ;
                     }
                  }
               }
               if (MATCH(buf,"serial"))
               {
                  if (!strcmp(value,"NULL"))
                  {
                     current->serial_type = SERIAL_FIRST;
                  }
                  else if (!strcmp(value,"AUTO"))
                  {
                     current->serial_type = SERIAL_AUTO;
                  }
                  else
                  {
                     current->serial_type = SERIAL_FIXE;
                     current->serial = strdup(value);
                  }
               }
               else if (MATCH(buf,"num_relays")) 
               {
                  current->num_relays = atoi(value);
               }
               else if (MATCH(buf,"comment"))
               {
                  current->comment = strdup(value);
               }
               else 
               {
                  for (int k=0 ; k<16; k++)
                  {
                     sprintf(template,"relay%d_label",k+1) ;
                     if (MATCH(buf, template))
                     {
                        current->relay_label[k] = strdup(value) ;
                     }
                  }
               }
               match_found = 1 ;
            }
         }
      }
      
      if (match_found == 0 )
      {
         syslog(LOG_DAEMON | LOG_WARNING, "unknown config parameter %s/%s\n", section, name);
         return -1;  /* unknown section/name, error */
      }
   }
   return 0;
}

static void free_config()
{
   free((void *)config.server_iface); config.server_iface = NULL ;
   for (int k=0 ; k<16 ; k++)
   {
      free((void *)config.relay_label[k]); config.relay_label[k] = NULL ;
   }
   
   if (config.number != 0)
   {
      card_info_t * current;
      card_info_t * prev;
      current = config.card_list ;
      while ( current != NULL ) 
      {
         free((void *)current->serial); current->serial = NULL ;
         for (int k=0 ; k<16 ; k++)
         {
            free((void *)current->relay_label[k]); current->relay_label[k] = NULL ;
         }
         free((void *)current->comment); current->comment = NULL ;
         prev = current ;
         current = current->next ;
         free(prev) ;
      }
   }
}

int count_occurrence(char * str, int c)
{
   int occurrence = 0 ;
   
   for(int i = 0; str[i] != '\0'; ++i)
   {
       if(str[i] == c)
           ++occurrence;
   }
   
   return occurrence ;
}

int isNumeric(const char *str) 
{
    while(*str != '\0')
    {
        if(*str < '0' || *str > '9')
            return 0;
        str++;
    }
    return 1;
}

/**********************************************************
 * Function: exit_handler()
 * 
 * Description:
 *           Handles the cleanup at reception of the 
 *           TERM signal.
 * 
 * Returns:  -
 *********************************************************/
static void exit_handler(int signum)
{
   syslog(LOG_DAEMON | LOG_NOTICE, "Exit crelay daemon\n");
   
   free_config() ;
   crelay_close() ;
   crelay_free_static_mem() ;
   
   if (portHttp != 0)
   {
      syslog(LOG_DAEMON | LOG_NOTICE, "Trying Close port HTTP\n");
      if (global_s != -1) close(global_s) ; 
      close(portHttp);
      syslog(LOG_DAEMON | LOG_NOTICE, "Confirm Close port HTTP\n");
   }
   if (fout != NULL)
   {
      syslog(LOG_DAEMON | LOG_NOTICE, "Close fout\n");
      fclose(fout);
   }
   if (fin != NULL)
   {
      syslog(LOG_DAEMON | LOG_NOTICE, "Close fin\n");
      fclose(fin);
   }
   syslog(LOG_DAEMON | LOG_NOTICE, "Bye\n");
   exit(EXIT_SUCCESS);
}

                                           
/**********************************************************
 * Function send_headers()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void send_headers(FILE *f, int status, char *title, char *extra, char *mime, 
                  int length, time_t date)
{
   time_t now;
   char timebuf[128];
   fprintf(f, "%s %d %s\r\n", PROTOCOL, status, title);
   fprintf(f, "Server: %s\r\n", SERVER);
   //fprintf(f, "Access-Control-Allow-Origin: *\r\n"); // TEST For test only
   now = time(NULL);
   strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
   fprintf(f, "Date: %s\r\n", timebuf);
   if (extra) fprintf(f, "%s\r\n", extra);
   if (mime) fprintf(f, "Content-Type: %s; charset=utf-8\r\n", mime);
   if (length >= 0) fprintf(f, "Content-Length: %d\r\n", length);
   if (date != -1)
   {
      strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&date));
      fprintf(f, "Last-Modified: %s\r\n", timebuf);
   }
   fprintf(f, "Connection: close\r\n");
   fprintf(f, "\r\n");
}


/**********************************************************
 * Function java_script_src()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void java_script_src(FILE *f)
{
   fprintf(f, "<script type='text/javascript'>\r\n");
   fprintf(f, "function switch_relay(checkboxElem){\r\n");
   fprintf(f, "   var status = checkboxElem.checked ? 1 : 0;\r\n");
   fprintf(f, "   var pin =  checkboxElem.id;\r\n");
   fprintf(f, "   var serial = checkboxElem.getAttribute('serial');\r\n");                   // JLP ADD
   //fprintf(f, "   var url = '/gpio?pin='+pin+'&status='+status+'&serial='+serial;\r\n");     // JLP UPDATE
   fprintf(f, "   var url = '/api/serial/'+serial+'/'+pin+'/'+status;\r\n"); 
   fprintf(f, "   var xmlHttp = new XMLHttpRequest();\r\n");
   fprintf(f, "   xmlHttp.onreadystatechange = function () {\r\n");
   fprintf(f, "      if (this.readyState < 4)\r\n");
   fprintf(f, "         document.getElementById('status').innerHTML = '';\r\n");
   fprintf(f, "      else if (this.readyState == 4) {\r\n"); 
   fprintf(f, "         if (this.status == 0) {\r\n");
   fprintf(f, "            document.getElementById('status').innerHTML = \"Network error\";\r\n");
   fprintf(f, "            checkboxElem.checked = (status==0);\r\n");
   fprintf(f, "         }\r\n");
   fprintf(f, "         else if (this.status != 200) {\r\n");
   fprintf(f, "            document.getElementById('status').innerHTML = this.statusText;\r\n");
   fprintf(f, "            checkboxElem.checked = (status==0);\r\n");
   fprintf(f, "         }\r\n");
   // TODO: add update of all relays status here (according to API response in xmlHttp.responseText) 
   fprintf(f, "      }\r\n");
   fprintf(f, "   }\r\n");
   fprintf(f, "   xmlHttp.open( 'GET', url, true );\r\n");
   fprintf(f, "   xmlHttp.send( null );\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, "</script>\r\n");
}


/**********************************************************
 * Function style_sheet()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void style_sheet(FILE *f)
{
   fprintf(f, "<style>\r\n");
   fprintf(f, ".switch {\r\n");
   fprintf(f, "  position: relative;\r\n");
   fprintf(f, "  display: inline-block;\r\n");
   fprintf(f, "  width: 60px;\r\n");
   fprintf(f, "  height: 34px;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, ".switch input {\r\n"); 
   fprintf(f, "  opacity: 0;\r\n");
   fprintf(f, "  width: 0;\r\n");
   fprintf(f, "  height: 0;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, ".slider {\r\n");
   fprintf(f, "  position: absolute;\r\n");
   fprintf(f, "  cursor: pointer;\r\n");
   fprintf(f, "  top: 0;\r\n");
   fprintf(f, "  left: 0;\r\n");
   fprintf(f, "  right: 0;\r\n");
   fprintf(f, "  bottom: 0;\r\n");
   fprintf(f, "  background-color: #ccc;\r\n");
   fprintf(f, "  -webkit-transition: .4s;\r\n");
   fprintf(f, "  transition: .4s;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, ".slider:before {\r\n");
   fprintf(f, "  position: absolute;\r\n");
   fprintf(f, "  content: \"\";\r\n");
   fprintf(f, "  height: 26px;\r\n");
   fprintf(f, "  width: 26px;\r\n");
   fprintf(f, "  left: 4px;\r\n");
   fprintf(f, "  bottom: 4px;\r\n");
   fprintf(f, "  background-color: white;\r\n");
   fprintf(f, "  -webkit-transition: .4s;\r\n");
   fprintf(f, "  transition: .4s;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, "input:checked + .slider {\r\n");
   fprintf(f, "  background-color: #2196F3;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, "input:focus + .slider {\r\n");
   fprintf(f, "  box-shadow: 0 0 1px #2196F3;\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, "input:checked + .slider:before {\r\n");
   fprintf(f, "  -webkit-transform: translateX(26px);\r\n");
   fprintf(f, "  -ms-transform: translateX(26px);\r\n");
   fprintf(f, "  transform: translateX(26px);\r\n");
   fprintf(f, "}\r\n");
   fprintf(f, "</style>\r\n");   
}


/**********************************************************
 * Function web_page_header()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void web_page_header(FILE *f)
{
   /* Send http header */
   send_headers(f, 200, "OK", NULL, "text/html", -1, -1);   
   fprintf(f, "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\r\n");
   fprintf(f, "<html><head><title>Relay Card Control</title>\r\n");
   style_sheet(f);
   java_script_src(f);
   fprintf(f, "</head>\r\n");
   
   /* Display web page heading */
   fprintf(f, "<body><table style=\"text-align: left; width: 460px; background-color: #2196F3; font-family: Helvetica,Arial,sans-serif; font-weight: bold; color: white;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\r\n");
   fprintf(f, "<tbody><tr><td>\r\n");
   fprintf(f, "<span style=\"vertical-align: top; font-size: 48px;\">Relay Card Control</span><br>\r\n");
   fprintf(f, "<span style=\"font-size: 16px; color: rgb(204, 255, 255);\">Remote relay card control <span style=\"font-style: italic; color: white;\">made easy</span></span>\r\n");
   fprintf(f, "</td></tr></tbody></table><br>\r\n");  
}


/**********************************************************
 * Function web_page_footer()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void web_page_footer(FILE *f)
{
   /* Display web page footer */
   fprintf(f, "<table style=\"text-align: left; width: 460px; background-color: #2196F3;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\"><tbody>\r\n");
   fprintf(f, "<tr><td style=\"vertical-align: top; text-align: center;\"><span style=\"font-family: Helvetica,Arial,sans-serif; color: white;\"><a style=\"text-decoration:none; color: white;\" href=http://ondrej1024.github.io/crelay>crelay</a> | version %s | %s</span></td></tr>\r\n",
           VERSION, DATE);
   fprintf(f, "</tbody></table></body></html>\r\n");
}   


/**********************************************************
 * Function web_page_error()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void web_page_error(FILE *f)
{    
   /* No relay card detected, display error message on web page */
   fprintf(f, "<br><table style=\"text-align: left; width: 460px; background-color: yellow; font-family: Helvetica,Arial,sans-serif; font-weight: bold; color: black;\" border=\"0\" cellpadding=\"2\" cellspacing=\"2\">\r\n");
   fprintf(f, "<tbody><tr style=\"font-size: 20px; font-weight: bold;\">\r\n");
   fprintf(f, "<td>No compatible relay card detected !<br>\r\n");
   fprintf(f, "<span style=\"font-size: 14px; color: grey;  font-weight: normal;\">This can be due to the following reasons:\r\n");
   fprintf(f, "<div>- No supported relay card is connected via USB cable</div>\r\n");
   fprintf(f, "<div>- The relay card is connected but it is broken</div>\r\n");
   fprintf(f, "<div>- There is no GPIO sysfs support available or GPIO pins not defined in %s\r\n", CONFIG_FILE);
   fprintf(f, "<div>- You are running on a multiuser OS and don't have root permissions\r\n");
   fprintf(f, "</span></td></tbody></table><br>\r\n");
}   


/**********************************************************
 * Function read_httppost_data()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int read_httppost_data(FILE* f, char* data, size_t datalen)
{
   char buf[256];
   int data_len=0;
   
   /* POST request: data is provided after the page header.
    * Therefore we skip the header and then read the form
    * data into the buffer.
    */
   *data = 0;
   while (fgets(buf, sizeof(buf), f) != NULL)
   {
      //printf("%s", buf);
      /* Find length of form data */
      if (strstr(buf, "Content-Length:"))
      {
         data_len=atoi(buf+strlen("Content-Length:"));
         //printf("DBG: data length is %d\n", data_len);
      }
      
      /* Find end of header (empty line) */
      if (!strcmp(buf, "\r\n")) break;
   }

   /* Make sure we're not trying to overwrite the buffer */
   if (data_len >= datalen)
     return -1;

   /* Get form data string */
   if (!fgets(data, data_len+1, f)) return -1;
   *(data+data_len) = 0;
   
   return data_len;
}


/**********************************************************
 * Function read_httpget_data()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int read_httpget_data(char* buf, char* data, size_t datalen)
{
   char *datastr;
         
   /* GET request: data (if any) is provided in the first line
    * of the page header. Therefore we first check if there is
    * any data. If so we read the data into a buffer.
    *
    * Note that we may truncate the input if it's too long.
    */
   *data = 0;
   if ((datastr=strchr(buf, '?')) != NULL)
   {
       strncpy(data, datastr+1, datalen);
   }
   
   return strlen(data);
}

void exit_page(int sock)
{
   
   fout = fdopen(sock, "w");
   web_page_header(fout);
   fprintf(fout, "Program stopped<BR><BR>");
   web_page_footer(fout);
   fclose(fout) ;
   fout = NULL ;

}

void error_page(int sock, char * texte)
{
   fout = fdopen(sock, "w");
   //web_page_header(fout);
   send_headers(fout, 500, "Internal Error", NULL, "text/html", -1, -1);
   fprintf(fout, "ERROR: %s \r\n",texte);
   //web_page_footer(fout);
   fclose(fout) ;
   fout = NULL ;

}

void webui(int sock)
{

   int  i, serial_in_use, not_found;
   relay_info_t *relay_info;
   relay_info_t *prev_relay_info;
   relay_info_t *current_relay_info ;
   card_info_t *search ;
   char com_port[MAX_COM_PORT_NAME_LEN];
   relay_state_t rstate[MAX_NUM_RELAYS];
   uint8_t last_relay=FIRST_RELAY;
   
   fout = fdopen(sock, "w");
   /* Web request */
   web_page_header(fout);
   
   /* Display relay status and controls on web page */
   fprintf(fout, "<table style=\"text-align: left; width: 460px; background-color: white; font-family: Helvetica,Arial,sans-serif; font-weight: bold; font-size: 20px;\" border=\"0\" cellpadding=\"2\" cellspacing=\"3\"><tbody>\r\n");
    
   if (config.number == 0)
   {
      char cname[MAX_RELAY_CARD_NAME_LEN];
      crelay_get_relay_card_name(crelay_get_relay_card_type(), cname);
      
      syslog(LOG_DAEMON | LOG_NOTICE, "Step 12 no list");
      
      if (crelay_detect_all_relay_cards(&relay_info) != -1)
      { 
         while (relay_info->next != NULL)
         {
            crelay_detect_relay_card(com_port, &last_relay, relay_info->serial, NULL) ;
            crelay_get_relay_card_name(relay_info->relay_type, cname);
            fprintf(fout, "<tr style=\"font-size: 14px; background-color: lightgrey\">\r\n");
            fprintf(fout, "<td style=\"width: 200px;\">%s<br><span style=\"font-style: italic; font-size: 12px; color: grey; font-weight: normal;\">on %s</span></td>\r\n", 
                    cname, com_port);
            fprintf(fout, "<td style=\"width: 200px;\">Serial<br>%s<span style=\"font-style: italic; font-size: 12px; color: grey; font-weight: normal;\"></span></td>\r\n", 
                  relay_info->serial);
            
            fprintf(fout, "<td style=\"background-color: white;\"></td><td style=\"background-color: white;\"></td></tr>\r\n");
            for (i=FIRST_RELAY; i<=last_relay; i++)
            {
               crelay_get_relay(com_port, i, &rstate[i-1], (char *)(relay_info->serial));
               
               fprintf(fout, "<tr style=\"vertical-align: top; background-color: rgb(230, 230, 255);\">\r\n");
               fprintf(fout, "<td style=\"width: 300px;\">Relay %d<br><span style=\"font-style: italic; font-size: 16px; color: grey;\">%s</span></td>\r\n", 
                          i, config.relay_label[i-1]);
               fprintf(fout, "<td style=\"text-align: center; vertical-align: middle; width: 100px; background-color: white;\"><label class=\"switch\"><input type=\"checkbox\" %s id=%d serial=\"%s\" onchange=\"switch_relay(this)\"><span class=\"slider\"></span></label></td>\r\n", 
                       rstate[i-1]==ON?"checked":"",i,relay_info->serial);
            }
            
            prev_relay_info = relay_info ;
            relay_info = relay_info->next;
            free(prev_relay_info) ;
         }
      }
      else
      {
         fprintf(fout, "<td style=\"text-align: center; vertical-align: middle; width: 100px; background-color: white;\">No compatible device detected</td>\r\n") ;
      }
      free(relay_info) ;
   }
   else
   {
      
      card_info_t * current;
      current = config.card_list ;
      
      syslog(LOG_DAEMON | LOG_NOTICE, "Step 12 List");
      
      crelay_detect_all_relay_cards(&relay_info) ;
      
      while ( current != NULL ) 
      {     
         not_found = 1 ;
         if (crelay_detect_relay_card(com_port, &last_relay, (char *)current->serial, NULL) == -1)
         {
            not_found = 0 ;
            if (current->serial_type == SERIAL_AUTO)
               {
                  syslog(LOG_DAEMON | LOG_NOTICE, "type serial: %d\n", current->serial_type);
                  current_relay_info = relay_info ;
                  while (current_relay_info->next != NULL)
                  {
                     if (current_relay_info->num_relays == current->num_relays)
                     {
                        serial_in_use = 0 ;
                        search = config.card_list ;
                        while ( search != NULL ) 
                        {
                           if (search->serial != NULL && !strcmp(search->serial ,current_relay_info->serial))
                           {
                              serial_in_use = 1 ;
                              break ;
                           }
                           search = search->next ;
                        }
                        if (serial_in_use == 0)
                        {
                           free((void *)current->serial) ;
                           current->serial = strdup(current_relay_info->serial) ;
                           crelay_detect_relay_card(com_port, &last_relay, (char *)current->serial, NULL) ;
                           not_found = 1 ;
                           syslog(LOG_DAEMON | LOG_NOTICE, "serial affected : %s\n", current->serial);
                           break ;
                        }
                     }
                     current_relay_info = current_relay_info->next ;
                  }
                  if (current->serial == NULL) syslog(LOG_DAEMON | LOG_NOTICE, "serial: NOT FOUND\n");
               }
         }
            
         if (not_found == 0)
         {
            fprintf(fout, "<tr style=\"font-size: 14px; background-color: lightgrey\">\r\n");
            
            fprintf(fout, "<td style=\"width: 200px;\">%s<br><span style=\"font-style: italic; font-size: 12px; color: grey; font-weight: normal;\"></span></td>\r\n", 
                    current->comment);
            fprintf(fout, "<td style=\"width: 200px;\">board : %u<br><span style=\"font-style: italic; font-size: 12px; color: grey; font-weight: normal;\">Serial : %s</span></td>\r\n", 
                  current->card_id, current->serial);
                  
            fprintf(fout, "</tr><tr><td col=2 style=\"text-align: center; vertical-align: middle; width: 100px; background-color: white;\">Card not found</td>\r\n</tr>") ;
         }
         else
         {
            fprintf(fout, "<tr style=\"font-size: 14px; background-color: lightgrey\">\r\n");
            
            fprintf(fout, "<td style=\"width: 200px;\">%s<br><span style=\"font-style: italic; font-size: 12px; color: grey; font-weight: normal;\"></span></td>\r\n", 
                    current->comment);
            fprintf(fout, "<td style=\"width: 200px;\">board : %u<br><span style=\"font-style: italic; font-size: 12px; color: grey; font-weight: normal;\">Serial : %s</span></td>\r\n", 
                  current->card_id, current->serial);

            fprintf(fout, "<td style=\"background-color: white;\"></td><td style=\"background-color: white;\"></td></tr>\r\n");
            for (i=1; i<=current->num_relays; i++)
            {
               fprintf(fout, "<tr style=\"vertical-align: top; background-color: rgb(230, 230, 255);\">\r\n");
               fprintf(fout, "<td style=\"width: 300px;\">Relay %d<br><span style=\"font-style: italic; font-size: 16px; color: grey;\">%s</span></td>\r\n", 
                     i, current->relay_label[i-1]);
               syslog(LOG_DAEMON | LOG_NOTICE, "Step 13 : com_port : %s / serial : %s",com_port,current->serial);
               if (crelay_get_relay(com_port, i, &rstate[i-1], (char *)(current->serial)) == 0)
               {
                  fprintf(fout, "<td style=\"text-align: center; vertical-align: middle; width: 100px; background-color: white;\"><label class=\"switch\"><input type=\"checkbox\" %s id=%d serial=\"%s\" onchange=\"switch_relay(this)\"><span class=\"slider\"></span></label></td>\r\n", 
                          rstate[i-1]==ON?"checked":"",i,current->serial);
               }
               else
               {
                  fprintf(fout, "<td style=\"text-align: center; vertical-align: middle; width: 100px; background-color: white;\">Not Avalaible</td>\r\n") ;
               }
               
               syslog(LOG_DAEMON | LOG_NOTICE, "Step 14");
            }
         }
         
         current = current->next ;
      }
      
      while (relay_info->next != NULL)
      {
         prev_relay_info = relay_info ;
         relay_info = relay_info->next;
         free(prev_relay_info) ;
      }
      free(relay_info) ;
   }
   
   fprintf(fout, "</tbody></table><br>\r\n");
   fprintf(fout, "<span id=\"status\" style=\"font-size: 16px; color: red; font-family: Helvetica,Arial,sans-serif;\"></span><br><br>\r\n");
   
   web_page_footer(fout);
   fclose(fout) ;
   fout = NULL ;
}

void send_json_info(int sock, relay_info_t **relay_info)
{
   relay_info_t *prev_relay_info;
   int i = 1 ;
   char cname[MAX_RELAY_CARD_NAME_LEN];
   
   fout = fdopen(sock, "w");
   
   send_headers(fout, 200, "OK", NULL, "text/plain", -1, -1);
   
   /* Detect all cards connected to the system */
   
   fprintf(fout, "{ \"meta\": { }, \"data\": [ ");
   while ((*relay_info)->next != NULL)
   {
      crelay_get_relay_card_name((*relay_info)->relay_type, cname);
      fprintf(fout, "{ \"num\" : \"%d\", \"relay_type\": \"%s\", \"serial\": \"%s\" }", i++, cname, (*relay_info)->serial);
      prev_relay_info = (*relay_info) ;
      (*relay_info) = (*relay_info)->next;
      free(prev_relay_info) ;
      
      if ((*relay_info)->next != NULL) fprintf(fout, " , ") ;
   }
   fprintf(fout, " ] }");

   fclose(fout) ;
   fout = NULL ;
}

void send_json_card(int sock, char * com_port, uint8_t first_relay, uint8_t last_relay, char * serial)
{
   relay_state_t rstate[MAX_NUM_RELAYS];
   int i ;
   
   for (i=first_relay; i<=last_relay; i++)
   {
      syslog(LOG_DAEMON | LOG_NOTICE, "Step 10-%d",i);
      if (crelay_get_relay(com_port, i, &rstate[i-1], serial) != 0)
      {
         rstate[i-1] = INVALID ;
      }
   }
   
   syslog(LOG_DAEMON | LOG_NOTICE, "Step 11");
   
   /* HTTP API request, send response */
   fout = fdopen(sock, "w");
   send_headers(fout, 200, "OK", NULL, "text/plain", -1, -1);
   
   fprintf(fout, "{ \"meta\": { }, \"data\": [ ");
   for (i=first_relay; i<=last_relay; i++)
   {
      fprintf(fout, "{ \"relay\" : \"%d\", \"value\": \"%d\" }", i, rstate[i-1]);
      if (i != last_relay) fprintf(fout, " , ") ;
   }
   fprintf(fout, " ] }");

   fclose(fout) ;
   fout = NULL ;
}

void send_json_board(int sock, relay_info_t *relay_info)
{
   card_info_t *current;
   card_info_t *search ;
   relay_info_t *relay_info_origine ;
   relay_info_t *current_relay_info;
   char cname[MAX_RELAY_CARD_NAME_LEN];
   int found_card, serial_in_use, not_found;
   
   relay_info_origine = relay_info ;
   
   fout = fdopen(sock, "w");
   send_headers(fout, 200, "OK", NULL, "text/plain", -1, -1);
   
   fprintf(fout, "{ \"meta\": { }, \"data\": [ ");
   current = config.card_list ;
   while ( current != NULL ) 
   {
      relay_info = relay_info_origine ;
      found_card = 0 ;
      
      while (relay_info->next != NULL)
      {
         if (!strcmp(relay_info->serial, current->serial))
         {
            crelay_get_relay_card_name(relay_info->relay_type, cname) ;
            found_card = 1 ;
            break ;
         }
         else
         {
            if (current->serial_type == SERIAL_AUTO)
            {
               current_relay_info = relay_info ;
               not_found = 0 ;
               while (current_relay_info->next != NULL)
               {
                  if (current_relay_info->num_relays == current->num_relays)
                  {
                     serial_in_use = 0 ;
                     search = config.card_list ;
                     while ( search != NULL ) 
                     {
                        if (search->serial != NULL && !strcmp(search->serial ,current_relay_info->serial))
                        {
                           serial_in_use = 1 ;
                           break ;
                        }
                        search = search->next ;
                     }
                     if (serial_in_use == 0)
                     {
                        free((void *)current->serial) ;
                        current->serial = strdup(current_relay_info->serial) ;
                        not_found = 1 ;
                        syslog(LOG_DAEMON | LOG_NOTICE, "serial affected : %s\n", current->serial);
                        break ;
                     }
                  }
                  current_relay_info = current_relay_info->next ;
               }
               if (not_found == 1)
               {
                  crelay_get_relay_card_name(relay_info->relay_type, cname) ;
                  found_card = 1 ;
                  break ;
               }
            }
         }
         relay_info = relay_info->next;
      }
      
      fprintf(fout, "{ \"board\" : \"%d\", \"comment\" : \"%s\", \"relay_type\": \"%s\", \"serial\": \"%s\" }", current->card_id,current->comment, (found_card == 1)?cname:"NOT FOUND", current->serial);          

      current = current->next ;
      if (current != NULL) fprintf(fout, " , ") ;
   }
   fprintf(fout, " ] }");
   
   fclose(fout) ;
   fout = NULL ;
}

void send_json_setrelay(int sock, char * com_port, uint8_t nrelay, uint8_t nstate, char * serial)
{
   //printf("nrelay/value : %d/%d\n", nrelay,nstate) ;
   crelay_set_relay(com_port, nrelay, nstate, serial);
   send_json_card(sock,com_port,nrelay,nrelay,serial) ;
   
}

void send_json_no_device(int sock)
{
   
   fout = fdopen(sock, "w");
   send_headers(fout, 200, "OK", NULL, "text/plain", -1, -1);
   fprintf(fout, "{ \"meta\": { \"error\" : 1001, \"message\": \"No compatible device detected.\" }, \"data\": { } }");
   fclose(fout) ;
   fout = NULL ;
}

void send_json_unavailable(int sock)
{
   
   fout = fdopen(sock, "w");
   send_headers(fout, 200, "OK", NULL, "text/plain", -1, -1);
   fprintf(fout, "{ \"meta\": { \"error\" : 1002, \"message\": \"function unavailable in this context.\" }, \"data\": { } }");
   fclose(fout) ;
   fout = NULL ;
}

void send_json_invalid_param(int sock)
{
   
   fout = fdopen(sock, "w");
   send_headers(fout, 200, "OK", NULL, "text/plain", -1, -1);
   fprintf(fout, "{ \"meta\": { \"error\" : 1003, \"message\": \"Invalid value.\" }, \"data\": { } }");
   fclose(fout) ;
   fout = NULL ;
}

/**********************************************************
 * Function new_process_http_request()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int new_process_http_request(int sock)
{
   char formdata[64];
   char buf[256];
   char *method;
   char *url = NULL;
   int  formdatalen;
   int exit_value = 0 ;
   char com_port[MAX_COM_PORT_NAME_LEN];
   uint8_t last_relay=FIRST_RELAY;
   char *serial = NULL;
   char *nrelay = NULL ;
   char *nvalue = NULL ;
   char *ncard_id = NULL ;
   int vrelay ;
   int value ;
   int vcard_id ;
   relay_info_t *relay_info;
   relay_info_t *prev_relay_info;
   relay_info_t *current_relay_info;
   int action, serial_in_use ;
   card_info_t *search ;

   fin = NULL ;
   fout = NULL ;

   /* Open file for input */
   fin = fdopen(sock, "r");
   
   /* Read  first line of request header which contains 
    * the request method and url seperated by a space
    */
   if (!fgets(buf, sizeof(buf), fin)) 
   {
      exit_value = -1;
      goto new_done;
   }
//   printf("********** Raw data ***********\n");
//   printf("%s", buf);

   method = strtok(buf, " ");
   if (!method) 
   {
      exit_value = -1;
      goto new_done;
   }
   //printf("method: %s\n", method);
   
   url = strtok(NULL, " ");
   if (!url)
   {
      exit_value = -2;
      goto new_done;
   }
   //printf("url: %s\n", url);
   
   /* Check the request method we are dealing with */
   if (strcasecmp(method, "POST") == 0)
   {
      formdatalen = read_httppost_data(fin, formdata, sizeof(formdata));
   }
   else if (strcasecmp(method, "GET") == 0)
   {
      formdatalen = read_httpget_data(url, formdata, sizeof(formdata));
   }
   else
   {
      exit_value = -3;
      goto new_done;
   }

//syslog(LOG_DAEMON | LOG_NOTICE, "formdatalen : %d",formdatalen);
//syslog(LOG_DAEMON | LOG_NOTICE, "Formdata : %s",formdata);

   /* Send an error if we failed to read the form data properly */
   if (formdatalen < 0) {
     error_page(sock,"Invalid Input.") ;
     goto new_done ;
   }
   
//syslog(LOG_DAEMON | LOG_NOTICE, "URL : %s",url);

   if (!strcmp(url,"/quit"))
   {
      exit_page(sock) ;
      exit_value = 1;
      goto new_done ;
   }

   if (!strcmp(url,"/"))
   {
      webui(sock) ;
      goto new_done ;
   }

   if (!strcmp(url,"/api/info") || !strcmp(url,"/api/serial"))   // Attention si config.number !=0, faire la liste des cartes config
   {
      if (crelay_detect_all_relay_cards(&relay_info) == -1)
      {
         send_json_no_device(sock) ;
      }
      else
      {
         send_json_info(sock,&relay_info) ;
      }
      free(relay_info) ;
      goto new_done ;
   }

   if ((!strncmp(url,"/api/board",10) && (config.number == 0)) || 
         (!strncmp(url,"/api/card",9) && (config.number != 0)) )
   {
      send_json_unavailable(sock) ;
      goto new_done ;
   }
   
   if (!strcmp(url,"/api/board"))      // Attention se limiter à liste des cartes
   {
      crelay_detect_all_relay_cards(&relay_info) ;
      send_json_board(sock,relay_info) ;
      
      while (relay_info->next != NULL)
      {
         prev_relay_info = relay_info ;
         relay_info = relay_info->next;
         free(prev_relay_info) ;
      }
      free(relay_info) ;
      goto new_done ;
   }

   if (!strncmp(url,"/api/card",9))
   {
      
      if (crelay_detect_relay_card(com_port, &last_relay, NULL, NULL) == -1)
      {
         send_json_no_device(sock) ;
         goto new_done ;
      }
      
      if (!strcmp(url,"/api/card"))
      {
         send_json_card(sock, com_port, FIRST_RELAY, last_relay, NULL) ;
         goto new_done ;
      }
      
      if ( url[9] != '/' )
      {
         send_json_invalid_param(sock) ;
         goto new_done;
      }
      
      action = 0 ;
      switch (count_occurrence(url,'/')) {
         
         case 4:
            nvalue = strrchr(url,'/') ;
            nvalue[0] = '\0' ;
            nvalue = &(nvalue[1]) ;
            value = (isNumeric(nvalue))?atoi(nvalue):-1 ;
            action = 2 ; 
            
         case 3:
            nrelay = strrchr(url,'/') ;
            nrelay[0] = '\0' ;
            nrelay = &(nrelay[1]) ;
            vrelay = atoi(nrelay) ;
            action = (action==0)?1:action ;
            break ;
      }
      
      switch (action) {
         
         case 0:
            send_json_invalid_param(sock) ;
            break ;
         
         case 1:
            if (vrelay <= 0 || vrelay > 16)
            {
               send_json_invalid_param(sock) ;
            }
            else
            {
               send_json_card(sock, com_port, vrelay, vrelay, NULL) ;
            }
            break ;
            
         case 2:
            if ((value != 0 && value != 1) || vrelay <= 0 || vrelay > 16)
            {
               send_json_invalid_param(sock) ;
            }
            else
            {
               send_json_setrelay(sock, com_port, vrelay, value, NULL) ;
            }
            break ;
      }
      
      goto new_done ;
   }
   
   if (!strncmp(url,"/api/serial/",12) || !strncmp(url,"/api/board/",11))
   {
      if (!strncmp(url,"/api/serial/",12))    // Attention si config.number !=0, faire la liste des cartes config
      {
         serial = &(url[12]) ;
      }
      else
      {
         ncard_id = &(url[11]) ;
         vcard_id = atoi(ncard_id) ;
      
         if (vcard_id != 0)
         {
            card_info_t * current;
            current = config.card_list ;
            while ( current != NULL ) 
            {
               if (current->card_id == vcard_id)
               {
                  if (crelay_detect_relay_card(com_port, &last_relay, (char *)current->serial, NULL) == -1)
                  {
                     if (current->serial_type == SERIAL_AUTO)
                     {
                        crelay_detect_all_relay_cards(&relay_info) ;

                        current_relay_info = relay_info ;
                        while (current_relay_info->next != NULL)
                        {
                           if (current_relay_info->num_relays == current->num_relays)
                           {
                              serial_in_use = 0 ;
                              search = config.card_list ;
                              while ( search != NULL ) 
                              {
                                 if (search->serial != NULL && !strcmp(search->serial ,current_relay_info->serial))
                                 {
                                    serial_in_use = 1 ;
                                    break ;
                                 }
                                 search = search->next ;
                              }
                              if (serial_in_use == 0)
                              {
                                 free((void *)current->serial) ;
                                 current->serial = strdup(current_relay_info->serial) ;
                                 syslog(LOG_DAEMON | LOG_NOTICE, "serial affected : %s\n", current->serial);
                                 break ;
                              }
                           }
                           current_relay_info = current_relay_info->next ;
                        }
                        
                        while (relay_info->next != NULL)
                        {
                           prev_relay_info = relay_info ;
                           relay_info = relay_info->next;
                           free(prev_relay_info) ;
                        }
                        free(relay_info) ;
                        relay_info = NULL ;
                     }
                  }
                  serial = (char *)current->serial ; 
                  current = NULL ;
                  break ;
               }
               else
               {
                  current = current->next ;
               }
            }
         }
      }
      
      action = 0 ;
      switch (count_occurrence(url,'/')) {
         
         case 5:
            nvalue = strrchr(url,'/') ;
            nvalue[0] = '\0' ;
            nvalue = &(nvalue[1]) ;
            value = (isNumeric(nvalue))?atoi(nvalue):-1 ;
            action = 3 ; 
         
         case 4:
            nrelay = strrchr(url,'/') ;
            nrelay[0] = '\0' ;
            nrelay = &(nrelay[1]) ;
            vrelay = atoi(nrelay) ;
            action = (action==0)?2:action ; 
            
         case 3:
            strrchr(url,'/')[0] = '\0' ;
            action = (action==0)?1:action ;
            break ;
      }
      
      syslog(LOG_DAEMON | LOG_NOTICE, "serial B : %s\n", serial);
      if (crelay_detect_relay_card(com_port, &last_relay, serial, NULL) == -1)
      {
         send_json_no_device(sock) ;
         goto new_done ;
      }
      
      switch (action) {
         
         case 0:
            send_json_invalid_param(sock) ;
            break ;
         
         case 1:
            send_json_card(sock, com_port, FIRST_RELAY, last_relay, serial) ;
            break ;
            
         case 2:
            if (vrelay <= 0 || vrelay > 16)
            {
               send_json_invalid_param(sock) ;
            }
            else
            {
               send_json_card(sock, com_port, vrelay, vrelay, serial) ;
            }
            break ;
            
         case 3:
            if ((value != 0 && value != 1) || vrelay <= 0 || vrelay > 16)
            {
               send_json_invalid_param(sock) ;
            }
            else
            {
               send_json_setrelay(sock, com_port, vrelay, value, serial) ;
            }
            break ;
      }
      
      goto new_done ;
   }

   error_page(sock,"PAGE INTROUVABLE") ;

 new_done:
   if (fout) fclose(fout);
   fout = NULL ;
   if (fin) fclose(fin);
   fin = NULL ;

   return exit_value ;

}

/**********************************************************
 * Function print_usage()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
void print_usage()
{
   relay_type_t rtype;
   char cname[60];
   
   printf("crelay, version %s\n\n", VERSION);
   printf("This utility provides a unified way of controlling different types of relay cards.\n");
   printf("Supported relay cards:\n");
   for(rtype=NO_RELAY_TYPE+1; rtype<LAST_RELAY_TYPE; rtype++)
   {
      crelay_get_relay_card_name(rtype, cname);
      printf("  - %s\n", cname);
   }
   printf("\n");
   printf("The program can be run in interactive (command line) mode or in daemon mode with\n");
   printf("built-in web server.\n\n");
   printf("Interactive mode:\n");
   printf("    crelay -i | [-s <serial number>] <relay number> [ON|OFF]\n\n");
   printf("       -i print relay information\n\n");
   printf("       The state of any relay can be read or it can be changed to a new state.\n");
   printf("       If only the relay number is provided then the current state is returned,\n");
   printf("       otherwise the relays state is set to the new value provided as second parameter.\n");
   printf("       The USB communication port is auto detected. The first compatible device\n");
   printf("       found will be used, unless -s switch and a serial number is passed.\n\n");
   printf("Daemon mode:\n");
   printf("    crelay -d|-D [<relay1_label> [<relay2_label> [<relay3_label> [<relay4_label>]]]] \n\n");
   printf("       -d use daemon mode, run in foreground\n");
   printf("       -D use daemon mode, run in background\n\n");
   printf("       In daemon mode the built-in web server will be started and the relays\n");
   printf("       can be completely controlled via a Web browser GUI or HTTP API.\n");
   printf("       The config file %s will be used, if present.\n", CONFIG_FILE);
   printf("       Optionally a personal label for each relay can be supplied as command\n");
   printf("       line parameter which will be displayed next to the relay name on the\n");
   printf("       web page.\n\n");
   printf("       To access the web interface point your Web browser to the following address:\n");
   printf("       http://<my-ip-address>:%d\n\n", DEFAULT_SERVER_PORT);
   printf("       To use the HTTP API send a GET request (response JSON format) from the client to this URL:\n");
   printf("       http://<my-ip-address>:%d/api/info\n", DEFAULT_SERVER_PORT );
   printf("       http://<my-ip-address>:%d/api/card\n", DEFAULT_SERVER_PORT );
   printf("       http://<my-ip-address>:%d/api/card/<r>\n", DEFAULT_SERVER_PORT );
   printf("       http://<my-ip-address>:%d/api/card/<r>/<v>\n", DEFAULT_SERVER_PORT );
   printf("       http://<my-ip-address>:%d/api/serial/<s>\n", DEFAULT_SERVER_PORT );
   printf("       http://<my-ip-address>:%d/api/serial/<s>/<v>\n", DEFAULT_SERVER_PORT );
   printf("       http://<my-ip-address>:%d/api/board\n", DEFAULT_SERVER_PORT );
   printf("       http://<my-ip-address>:%d/api/board/<c>\n", DEFAULT_SERVER_PORT );
   printf("       http://<my-ip-address>:%d/api/board/<c>/<r>\n", DEFAULT_SERVER_PORT );
   printf("       http://<my-ip-address>:%d/api/board/<c>/<r>/<v>\n", DEFAULT_SERVER_PORT );
   printf("       http://<my-ip-address>:%d/quit\n\n", DEFAULT_SERVER_PORT ); 
   printf("       With <r> : relay (between 1 and 16)\n"); 
   printf("            <v> : status (0 : OFF / 1 : ON)\n"); 
   printf("            <s> : card serial number\n"); 
   printf("            <c> : board (from the file crelay.conf)\n\n");
   
}


/**********************************************************
 * Function main()
 * 
 * Description:
 * 
 * Parameters:
 * 
 *********************************************************/
int main(int argc, char *argv[])
{
      relay_state_t rstate;
      char com_port[MAX_COM_PORT_NAME_LEN];
      char cname[MAX_RELAY_CARD_NAME_LEN];
      char template[255] ;
      char *serial = NULL;
      uint8_t num_relays=FIRST_RELAY;
      relay_info_t *relay_info;
      relay_info_t *prev_relay_info;
      relay_info_t *current_relay_info;
      int argn = 1;
      int err;
      int i = 1;
      card_info_t * current;
      card_info_t * search;

   portHttp = 0 ;

   if (argc==1)
   {
      print_usage();
      exit(EXIT_SUCCESS);
   }
   
   if (!strcmp(argv[1],"-d") || !strcmp(argv[1],"-D"))
   {
      /*****  Daemon mode *****/
      
      struct sockaddr_in sin;
      struct in_addr iface;
      int port=DEFAULT_SERVER_PORT;
      int sock;
      int i;
      int serial_in_use ;
      
      iface.s_addr = INADDR_ANY;

      
      openlog("crelay", LOG_PID|LOG_CONS, LOG_USER);
      syslog(LOG_DAEMON | LOG_NOTICE, "Starting crelay daemon (version %s)\n", VERSION);
   
      /* Setup signal handlers */
      signal(SIGINT, exit_handler);   /* Ctrl-C */
      signal(SIGTERM, exit_handler);  /* "regular" kill */
   
      /* Load configuration from .conf file */
      memset((void*)&config, 0, sizeof(config_t));
      config.number = 0 ;
      config.card_list = NULL ;
      for (int k=0;k<16;k++)
      {
         config.relay_label[k] = NULL ;
      }
      
      if (conf_parse(CONFIG_FILE, config_cb, &config) >= 0) 
      {
         syslog(LOG_DAEMON | LOG_NOTICE, "Config parameters read from %s:\n", CONFIG_FILE);
         syslog(LOG_DAEMON | LOG_NOTICE, "***************************\n");
         if (config.server_iface != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "server_iface: %s\n", config.server_iface);
         if (config.server_port != 0)     syslog(LOG_DAEMON | LOG_NOTICE, "server_port: %u\n", config.server_port);
         for (int k=0; k<16; k++)
         {
            if (config.relay_label[k] != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "relay_label %d: %s\n",k+1, config.relay_label[k]);
         }
         
         if (config.pulse_duration != 0)  syslog(LOG_DAEMON | LOG_NOTICE, "pulse_duration: %u\n", config.pulse_duration);
         if (config.gpio_num_relays != 0) syslog(LOG_DAEMON | LOG_NOTICE, "gpio_num_relays: %u\n", config.gpio_num_relays);
         if (config.gpio_active_value >= 0) syslog(LOG_DAEMON | LOG_NOTICE, "gpio_active_value: %u\n", config.gpio_active_value);
         if (config.relay1_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay1_gpio_pin: %u\n", config.relay1_gpio_pin);
         if (config.relay2_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay2_gpio_pin: %u\n", config.relay2_gpio_pin);
         if (config.relay3_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay3_gpio_pin: %u\n", config.relay3_gpio_pin);
         if (config.relay4_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay4_gpio_pin: %u\n", config.relay4_gpio_pin);
         if (config.relay5_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay5_gpio_pin: %u\n", config.relay5_gpio_pin);
         if (config.relay6_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay6_gpio_pin: %u\n", config.relay6_gpio_pin);
         if (config.relay7_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay7_gpio_pin: %u\n", config.relay7_gpio_pin);
         if (config.relay8_gpio_pin != 0) syslog(LOG_DAEMON | LOG_NOTICE, "relay8_gpio_pin: %u\n", config.relay8_gpio_pin);
         if (config.sainsmart_num_relays != 0) syslog(LOG_DAEMON | LOG_NOTICE, "sainsmart_num_relays: %u\n", config.sainsmart_num_relays);
         if (config.number != 0)
         {
            syslog(LOG_DAEMON | LOG_NOTICE, "Number Card in List: %u\n", config.number);
            current = config.card_list ;
            crelay_detect_all_relay_cards(&relay_info) ;
            
            while ( current != NULL ) 
            {
               syslog(LOG_DAEMON | LOG_NOTICE, "card_id: %u\n", current->card_id);
               if (current->serial != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "serial: %s\n", current->serial);
               if (current->num_relays != 0) syslog(LOG_DAEMON | LOG_NOTICE, "num_relays: %u\n", current->num_relays);
               for (int k=0; k<16; k++)
               {
                  if (current->relay_label[k] != NULL)
                  {
                     syslog(LOG_DAEMON | LOG_NOTICE, "relay%d_label : %s\n",k+1, current->relay_label[k]);
                  }
                  else
                  {
                     /* Set default relay labels if no exist in config file */
                     sprintf(template,"My appliance %d",k+1) ;
                     current->relay_label[k] = strdup(template);
                  }
               }
               if (current->comment != NULL) syslog(LOG_DAEMON | LOG_NOTICE, "comment: %s\n", current->comment);
               
               if (current->serial_type == SERIAL_AUTO || current->serial_type == SERIAL_FIRST)
               {
                  syslog(LOG_DAEMON | LOG_NOTICE, "type serial: %d\n", current->serial_type);
                  current_relay_info = relay_info ;
                  while (current_relay_info->next != NULL)
                  {
                     if (current_relay_info->num_relays == current->num_relays)
                     {
                        serial_in_use = 0 ;
                        search = config.card_list ;
                        while ( search != NULL ) 
                        {
                           if (search->serial != NULL && !strcmp(search->serial ,current_relay_info->serial))
                           {
                              serial_in_use = 1 ;
                              break ;
                           }
                           search = search->next ;
                        }
                        if (serial_in_use == 0)
                        {
                           current->serial = strdup(current_relay_info->serial) ;
                           syslog(LOG_DAEMON | LOG_NOTICE, "serial affected : %s\n", current->serial);
                           break ;
                        }
                     }
                     current_relay_info = current_relay_info->next ;
                  }
                  if (current->serial == NULL) syslog(LOG_DAEMON | LOG_NOTICE, "serial: NOT FOUND\n");
               }
               
               current = current->next ;
            }
            
            while (relay_info->next != NULL)
            {
               prev_relay_info = relay_info ;
               relay_info = relay_info->next;
               free(prev_relay_info) ;
            }
            free(relay_info) ;
         }
         else
         {
            syslog(LOG_DAEMON | LOG_NOTICE, "No card list\n");
         }
         
         syslog(LOG_DAEMON | LOG_NOTICE, "***************************\n");
         
         /* Set default relay labels if no exist in config file */
         for (int k=0; k<16; k++)
         {
            sprintf(template,"My appliance %d",k+1) ;
            if (config.relay_label[k] == NULL) config.relay_label[k] = strdup(template); 
         }
         
         /* Get listen interface from config file */
         if (config.server_iface != NULL)
         {
            if (inet_aton(config.server_iface, &iface) == 0)
            {
               syslog(LOG_DAEMON | LOG_NOTICE, "Invalid iface address in config file, using default value");
            }
         }
         
         /* Get listen port from config file */
         if (config.server_port > 0)
         {
            port = config.server_port;
         }

      }
      else
      {
         syslog(LOG_DAEMON | LOG_NOTICE, "Can't load %s, using default parameters\n", CONFIG_FILE);
      }

      /* Ensure pulse duration is valid **/
      if (config.pulse_duration == 0)
      {
         config.pulse_duration = 1;
      }
      
      /* Parse command line for relay labels (overrides config file)*/
      for (i=0; i<argc-2 && i<MAX_NUM_RELAYS; i++)
      {
         free((void *)config.relay_label[i]);
         config.relay_label[i] = strdup(argv[i+2]);
      }         
      
      /* Start build-in web server */
      sock = socket(AF_INET, SOCK_STREAM, 0);
      struct linger lin;
      lin.l_onoff = 0;
      lin.l_linger = 0;
      setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&lin, sizeof(int));
      setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) ;

      sin.sin_family = AF_INET;
      sin.sin_addr.s_addr = iface.s_addr;
      sin.sin_port = htons(port);
      if (bind(sock, (struct sockaddr *) &sin, sizeof(sin)) != 0)
      {
         syslog(LOG_DAEMON | LOG_ERR, "Failed to bind socket to port %d : %s", port, strerror(errno));
         free_config();
         crelay_free_static_mem() ;
         crelay_close() ;
         exit(EXIT_FAILURE);         
      }
      if (listen(sock, 5) != 0)
      {
         syslog(LOG_DAEMON | LOG_ERR, "Failed to listen to port %d : %s", port, strerror(errno));
         free_config();
         crelay_free_static_mem() ;
         crelay_close() ;
         exit(EXIT_FAILURE);         
      }
      
      portHttp = sock ;
      
      syslog(LOG_DAEMON | LOG_NOTICE, "HTTP server listening on %s:%d\n", inet_ntoa(iface), port);      

      if (!strcmp(argv[1],"-D"))
      {
         /* Daemonise program (send to background) */
         if (daemon(0, 0) == -1) 
         {
            syslog(LOG_DAEMON | LOG_ERR, "Failed to daemonize: %s", strerror(errno));
            close(sock);
            free_config();
            crelay_free_static_mem() ;
            crelay_close() ;
            exit(EXIT_FAILURE);
         }
         syslog(LOG_DAEMON | LOG_NOTICE, "Program is now running as system daemon");
      }

      /* Init GPIO pins in case they have been configured */
//      crelay_detect_relay_card(com_port, &num_relays, NULL, NULL);
      
      while (1)
      {
         int s;
         
         /* Wait for request from web client */
         global_s = s = accept(sock, NULL, NULL);
         if (s < 0) break;
         
         /* Process request */
         if (new_process_http_request(s) == 1)
         {
            syslog(LOG_DAEMON | LOG_NOTICE, "Program quit by URL");
            close(s);
            global_s = -1 ;
            break ;
         }
         
         close(s);
         global_s = -1 ;
      }
      
      close(sock);
   }
   else
   {
      /*****  Command line mode *****/
      
      if (!strcmp(argv[argn],"-i"))
      {
         /* Detect all cards connected to the system */
         if (crelay_detect_all_relay_cards(&relay_info) == -1)
         {
            printf("No compatible device detected.\n");
            if(geteuid() != 0)       // JLP ADD
            {
               printf("\nWarning: this program is currently not running with root privileges !\n");
               printf("Therefore it might not be able to access your relay card communication port.\n");
               printf("Consider invoking the program from the root account or use \"sudo ...\"\n");
            }
            free(relay_info) ;
            crelay_free_static_mem() ;
            crelay_close() ;
            free_config() ;
            return -1;
         }
         printf("\nDetected relay cards:\n");
         while (relay_info->next != NULL)
         {
            crelay_get_relay_card_name(relay_info->relay_type, cname);
            printf("  #%d\t%s (serial %s)\n", i++ ,cname, relay_info->serial);
            prev_relay_info = relay_info ;   // JLP ADD
            relay_info = relay_info->next;
            free(prev_relay_info) ;          // JLP ADD
         }
         free(relay_info);
         free_config();
         crelay_free_static_mem() ;
         crelay_close() ;
         exit(EXIT_SUCCESS);
      }
   
      if (!strcmp(argv[argn], "-s"))
      {
         if (argc > (argn+1))
         {
            serial = malloc(sizeof(char)*(strlen(argv[argn+1])+1));
            strcpy(serial, argv[argn+1]);
            argn += 2;
         }
         else
         {
            print_usage();
            free_config();
            crelay_free_static_mem() ;
            crelay_close() ;
            exit(EXIT_FAILURE);            
         }
      }

      if (crelay_detect_relay_card(com_port, &num_relays, serial, NULL) == -1)
      {
         printf("No compatible device detected.\n");
         
         if(geteuid() != 0)
         {
            printf("\nWarning: this program is currently not running with root privileges !\n");
            printf("Therefore it might not be able to access your relay card communication port.\n");
            printf("Consider invoking the program from the root account or use \"sudo ...\"\n");
         }
         free(serial);
         free_config();
         crelay_free_static_mem() ;
         crelay_close() ;
         exit(EXIT_FAILURE);
      }

      switch (argc)
      {
         case 2:
         case 4:
            /* GET current relay state */
            if (crelay_get_relay(com_port, atoi(argv[argn]), &rstate, serial) == 0)
               printf("Relay %d is %s\n", atoi(argv[argn]), (rstate==ON)?"on":"off");
            else
            {
               free(serial);
               free_config();
               crelay_free_static_mem() ;
               crelay_close() ;
               exit(EXIT_FAILURE);
            }
            break;
            
         case 3:
         case 5:
            /* SET new relay state */
            if (!strcmp(argv[argn+1],"on") || !strcmp(argv[argn+1],"ON"))
               err = crelay_set_relay(com_port, atoi(argv[argn]), ON, serial);
            else if (!strcmp(argv[argn+1],"off") || !strcmp(argv[argn+1],"OFF"))
               err = crelay_set_relay(com_port, atoi(argv[argn]), OFF, serial);
            else 
            {
               print_usage();
               free(serial);
               free_config();
               crelay_free_static_mem() ;
               crelay_close() ;
               exit(EXIT_FAILURE);
            }
            if (err)
            {
               free(serial);
               free_config();
               crelay_free_static_mem() ;
               crelay_close() ;
               exit(EXIT_FAILURE);
            }
            break;
            
         default:
            print_usage();
      }
   }
   
   free(serial);
   free_config();
   crelay_free_static_mem() ;
   crelay_close() ;
   exit(EXIT_SUCCESS);
}
