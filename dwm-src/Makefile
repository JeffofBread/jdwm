# dwm - dynamic window manager
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c dwm.c util.c
OBJ = ${SRC:.c=.o}

all: options dwm

options:
	@echo dwm build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	cp config.def.h $@

dwm: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f dwm ${OBJ} dwm-${VERSION}.tar.gz

dist: clean
	mkdir -p dwm-${VERSION}
	cp -R LICENSE Makefile README config.def.h config.mk\
		dwm.1 drw.h util.h ${SRC} dwm.png transient.c dwm-${VERSION}
	tar -cf dwm-${VERSION}.tar dwm-${VERSION}
	gzip dwm-${VERSION}.tar
	rm -rf dwm-${VERSION}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f dwm ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwm
	cp -f dwmswallow ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwmswallow
	sudo ln -sf ${CURDIR}/dwm-scripts/dwm-run.sh /bin
	cp -f dwm-scripts/jeff_dwm.desktop /usr/share/xsessions
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < dwm.1 > ${DESTDIR}${MANPREFIX}/man1/dwm.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/dwm.1
	rm -f /bin/jeff-dwm-recompile.sh
	echo -e '#!/bin/sh\nkitty --hold sh -c "cd ${CURDIR} && sudo make clean install"' > /bin/jeff-dwm-recompile.sh
	chmod 755 /bin/jeff-dwm-recompile.sh
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dwm\
		${DESTDIR}${MANPREFIX}/bin/dwmswallow\
		${DESTDIR}${MANPREFIX}/bin/dwm-run.sh\
		${DESTDIR}${MANPREFIX}/usr/share/xsessions/jeff_dwm.desktop\
		${DESTDIR}${MANPREFIX}/bin/jeff-dwm-recompile.sh\
		${DESTDIR}${MANPREFIX}/man1/dwm.1

.PHONY: all options clean dist install uninstall
