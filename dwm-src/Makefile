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

# includes and libs
INCS = -I${X11INC} -I${FREETYPEINC} -I${CURDIR} -I${CURDIR}/config -I${CURDIR}/themes -I${CURDIR}/resources
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lXrender -lImlib2

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS   = -g -std=c99 -pedantic -Wall -O0 -Wno-deprecated-declarations -Wno-implicit-function-declaration ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
CC = cc

# jeff_dwm dirs
BUILD_DIR := build
CONFIG_DIR := config
RESOURCES_DIR := resources
THEMES_DIR := themes
SCRIPTS_DIR := scripts

SRC := drw.c dwm.c util.c
OBJ := ${addprefix ${BUILD_DIR}/,${SRC:.c=.o}}

# 0 = pretty, 1 = raw make output
VERBOSE := 0

# Print format for prettier output
PRINTF := @printf "%-21s  |  %s\n"
ifeq ($(VERBOSE), 0)
	Q := @
all: dwm
else
all: options dwm
endif

options:
	@echo dwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

${BUILD_DIR}/%.o: %.c | ${BUILD_DIR}
	$(PRINTF) "Compile source (${CC})" $@
	$Q${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}:
	$(PRINTF) "Make build directory" $@
	$Qmkdir -p ${BUILD_DIR}

${OBJ}: ${CONFIG_DIR}/config.h

config/config.h:
	$(PRINTF) "Copy config.def.h" "${CONFIG_DIR}/config.def.h -> ${CONFIG_DIR}/config.h"
	$Qcp ${CONFIG_DIR}/config.def.h $@

dwm: ${OBJ}
	$(PRINTF) "Compile source (${CC})" ${BUILD_DIR}/$@
	$Q${CC} -o ${BUILD_DIR}/$@ ${OBJ} ${LDFLAGS}

clean:
	$(PRINTF) "Clean build directory" /$(BUILD_DIR)
	$Qrm -f ${BUILD_DIR}/dwm ${OBJ} dwm-${VERSION}.tar.gz

install: all
	$(PRINTF) "Install binary" ${DESTDIR}${PREFIX}/bin
	$Qmkdir -p ${DESTDIR}${PREFIX}/bin
	$Qcp -f ${BUILD_DIR}/dwm ${DESTDIR}${PREFIX}/bin
	$Qchmod 755 ${DESTDIR}${PREFIX}/bin/dwm
	$(PRINTF) "Install scripts" ${DESTDIR}${PREFIX}/bin
	$Qcp -f ${SCRIPTS_DIR}/dwmswallow ${DESTDIR}${PREFIX}/bin
	$Qchmod 755 ${DESTDIR}${PREFIX}/bin/dwmswallow
#Alternate method of having the swallow script executed, makes editable without recompile
##	sudo ln -sf ${CURDIR}/${SCRIPTS_DIR}/dwmswallow /bin
	$Qcp -f ${SCRIPTS_DIR}/jeff_dwm-run.sh ${DESTDIR}${PREFIX}/bin
	$Qchmod 755 ${DESTDIR}${PREFIX}/bin/jeff_dwm-run.sh
# Alternate method of having the run script executed, makes editable without recompile
##	sudo ln -sf ${CURDIR}/${SCRIPTS_DIR}/jeff_dwm-run.sh /bin
	$Qrm -f /bin/jeff_dwm-recompile.sh
	$Qecho -e '#!/bin/sh\nkitty sh -c "cd ${CURDIR} && sudo make clean install && read -rp \"\n\033[32;1mBuild and install completed successfully. Press ENTER to exit terminal...\033[0m\" || read -rp \"\n\n\033[31;1mBuild Failed. Press ENTER to exit terminal...\033[0m\""' > /bin/jeff_dwm-recompile.sh
	$Qchmod 755 /bin/jeff_dwm-recompile.sh
	$(PRINTF) "Install .desktop file" /usr/share/xsessions/jeff_dwm.desktop
	$Qcp -f ${SCRIPTS_DIR}/jeff_dwm.desktop /usr/share/xsessions
	$Qecho "Icon=${CURDIR}/${RESOURCES_DIR}/dwm.png" >> /usr/share/xsessions/jeff_dwm.desktop
	$(PRINTF) "Install man page" ${DESTDIR}${MANPREFIX}/man1/dwm.1
	$Qmkdir -p ${DESTDIR}${MANPREFIX}/man1
	$Qsed "s/VERSION/${VERSION}/g" < resources/dwm.1 > ${DESTDIR}${MANPREFIX}/man1/dwm.1
	$Qchmod 644 ${DESTDIR}${MANPREFIX}/man1/dwm.1

uninstall:
	$(PRINTF) "Remove all files" "dwm, dwmswallow,jeff_dwm-run.sh,jeff_dwm.desktop, dwm.1"
	$Qrm -f ${DESTDIR}${PREFIX}/bin/dwm\
		${DESTDIR}${MANPREFIX}/bin/dwmswallow\
		${DESTDIR}${MANPREFIX}/bin/jeff_dwm-run.sh\
		${DESTDIR}${MANPREFIX}/usr/share/xsessions/jeff_dwm.desktop\
		${DESTDIR}${MANPREFIX}/bin/jeff_dwm-recompile.sh\
		${DESTDIR}${MANPREFIX}/man1/dwm.1

.PHONY: all options clean install uninstall