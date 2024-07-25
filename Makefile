# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

#######################################################################################
#  Big thanks to FTLabs (https://github.com/FT-Labs) for help with the build dir and  #
#  thanks much to UtkarshVerma (https://github.com/UtkarshVerma) who is the creator   #
#  of dwmblocks-async, whos Makefile I shamelessly pulled and learned from.           #
#######################################################################################

# jeff_dwm version
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
INCS = -I${X11INC} -I${FREETYPEINC} -I${JEFF_DWM_SRC_DIR} -I${JEFF_DWM_CONFIG_DIR} -I${JEFF_DWM_THEMES_DIR} -I${JEFF_DWM_RESOURCES_DIR} -I${YAJLINC}
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lXrender -lImlib2 ${YAJLLIBS}

# flags
# DEBUGFLAG = -ggdb # Debug flag to allow you to debug dwm with gdb
DEBUGFLAG = -g  # Default flag, use if you are not debugging dwm

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS   = ${DEBUGFLAG} -std=c99 -pedantic -Wall -O3 -Wno-unused-function -Wno-deprecated-declarations -march=native ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
CC = cc

# jeff_dwm dirs
JEFF_DWM_SRC_DIR = ${CURDIR}/dwmsrc
JEFF_DWM_BUILD_DIR = ${JEFF_DWM_SRC_DIR}/build
JEFF_DWM_CONFIG_DIR = ${JEFF_DWM_SRC_DIR}/config
JEFF_DWM_RESOURCES_DIR = ${JEFF_DWM_SRC_DIR}/resources
JEFF_DWM_THEMES_DIR = ${JEFF_DWM_SRC_DIR}/themes
JEFF_DWM_SCRIPTS_DIR = ${JEFF_DWM_SRC_DIR}/scripts

JEFF_DWM_SRC := drw.c jeff_dwm.c util.c
JEFF_DWM_OBJ := ${addprefix ${JEFF_DWM_BUILD_DIR}/,${JEFF_DWM_SRC:.c=.o}}

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
VERBOSE := 0

# Print format for prettier output
PRINTF := @printf "%-21s  |  %s\n"
ifeq ($(VERBOSE), 0)
	Q := @
endif

all: jeff_dwm-clean blocks-clean jeff_dwm dwm-msg dwmblocks

${JEFF_DWM_BUILD_DIR}/%.o: ${JEFF_DWM_SRC_DIR}/%.c | ${JEFF_DWM_BUILD_DIR}
	$(PRINTF) "Compile jeff_dwm source  (${CC}) " $@
	$Q${CC} -c ${CFLAGS} $< -o $@

${JEFF_DWM_BUILD_DIR}:
	$(PRINTF) "Make jeff_dwm build directory " $@
	$Qmkdir -p ${JEFF_DWM_BUILD_DIR}

${JEFF_DWM_OBJ}: ${JEFF_DWM_CONFIG_DIR}/config.h ${JEFF_DWM_CONFIG_DIR}/binds.h ${JEFF_DWM_CONFIG_DIR}/autorun.h ${JEFF_DWM_CONFIG_DIR}/keydefs.h

${JEFF_DWM_CONFIG_DIR}/config.h:
	$(PRINTF) "Copy jeff_dwm default config  " "cp config.def.h -> config.h"
	$Qcp ${JEFF_DWM_CONFIG_DIR}/config.def.h $@
	$Qchmod a=rw $@

${JEFF_DWM_CONFIG_DIR}/binds.h:
	$(PRINTF) "Copy jeff_dwm default binds   " "cp binds.def.h -> binds.h"
	$Qcp ${JEFF_DWM_CONFIG_DIR}/binds.def.h $@
	$Qchmod a=rw $@

${JEFF_DWM_CONFIG_DIR}/autorun.h:
	$(PRINTF) "Copy jeff_dwm default autorun " "cp autorun.def.h -> autorun.h"
	$Qcp ${JEFF_DWM_CONFIG_DIR}/autorun.def.h $@
	$Qchmod a=rw $@

${JEFF_DWM_CONFIG_DIR}/keydefs.h:
	$(PRINTF) "Copy jeff_dwm default keydefs " "cp keydefs.def.h -> keydefs.h"
	$Qcp ${JEFF_DWM_CONFIG_DIR}/keydefs.def.h $@
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

jeff_dwm: ${JEFF_DWM_OBJ} ${JEFF_DWM_BUILD_DIR}
	$(PRINTF) "Linking jeff_dwm source  (${CC}) " ${JEFF_DWM_BUILD_DIR}/jeff_dwm
	$Q${CC} -o ${JEFF_DWM_BUILD_DIR}/$@ ${JEFF_DWM_OBJ} ${LDFLAGS}

dwm-msg: ${JEFF_DWM_BUILD_DIR}/dwm-msg.o
	$(PRINTF) "Linking dwm-msg source   (${CC}) " ${JEFF_DWM_BUILD_DIR}/$@
	$Q${CC} -o ${JEFF_DWM_BUILD_DIR}/$@ $< ${LDFLAGS}

dwmblocks: ${BLOCKS_OBJ} ${BLOCKS_BUILD_DIR}
	$(PRINTF) "Linking dwmblocks source (${CC}) " ${JEFF_DWM_BUILD_DIR}/dwmblocks
	$Q${CC} -o ${BLOCKS_BUILD_DIR}/$@ ${BLOCKS_OBJ} ${LDFLAGS}

jeff_dwm-clean:
	$(PRINTF) "Clean jeff_dwm build directory" $(JEFF_DWM_BUILD_DIR)
	$Qrm -f ${JEFF_DWM_BUILD_DIR}/jeff_dwm ${JEFF_DWM_BUILD_DIR}/dwm-msg ${JEFF_DWM_OBJ}

blocks-clean:
	$(PRINTF) "Clean blocks build directory  " $(BLOCKS_BUILD_DIR)
	$Qrm -f ${BLOCKS_BUILD_DIR}/dwmblocks

install: all

# jeff_dwm
	$(PRINTF) "Install jeff_dwm binary       " ${DESTDIR}${PREFIX}/jeff_dwm
	$Qmkdir -p ${DESTDIR}${PREFIX}
	$Qcp -f ${JEFF_DWM_BUILD_DIR}/jeff_dwm ${DESTDIR}${PREFIX}
	$Qchmod 755 ${DESTDIR}${PREFIX}/jeff_dwm

# dwm-msg
	$(PRINTF) "Install dwm-msg binary        " ${DESTDIR}${PREFIX}/dwm-msg
	$Qcp -f ${JEFF_DWM_BUILD_DIR}/dwm-msg ${DESTDIR}${PREFIX}
	$Qchmod 755 ${DESTDIR}${PREFIX}/dwm-msg

# dwmblocks
	$(PRINTF) "Install dwmblocks binary      " ${DESTDIR}${PREFIX}/dwmblocks
	$Qcp -f ${BLOCKS_BUILD_DIR}/dwmblocks ${DESTDIR}${PREFIX}
	$Qchmod 755 ${DESTDIR}${PREFIX}/dwmblocks

uninstall:
	$(PRINTF) "Removed all jeff_dwm binary files " " jeff_dwm, dwm-msg, dwmblocks"
	$Qrm -f ${DESTDIR}${PREFIX}/jeff_dwm ${DESTDIR}${PREFIX}/dwm-msg ${DESTDIR}${PREFIX}/dwmblocks

.PHONY: all blocks-clean dwm-clean install uninstall