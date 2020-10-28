#!/bin/bash


# Sends a open/close param command to a USB relay controller
# via its virtual serial port enumerated as Linux /dev/ttyUSB{x} 
# Usage: bash_relay.sh COMMAND TARGET
#        bash_relay.sh open /dev/ttyUSB1
#        bash_relay.sh close /dev/ttyUSB1
#
# Mfg Style: bash_relay.sh TARGET START_ID SWITCH OP_DATA CHECK
#      Open:
#        bash_relay.sh /dev/ttyUSB1 A0 01 01 A2
#     Close:
#        bash_relay.sh /dev/ttyUSB1 A0 01 00 A1

# If you have multiple TARGET plugged into a USB hub (or different OS types):
#    Linux Mint14 : /dev/ttyUSB0
#    Linux Mint14 : /dev/ttyUSB1
#    Linux Mint14 : /dev/ttyUSB2 . . . etc
#        Mac OS X : /dev/tty.usbmodem00007011
# BeagleBone Black: /dev/cu.usbmodem00007011
#  Windows CYGWIN : '\\.\USBSER000'

# Mfg's Command Example:
# Open the USB switch: A0 0101 A2
# Close the USB switch: A0 0100 A1

COMMAND=$1
TARGET=$2
byte() {
  printf "\\x$(printf "%x" $1)"
}

if [ $COMMAND = "open" ] 
then
echo "Simple OPEN . . ."
{
  byte 0xA0
  byte 0X01
  byte 0x01
  byte 0xA2
} > $TARGET
fi

if [ $COMMAND = "close" ] 
then
echo "Simple CLOSE . . ."
{
  byte 0xA0
  byte 0X01
  byte 0x00
  byte 0xA1
} > $TARGET
fi

if [ $COMMAND = "teston" ] 
then
echo "Simple TESTon . . ."
{
  byte 0x3A
  byte 0X46
  byte 0x45
  byte 0x30
  byte 0x35
  byte 0x30
  byte 0x30
  byte 0x30
  byte 0x35
  byte 0x46
  byte 0x46
  byte 0x30
  byte 0x30
  byte 0x46
  byte 0x39
  byte 0x0D
  byte 0x0A
} > $TARGET
fi

if [ $COMMAND = "testoff" ] 
then
echo "Simple TESToff . . ."
{
  byte 0x3A
  byte 0X46
  byte 0x45
  byte 0x30
  byte 0x35
  byte 0x30
  byte 0x30
  byte 0x30
  byte 0x35
  byte 0x30
  byte 0x30
  byte 0x30
  byte 0x30
  byte 0x46
  byte 0x38
  byte 0x0D
  byte 0x0A
} > $TARGET
fi

if [[ $COMMAND != "close" && $COMMAND != "open" && $COMMAND != "teston"  && $COMMAND != "testoff" ]] 
then
TARGET=$1
START_ID=$2
SWITCH=$3
OP_DATA=$4
CHECK=$5
echo "Mfg style $START_ID $SWITCH $OP_DATA $CHECK > $TARGET . . ."
{
  byte 0x$START_ID
  byte 0X$SWITCH
  byte 0x$OP_DATA
  byte 0x$CHECK
} > $TARGET
fi
