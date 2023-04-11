# Makefile for /path/to/final/

DIR1 := /media/psf/pd/packet-mutation
DIR2 := /media/psf/pd/packetdrill/gtests/net/packetdrill
DIR3 := /media/psf/pd/rtos-bridge
TAP_INTERFACE_NAME := tun0

.PHONY: all run

all:
	cd $(DIR1) && $(MAKE)
	cd $(DIR2) && $(MAKE)
	cd $(DIR3) && $(MAKE)

run:
	sudo TAP_INTERFACE_NAME=$(TAP_INTERFACE_NAME) \
		$(DIR2)/packetdrill \
		--so_filename=$(DIR3)/libfreertos-bridge.so \
		--fm_filename=$(DIR1)/libmutation-interface.so \
		$(ARGS)
