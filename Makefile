# ;
# Makefile:
###############################################################################
#
# crelay project makefile
#
###############################################################################

DESTDIR=/usr
PREFIX=/local

BIN=crelay

# By default enable all drivers
# To exclude a specific driver add DRV_<XYZ>=n to the make command
DRV_CONRAD	= n
DRV_SAINSMART	= y
DRV_SAINSMART16	= n
DRV_SAINSMART16_CH340 = y
DRV_HIDAPI	= n
CONFBASE = "NOCONF"
CONF = $(CONFBASE)

DEBUG	= -g -O0
#DEBUG	= -O2
CC	= gcc
INCLUDE	= -I.
DEFS	= -D_GNU_SOURCE
CFLAGS	= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Winline $(INCLUDE) -pipe -fPIC

# Main source files (don't change)
#########################################
SRC	= $(BIN).c
SRC	+= relay_drv.c
SRC	+= config.c

# Relay card specific driver source files
#########################################

SRC	+= relay_drv_gpio.c

# Include only needed drivers and libraries
ifeq ($(DRV_CONRAD), y)
SRC	+= relay_drv_conrad.c
LIBS	+= -lusb-1.0
OPTS	+= -DDRV_CONRAD
endif
ifeq ($(DRV_SAINSMART), y)
SRC	+= relay_drv_sainsmart.c
LIBS	+= -lftdi -lusb-1.0
OPTS	+= -DDRV_SAINSMART
endif
ifeq ($(DRV_SAINSMART16), y)
SRC	+= relay_drv_sainsmart16.c
LIBS	+= -lhidapi-libusb
OPTS	+= -DDRV_SAINSMART16
endif
ifeq ($(DRV_SAINSMART16_CH340), y)
SRC	+= relay_drv_sainsmart16_CH340.c
USBFLAGS = `libusb-config --cflags`
USBLIBS = `libusb-config --libs`
OPTS	+= -DDRV_SAINSMART16_CH340
endif
ifeq ($(DRV_HIDAPI), y)
SRC	+= relay_drv_hidapi.c
LIBS	+= -lhidapi-libusb
OPTS	+= -DDRV_HIDAPI
endif

OBJ	= $(SRC:.c=.o)

all:	$(BIN)

$(BIN):	$(OBJ)
	@echo "[Link $(BIN)] with libs $(LIBS)"
	@$(CC) -o $(BIN) $(OBJ) $(LDFLAGS) $(LIBS) $(USBLIBS)

.c.o:
	@echo "[Compile $<]"
	@$(CC) -c $(CFLAGS) $(USBFLAGS) $< -o $@  $(OPTS)

.PHONEY:	clean
clean:
	@echo "[Clean]"
	@rm -f $(OBJ) $(BIN)

.PHONEY:	install
install:	$(BIN)
	@echo "[Install binary]"
	@install -m 0755 -d		$(DESTDIR)$(PREFIX)/bin
	@install -m 0755 $(BIN)		$(DESTDIR)$(PREFIX)/bin/$(BIN)
	@if [ "$(CONF)" = $(CONFBASE) ]; then \
	echo "Conf actuelle conservée" ; \
	else \
	cp $(CONF) /etc/crelay.conf ; \
	echo "Conf installée : $(CONF)" ; \
	fi ;

