##########################################################################
#
# REMOTE.MAK linux.mak - Makefile for Remote Execution plugin.
#
# Copyright (C) 2005 IBM Corporation
#
##########################################################################
DIR = client/uxlite/remoteux/linux

VER_FILEDESCRIPTION_STR = "$(FDR_PRODUCTNAME)" " Build ReCBB for linux"

include $(FDR_ROOT)/BUILD/config.mak

EXTRA_INCLUDES = -I $(FDR_ROOT)/src/dsa/client/uxlite/remoteux/linux \
		 		 -I $(FDR_ROOT)/src/dsa/client/uxlite/remoteux/include
		 
ifeq ($(FDR_OS), linux)
EXTRA_LIBRARIES = -lutil
DOMKDIR = mkdir-linux
DOCOPY = copy-linux
MAIN = all
endif

IMAGE = $(FDR_BIN)/image
IMAGEFULL = $(FDR_BIN)/imagefull

LIBRARY = remoteux

SOURCES = RemoteInterface.cpp remoteux.cpp
			
master: $(DOMKDIR) $(MAIN) $(DOCOPY)

include $(ROOT)/BUILD/shared-library.mak

mkdir-linux:
	-$(MKDIRHIER) $(FDR_ROOT)/$(FDR_PLATFORM)/obj/$(DIR)/

copy-linux:
	-$(MKDIRHIER) $(IMAGE)
	-$(MKDIRHIER) $(IMAGEFULL)
	$(COPY) $(LIB_DIR)/libremoteux$(LIB_SUFFIX) $(IMAGE)
	$(COPY) $(LIB_DIR)/libremoteux$(LIB_SUFFIX) $(IMAGEFULL)
