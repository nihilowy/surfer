CC=gcc
CFLAGS += -Wall -Wextra -Wno-unused-parameter -O3



.PHONY: all clear install installdirs 



all:surfer

reinstall:install
	rm -f /usr/share/surfer/black.css

	

surfer: surfer.c
	$(CC) -o $@ $^ $(CFLAGS) -o surfer `pkg-config --cflags --libs gtk+-3.0 glib-2.0 webkit2gtk-4.0`

installdirs:
	mkdir -p $(DESTDIR)$(/usr/share/surfer)
	
install:all
	install -Dm755 surfer $(DESTDIR)/usr/bin/surfer
	install -Dm644 surfer.desktop $(DESTDIR)/usr/share/applications/surfer.desktop
	install -Dm644 surfer.1 $(DESTDIR)/usr/share/man/man1/surfer.1
	install -Dm644 black.css $(DESTDIR)/usr/share/surfer/black.css
	
