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
INCS = -I${X11INC} -I${FREETYPEINC} -I${JDWM_CONFIG_DIR} -I${YAJLINC}
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lXrender -lImlib2 ${YAJLLIBS}

# flags
# DEBUGFLAG = -ggdb # Debug flag to allow you to debug dwm with gdb
DEBUGFLAG = -g  # Default flag, use if you are not debugging dwm

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS} ${XINERAMAFLAGS}
CFLAGS   = ${DEBUGFLAG} -std=c99 -pedantic -Wall -O3 -Wno-unused-function -Wno-deprecated-declarations -flto=auto -mtune=native -march=native ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
CC = cc

# jdwm dirs
JDWM_BUILD_DIR = ${CURDIR}/build
JDWM_CONFIG_DIR = ${CURDIR}/config
JDWM_RESOURCES_DIR = ${CURDIR}/resources

JDWM_SRC := drw.c jdwm.c util.c
JDWM_OBJ := ${addprefix ${JDWM_BUILD_DIR}/,${JDWM_SRC:.c=.o}}

.DEFAULT_GOAL := all

${JDWM_BUILD_DIR}/%.o: %.c | ${JDWM_BUILD_DIR}
	${CC} -c ${CFLAGS} $< -o $@

${JDWM_BUILD_DIR}:
	mkdir -p ${JDWM_BUILD_DIR}

${JDWM_OBJ}: ${JDWM_CONFIG_DIR}/config.h ${JDWM_CONFIG_DIR}/binds.h ${JDWM_CONFIG_DIR}/autorun.h ${JDWM_CONFIG_DIR}/keydefs.h

${JDWM_CONFIG_DIR}/config.h:
	cp ${JDWM_CONFIG_DIR}/config.def.h $@
	chmod a=rw $@

${JDWM_CONFIG_DIR}/binds.h:
	cp ${JDWM_CONFIG_DIR}/binds.def.h $@
	chmod a=rw $@

${JDWM_CONFIG_DIR}/autorun.h:
	cp ${JDWM_CONFIG_DIR}/autorun.def.h $@
	chmod a=rw $@

${JDWM_CONFIG_DIR}/keydefs.h:
	cp ${JDWM_CONFIG_DIR}/keydefs.def.h $@
	chmod a=rw $@

jdwm: ${JDWM_OBJ} ${JDWM_BUILD_DIR}
	${CC} -o ${JDWM_BUILD_DIR}/$@ ${JDWM_OBJ} ${LDFLAGS}

jdwm-msg: ${JDWM_BUILD_DIR}/jdwm-msg.o
	${CC} -o ${JDWM_BUILD_DIR}/$@ $< ${LDFLAGS}

all: jdwm jdwm-msg 

jdwm_clean:
	rm -f ${JDWM_BUILD_DIR}/jdwm ${JDWM_OBJ}

jdwm-msg_clean:
	rm -f ${JDWM_BUILD_DIR}/jdwm-msg ${JDWM_BUILD_DIR}/jdwm-msg.o

clean: jdwm_clean jdwm-msg_clean 

jdwm_install: jdwm
	sudo mkdir -p ${DESTDIR}${PREFIX}
	sudo cp -f ${JDWM_BUILD_DIR}/jdwm ${DESTDIR}${PREFIX}
	sudo chmod 755 ${DESTDIR}${PREFIX}/jdwm

jdwm-msg_install: jdwm-msg
	sudo mkdir -p ${DESTDIR}${PREFIX}
	sudo cp -f ${JDWM_BUILD_DIR}/jdwm-msg ${DESTDIR}${PREFIX}
	sudo chmod 755 ${DESTDIR}${PREFIX}/jdwm-msg

install: jdwm_install jdwm-msg_install
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < ${JDWM_RESOURCES_DIR}/jdwm.1 > ${DESTDIR}${MANPREFIX}/man1/jdwm.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/dwm.1
	cp -f ${JDWM_RESOURCES_DIR}/jdwm.desktop /usr/share/xsessions
	echo "Icon=${CURDIR}/${JDWM_RESOURCES_DIR}/dwm.png" >> /usr/share/xsessions/jdwm.desktop

jdwm_uninstall:
	sudo rm -f ${DESTDIR}${PREFIX}/jdwm

jdwm-msg_uninstall:
	sudo rm -f ${DESTDIR}${PREFIX}/jdwm-msg

uninstall: jdwm_uninstall jdwm-msg_uninstall
	sudo rm -f ${DESTDIR}${MANPREFIX}/man1/jdwm.1 \
			   /usr/share/xsessions/jdwm.desktop

.PHONY: all jdwm_clean jdwm-msg_clean clean jdwm_install jdwm-msg_install install jdwm_uninstall jdwm-msg_uninstall uninstall