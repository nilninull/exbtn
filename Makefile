CFLAGS = -std=c99 `pkg-config --cflags --libs hidapi-libusb` -O3
all: exbtn

clean:
	@rm --verbose -f exbtn
