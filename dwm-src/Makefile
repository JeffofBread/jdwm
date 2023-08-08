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

${OBJ}: config/config.h config.mk

config/config.h:
	cp config/config.def.h $@

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
# Copy dwmswallow script into bin for dynamic swallow patch and make it executable
	cp -f dwm-scripts/dwmswallow ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/dwmswallow
#Alternate method of having the swallow script executed, makes editable without recompile
##	sudo ln -sf ${CURDIR}/dwm-scripts/dwmswallow /bin

# Copy run script to bin directory and make executable
	cp -f dwm-scripts/jeff_dwm-run.sh ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/jeff_dwm-run.sh
# Alternate method of having the run script executed, makes editable without recompile
##	sudo ln -sf ${CURDIR}/dwm-scripts/jeff_dwm-run.sh /bin

# Copy .desktop file into xsessions and append with icon directory (absolute path to dwm-src/resources/dwm.png)
	cp -f dwm-scripts/jeff_dwm.desktop /usr/share/xsessions
	echo "Icon=${CURDIR}/resources/dwm.png" >> /usr/share/xsessions/jeff_dwm.desktop

	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < resources/dwm.1 > ${DESTDIR}${MANPREFIX}/man1/dwm.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/dwm.1
# Remove old recompile file. Not 100% necessary but protects against changing source code directory
	rm -f /bin/jeff_dwm-recompile.sh

# Create recompile file and make executable. Lazy, easier than copying a file. If you need to change terminal you can, just update "kitty sh -c"
	echo -e '#!/bin/sh\nkitty sh -c "cd ${CURDIR} && sudo make clean install && read -rp \"\n\n\033[32;1mBuild and install completed successfully. Press ENTER to exit terminal...\033[0m\" || read -rp \"\n\n\033[31;1mBuild Failed. Press ENTER to exit terminal...\033[0m\""' > /bin/jeff_dwm-recompile.sh
	chmod 755 /bin/jeff_dwm-recompile.sh

# Lazy way of making a build directory, just moves the binaries after they have been built and copied. 
	mkdir -p ${CURDIR}/build
	mv dwm *.o ${CURDIR}/build
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/dwm\
		${DESTDIR}${MANPREFIX}/bin/dwmswallow\
		${DESTDIR}${MANPREFIX}/bin/jeff_dwm-run.sh\
		${DESTDIR}${MANPREFIX}/usr/share/xsessions/jeff_dwm.desktop\
		${DESTDIR}${MANPREFIX}/bin/jeff_dwm-recompile.sh\
		${DESTDIR}${MANPREFIX}/man1/dwm.1

.PHONY: all options clean dist install uninstall
