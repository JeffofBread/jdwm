# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

#######################################################################################
#  Big thanks to FTLabs (https://github.com/FT-Labs) for help with the build dir and  #
#  thanks much to UtkarshVerma (https://github.com/UtkarshVerma) who is the creator   #
#  of dwmblocks-async, whos Makefile I shamelessly pulled and learned from.           #
#######################################################################################

# dwm version
VERSION = 6.4

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

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
#MANPREFIX = ${PREFIX}/man

# yajl
YAJLLIBS = -lyajl
YAJLINC = /usr/include/yajl

# includes and libs
INCS = -I${X11INC} -I${FREETYPEINC} -I${DWM_SRC_DIR} -I${DWM_CONFIG_DIR} -I${DWM_THEMES_DIR} -I${DWM_RESOURCES_DIR} -I${YAJLINC}
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lXrender -lImlib2 ${YAJLLIBS}

# flags
# DEBUGFLAG = -ggdb # Debug flag to allow you to debug dwm with gdb
DEBUGFLAG = -g  # Default flag, use if you are not debugging dwm

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS   = ${DEBUGFLAG} -std=c99 -pedantic -Wall -O0 -Wno-deprecated-declarations -Wno-implicit-function-declaration ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
CC = cc

# jeff_dwm dirs
DWM_SRC_DIR = ${CURDIR}/dwmsrc
DWM_BUILD_DIR = ${DWM_SRC_DIR}/build
DWM_CONFIG_DIR = ${DWM_SRC_DIR}/config
DWM_RESOURCES_DIR = ${DWM_SRC_DIR}/resources
DWM_THEMES_DIR = ${DWM_SRC_DIR}/themes
DWM_SCRIPTS_DIR = ${DWM_SRC_DIR}/scripts

DWM_SRC := drw.c dwm.c util.c
DWM_OBJ := ${addprefix ${DWM_BUILD_DIR}/,${DWM_SRC:.c=.o}}

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
all: dwm-clean blocks-clean dwm dwm-msg dwmblocks
else
all: options dwm-clean blocks-clean dwm dwm-msg dwmblocks
endif

options:
	@echo dwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${DWM_BUILD_DIR}/%.o: ${DWM_SRC_DIR}/%.c | ${DWM_BUILD_DIR}
	$(PRINTF) "Compile jeff_dwm source  (${CC}) " $@
	$Q${CC} -c ${CFLAGS} $< -o $@

${DWM_BUILD_DIR}:
	$(PRINTF) "Make jeff_dwm build directory " $@
	$Qmkdir -p ${DWM_BUILD_DIR}

${DWM_OBJ}: ${DWM_CONFIG_DIR}/config.h ${DWM_CONFIG_DIR}/binds.h ${DWM_CONFIG_DIR}/autorun.h ${DWM_CONFIG_DIR}/keydefs.h

${DWM_CONFIG_DIR}/config.h:
	$(PRINTF) "Copy jeff_dwm default config  " "cp config.def.h -> config.h"
	$Qcp ${DWM_CONFIG_DIR}/config.def.h $@
	$Qchmod a=rw $@

${DWM_CONFIG_DIR}/binds.h:
	$(PRINTF) "Copy jeff_dwm default binds   " "cp binds.def.h -> binds.h"
	$Qcp ${DWM_CONFIG_DIR}/binds.def.h $@
	$Qchmod a=rw $@

${DWM_CONFIG_DIR}/autorun.h:
	$(PRINTF) "Copy jeff_dwm default autorun " "cp autorun.def.h -> autorun.h"
	$Qcp ${DWM_CONFIG_DIR}/autorun.def.h $@
	$Qchmod a=rw $@

${DWM_CONFIG_DIR}/keydefs.h:
	$(PRINTF) "Copy jeff_dwm default keydefs " "cp keydefs.def.h -> keydefs.h"
	$Qcp ${DWM_CONFIG_DIR}/keydefs.def.h $@
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

dwm: ${DWM_OBJ} ${DWM_BUILD_DIR}
	$(PRINTF) "Linking jeff_dwm source  (${CC}) " ${DWM_BUILD_DIR}/dwm
	$Q${CC} -o ${DWM_BUILD_DIR}/$@ ${DWM_OBJ} ${LDFLAGS}

dwm-msg: ${DWM_BUILD_DIR}/dwm-msg.o
	$(PRINTF) "Linking dwm-msg source   (${CC}) " ${DWM_BUILD_DIR}/$@
	$Q${CC} -o ${DWM_BUILD_DIR}/$@ $< ${LDFLAGS}

