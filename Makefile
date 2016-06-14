CFLAGS = `pkg-config --cflags --libs hidapi-libusb`

all: exbtn

clean:
	@rm --verbose -f exbtn
