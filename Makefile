# jdwm - jeff's dynamic window manager
# See LICENSE file for copyright and license details.

#######################################################################################
#  Big thanks to FTLabs (https://github.com/FT-Labs) for help with the build dir and  #
#  thanks much to UtkarshVerma (https://github.com/UtkarshVerma) who is the creator   #
#  of dwmblocks-async, who's Makefile I shamelessly pulled and learned from.          #
#######################################################################################

# jdwm version
VERSION = 0.1

# paths
PREFIX = /usr/local/bin

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# Xinerama, comment if you don't want it
XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# freetype
FREETYPELIBS = -lfontconfig -lXft
FREETYPEINC = /usr/include/freetype2

# OpenBSD (uncomment)
#FREETYPEINC = ${X11INC}/freetype2

# yajl
YAJLLIBS = -lyajl
YAJLINC = /usr/include/yajl

# includes and libs
INCS = -I${X11INC} -I${FREETYPEINC} -I${JDWM_SRC_DIR} -I${JDWM_CONFIG_DIR} -I${JDWM_THEMES_DIR} -I${JDWM_RESOURCES_DIR} -I${YAJLINC}
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lXrender -lImlib2 ${YAJLLIBS}

# flags
# DEBUGFLAG = -ggdb # Debug flag to allow you to debug dwm with gdb
DEBUGFLAG = -g  # Default flag, use if you are not debugging dwm

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\" -DUSERHOMEDIR=\"${HOME}\" ${XINERAMAFLAGS}
CFLAGS   = ${DEBUGFLAG} -std=c99 -pedantic -Wall -O3 -Wno-unused-function -Wno-deprecated-declarations -flto=auto -mtune=native -march=native ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
CC = cc

# jdwm dirs
JDWM_SRC_DIR = ${CURDIR}/dwm
JDWM_BUILD_DIR = ${JDWM_SRC_DIR}/build
JDWM_CONFIG_DIR = ${JDWM_SRC_DIR}/config
JDWM_RESOURCES_DIR = ${JDWM_SRC_DIR}/resources
JDWM_THEMES_DIR = ${JDWM_SRC_DIR}/themes
JDWM_SCRIPTS_DIR = ${JDWM_SRC_DIR}/scripts

JDWM_SRC := drw.c jdwm.c util.c
JDWM_OBJ := ${addprefix ${JDWM_BUILD_DIR}/,${JDWM_SRC:.c=.o}}

# dwmblocks dirs
BLOCKS_SRC_DIR = ${CURDIR}/dwmblocks
BLOCKS_BUILD_DIR = ${BLOCKS_SRC_DIR}/build
BLOCKS_CONFIG_DIR = ${BLOCKS_SRC_DIR}/config
BLOCKS_RESOURCES_DIR = ${BLOCKS_SRC_DIR}/resources
BLOCKS_SCRIPTS_DIR = ${BLOCKS_SRC_DIR}/scripts

BLOCKS_SRC := dwmblocks.c
BLOCKS_OBJ := ${addprefix ${BLOCKS_BUILD_DIR}/,${BLOCKS_SRC:.c=.o}}

# rofi diirs
ROFI_DIR = ${CURDIR}/rofi
ROFI_SCRIPTS_DIR = ${ROFI_DIR}/scripts
ROFI_THEMES_DIR = ${ROFI_DIR}/themes

# 0 = pretty, 1 = raw make output
# To run from cli with VERBOSE of 1 just do this:
# `make VERBOSE=1 command_to_run`
VERBOSE := 0

# Print format for prettier output
PRINTF := @printf "%-21s  |  %s\n"
ifeq ($(VERBOSE), 0)
	Q := @
endif

.DEFAULT_GOAL := all

${JDWM_BUILD_DIR}/%.o: ${JDWM_SRC_DIR}/%.c | ${JDWM_BUILD_DIR}
	$(PRINTF) "Compile jdwm source      (${CC}) " $@
	$Q${CC} -c ${CFLAGS} $< -o $@

${JDWM_BUILD_DIR}:
	$(PRINTF) "Make jdwm build directory " $@
	$Qmkdir -p ${JDWM_BUILD_DIR}

${JDWM_OBJ}: ${JDWM_CONFIG_DIR}/config.h ${JDWM_CONFIG_DIR}/binds.h ${JDWM_CONFIG_DIR}/autorun.h ${JDWM_CONFIG_DIR}/keydefs.h

${JDWM_CONFIG_DIR}/config.h:
	$(PRINTF) "Copy jdwm default config      " "cp config.def.h -> config.h"
	$Qcp ${JDWM_CONFIG_DIR}/config.def.h $@
	$Qchmod a=rw $@

${JDWM_CONFIG_DIR}/binds.h:
	$(PRINTF) "Copy jdwm default binds       " "cp binds.def.h -> binds.h"
	$Qcp ${JDWM_CONFIG_DIR}/binds.def.h $@
	$Qchmod a=rw $@

${JDWM_CONFIG_DIR}/autorun.h:
	$(PRINTF) "Copy jdwm default autorun     " "cp autorun.def.h -> autorun.h"
	$Qcp ${JDWM_CONFIG_DIR}/autorun.def.h $@
	$Qchmod a=rw $@

${JDWM_CONFIG_DIR}/keydefs.h:
	$(PRINTF) "Copy jdwm default keydefs     " "cp keydefs.def.h -> keydefs.h"
	$Qcp ${JDWM_CONFIG_DIR}/keydefs.def.h $@
	$Qchmod a=rw $@

