CC=gcc
CFLAGS += -Wall -Wextra -Wno-unused-parameter -O3


.PHONY: all  install

all:surfer

surfer: surfer.c
	$(CC) -o $@ $^ $(CFLAGS) -o surfer `pkg-config --cflags --libs gtk+-3.0 glib-2.0 webkit2gtk-4.0`

install:all
	sudo mv  surfer /usr/bin/
	sudo cp  surfer.desktop /usr/share/applications/
