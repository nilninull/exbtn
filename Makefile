CFLAGS = -std=c99 `pkg-config --cflags --libs hidapi-libusb` -O3
PREFIX = /usr

all: exbtn openrc

openrc:
	@echo exbtn.init created
	@sed "s@%%%PREFIX%%%@${PREFIX}@" exbtn.init.base > exbtn.init

install:
	@install 	 exbtn		${DESTDIR}/${PREFIX}/sbin
	@install 	 exbtn.init	${DESTDIR}/etc/init.d/exbtn
	@install -m 0644 99-exbtn.rules ${DESTDIR}/etc/udev/rules.d

clean:
	@rm --verbose -f exbtn exbtn.init