${BLOCKS_BUILD_DIR}/%.o: ${BLOCKS_SRC_DIR}/%.c | ${BLOCKS_BUILD_DIR}
	$(PRINTF) "Compile dwmblocks source (${CC}) " $@
	$Q${CC} -c ${CFLAGS} $< -o $@

${BLOCKS_BUILD_DIR}:
	$(PRINTF) "Make blocks build directory   " $@
	$Qmkdir -p ${BLOCKS_BUILD_DIR}

${BLOCKS_OBJ}: ${BLOCKS_CONFIG_DIR}/blocks.h

${BLOCKS_CONFIG_DIR}/blocks.h:
	$(PRINTF) "Copy dwmblocks default config " "cp blocks.def.h -> blocks.h"
	$Qcp ${BLOCKS_CONFIG_DIR}/blocks.def.h $@
	$Qchmod a=rw $@

jdwm: ${JDWM_OBJ} ${JDWM_BUILD_DIR}
	@if [ $(shell id -u) = 0 ]; then echo -e "\nMakefile needs to be run as non-root to collect $HOME bash variable. Cancelling make...\n"; exit 1; fi
	$(PRINTF) "Linking jdwm source      (${CC}) " ${JDWM_BUILD_DIR}/jdwm
	$Q${CC} -o ${JDWM_BUILD_DIR}/$@ ${JDWM_OBJ} ${LDFLAGS}

dwm-msg: ${JDWM_BUILD_DIR}/dwm-msg.o
	$(PRINTF) "Linking dwm-msg source   (${CC}) " ${JDWM_BUILD_DIR}/$@
	$Q${CC} -o ${JDWM_BUILD_DIR}/$@ $< ${LDFLAGS}

dwmblocks: ${BLOCKS_OBJ} ${BLOCKS_BUILD_DIR}
	$(PRINTF) "Linking dwmblocks source (${CC}) " ${JDWM_BUILD_DIR}/dwmblocks
	$Q${CC} -o ${BLOCKS_BUILD_DIR}/$@ ${BLOCKS_OBJ} ${LDFLAGS}

all: jdwm dwm-msg dwmblocks

jdwm_clean:
	$(PRINTF) "Clean build dir of jdwm       " $(JDWM_BUILD_DIR)
	$Qrm -f ${JDWM_BUILD_DIR}/jdwm ${JDWM_OBJ}

dwm-msg_clean:
	$(PRINTF) "Clean build dir of dwm-msg    " $(JDWM_BUILD_DIR)
	$Qrm -f ${JDWM_BUILD_DIR}/dwm-msg ${JDWM_BUILD_DIR}/dwm-msg.o

dwmblocks_clean:
	$(PRINTF) "Clean build dir of dwmblocks  " $(BLOCKS_BUILD_DIR)
	$Qrm -f ${BLOCKS_BUILD_DIR}/dwmblocks ${BLOCKS_OBJ}

clean: jdwm_clean dwm-msg_clean dwmblocks_clean

jdwm_install: jdwm
	$(PRINTF) "Install jdwm binary           " ${DESTDIR}${PREFIX}/jdwm
	$Qsudo mkdir -p ${DESTDIR}${PREFIX}
	$Qsudo cp -f ${JDWM_BUILD_DIR}/jdwm ${DESTDIR}${PREFIX}
	$Qsudo chmod 755 ${DESTDIR}${PREFIX}/jdwm

dwm-msg_install: dwm-msg
	$(PRINTF) "Install dwm-msg binary        " ${DESTDIR}${PREFIX}/dwm-msg
	$Qsudo mkdir -p ${DESTDIR}${PREFIX}
	$Qsudo cp -f ${JDWM_BUILD_DIR}/dwm-msg ${DESTDIR}${PREFIX}
	$Qsudo chmod 755 ${DESTDIR}${PREFIX}/dwm-msg

dwmblocks_install: dwmblocks
	$(PRINTF) "Install dwmblocks binary      " ${DESTDIR}${PREFIX}/dwmblocks
	$Qsudo mkdir -p ${DESTDIR}${PREFIX}
	$Qsudo cp -f ${BLOCKS_BUILD_DIR}/dwmblocks ${DESTDIR}${PREFIX}
	$Qsudo chmod 755 ${DESTDIR}${PREFIX}/dwmblocks

install: jdwm_install dwm-msg_install dwmblocks_install

jdwm_uninstall:
	$(PRINTF) "Removing jdwm bin file    " ${DESTDIR}${PREFIX}/jdwm
	$Qsudo rm -f ${DESTDIR}${PREFIX}/jdwm

dwm-msg_uninstall:
	$(PRINTF) "Removing dwm-msg bin file     " ${DESTDIR}${PREFIX}/dwm-msg
	$Qsudo rm -f ${DESTDIR}${PREFIX}/dwm-msg

dwmblocks_uninstall:
	$(PRINTF) "Removing dwmblocks bin file   " ${DESTDIR}${PREFIX}/dwmblocks
	$Qsudo rm -f ${DESTDIR}${PREFIX}/dwmblocks

uninstall: jdwm_uninstall dwm-msg_uninstall dwmblocks_uninstall

.PHONY: all jdwm_clean dwm-msg_clean blocks_clean clean jdwm_install dwm-msg_install dwmblocks_install install jdwm_uninstall dwm-msg_uninstall dwmblocks_uninstall uninstall