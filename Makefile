# jdwm - jeff's dynamic window manager
# See LICENSE file for copyright and license details.

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
INCS = -I${X11INC} -I${FREETYPEINC} -I${YAJLINC}
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lXrender -lImlib2 ${YAJLLIBS}

# flags
# DEBUGFLAG = -ggdb # Debug flag to allow you to debug dwm with gdb
DEBUGFLAG = -g  # Default flag, use if you are not debugging dwm

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS} ${XINERAMAFLAGS}
CFLAGS   = ${DEBUGFLAG} -std=c99 -pedantic -Wall -O3 -Wno-unused-function -Wno-deprecated-declarations -flto=auto -mtune=native -march=native ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# compiler and linker
CC = cc

JDWM_SRC = drw.c jdwm.c util.c
JDWM_OBJ = ${JDWM_SRC:.c=.o}

all: jdwm jdwm-msg 

.c.o:
	${CC} -c ${CFLAGS} $<

${JDWM_OBJ}: config.h keydefs.h

config.h:
	cp config.def.h $@

keydefs.h:
	cp keydefs.def.h $@

jdwm: ${JDWM_OBJ}
	${CC} -o $@ ${JDWM_OBJ} ${LDFLAGS}

jdwm-msg: jdwm-msg.o
	${CC} -o $@ $< ${LDFLAGS}

jdwm_clean:
	rm -f jdwm ${JDWM_OBJ}

jdwm-msg_clean:
	rm -f jdwm-msg jdwm-msg.o

clean: jdwm_clean jdwm-msg_clean 

jdwm_install: jdwm
	sudo mkdir -p ${DESTDIR}${PREFIX}
	sudo cp -f jdwm ${DESTDIR}${PREFIX}
	sudo chmod 755 ${DESTDIR}${PREFIX}/jdwm

jdwm-msg_install: jdwm-msg
	sudo mkdir -p ${DESTDIR}${PREFIX}
	sudo cp -f jdwm-msg ${DESTDIR}${PREFIX}
	sudo chmod 755 ${DESTDIR}${PREFIX}/jdwm-msg

install: jdwm_install jdwm-msg_install
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < jdwm.1 > ${DESTDIR}${MANPREFIX}/man1/jdwm.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/jdwm.1
	cp -f dwm.desktop /usr/share/xsessions
	echo "Icon=${CURDIR}/dwm.png" >> /usr/share/xsessions/jdwm.desktop

jdwm_uninstall:
	sudo rm -f ${DESTDIR}${PREFIX}/jdwm

jdwm-msg_uninstall:
	sudo rm -f ${DESTDIR}${PREFIX}/jdwm-msg

uninstall: jdwm_uninstall jdwm-msg_uninstall
	sudo rm -f ${DESTDIR}${MANPREFIX}/man1/jdwm.1 \
			   /usr/share/xsessions/jdwm.desktop

.PHONY: all jdwm_clean jdwm-msg_clean clean jdwm_install jdwm-msg_install install jdwm_uninstall jdwm-msg_uninstall uninstall