dwmblocks: ${BLOCKS_OBJ} ${BLOCKS_BUILD_DIR}
	$(PRINTF) "Linking dwmblocks source (${CC}) " ${DWM_BUILD_DIR}/dwmblocks
	$Q${CC} -o ${BLOCKS_BUILD_DIR}/$@ ${BLOCKS_OBJ} ${LDFLAGS}

dwm-clean:
	$(PRINTF) "Clean jeff_dwm build directory" $(DWM_BUILD_DIR)
	$Qrm -f ${DWM_BUILD_DIR}/dwm ${DWM_BUILD_DIR}/dwm-msg ${DWM_OBJ} dwm-${VERSION}.tar.gz

blocks-clean:
	$(PRINTF) "Clean blocks build directory  " $(BLOCKS_BUILD_DIR)
	$Qrm -f ${BLOCKS_BUILD_DIR}/dwmblocks

install: all
	$(PRINTF) "Install jeff_dwm binary       " ${DESTDIR}${PREFIX}/bin/dwm
	$Qmkdir -p ${DESTDIR}${PREFIX}/bin
	$Qcp -f ${DWM_BUILD_DIR}/dwm ${DESTDIR}${PREFIX}/bin
	$Qchmod 755 ${DESTDIR}${PREFIX}/bin/dwm
	$(PRINTF) "Install dwm-msg binary        " ${DESTDIR}${PREFIX}/bin/dwm-msg
	$Qcp -f ${DWM_BUILD_DIR}/dwm-msg ${DESTDIR}${PREFIX}/bin
	$Qchmod 755 ${DESTDIR}${PREFIX}/bin/dwm-msg
	$Qrm -f ${DESTDIR}${PREFIX}/bin/jeff_dwm-recompile.sh
	$Qecho -e '#!/bin/sh\nkitty sh -c "cd ${CURDIR} && sudo make install && read -rp \"\n\033[32;1mBuild and install completed successfully. Press ENTER to exit terminal...\033[0m\" || read -rp \"\n\n\033[31;1mBuild Failed. Press ENTER to exit terminal...\033[0m\""' > ${DESTDIR}${PREFIX}/bin/jeff_dwm-recompile.sh
	$Qchmod 755 ${DESTDIR}${PREFIX}/bin/jeff_dwm-recompile.sh
	$(PRINTF) "Install jeff_dwm man page     " ${DESTDIR}${MANPREFIX}/man1/dwm.1
	$Qmkdir -p ${DESTDIR}${MANPREFIX}/man1
	$Qsed "s/VERSION/${VERSION}/g" < ${DWM_RESOURCES_DIR}/dwm.1 > ${DESTDIR}${MANPREFIX}/man1/dwm.1
	$Qchmod 644 ${DESTDIR}${MANPREFIX}/man1/dwm.1
	$(PRINTF) "Install jeff_dwm .desktop file" /usr/share/xsessions/jeff_dwm.desktop
	$Qcp -f ${DWM_RESOURCES_DIR}/jeff_dwm.desktop /usr/share/xsessions
	$Qecho "Icon=${CURDIR}/${DWM_RESOURCES_DIR}/dwm.png" >> /usr/share/xsessions/jeff_dwm.desktop
	$(PRINTF) "Install dwmblocks binary      " ${DESTDIR}${PREFIX}/bin/dwmblocks
	$Qcp -f ${BLOCKS_BUILD_DIR}/dwmblocks ${DESTDIR}${PREFIX}/bin
	$Qchmod 755 ${DESTDIR}${PREFIX}/bin/dwmblocks

uninstall:
	$(PRINTF) "Remove all files" "dwm, dwm_swallow.sh, jeff_dwm_run.sh, jeff_dwm.desktop, dwm.1, dwmblocks, rofi_layoutmenu"
	$Qrm -f ${DESTDIR}${PREFIX}/bin/dwm\
		${DESTDIR}${MANPREFIX}/bin/dwm_swallow.sh\
		${DESTDIR}${MANPREFIX}/bin/jeff_dwm_run.sh\
		${DESTDIR}${MANPREFIX}/usr/share/xsessions/jeff_dwm.desktop\
		${DESTDIR}${MANPREFIX}/bin/jeff_dwm-recompile.sh\
		${DESTDIR}${MANPREFIX}/man1/dwm.1\
		${DESTDIR}${PREFIX}/bin/dwmblocks\
		${DESTDIR}${MANPREFIX}/bin/rofi_layoutmenu.sh

.PHONY: all options blocks-clean dwm-clean install uninstall