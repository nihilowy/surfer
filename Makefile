EXTENSION_DIR=$(DESTDIR)/usr/lib/surfer/
ifeq ($(DEBUG), 1)
	CFLAGS += -Wall
else
	DEBUG = 0
	CFLAGS += -Wno-deprecated-declarations
endif
DDEBUG=-DDEBUG=${DEBUG}

all: surfer adblock.so

surfer: surfer.c Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< \
		`pkg-config --cflags --libs gtk+-3.0 glib-2.0 webkit2gtk-4.0` \
		-DEXTENSION_DIR=\"$(EXTENSION_DIR)\" \
		$(DDEBUG) -lm

adblock.so: ext.c Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -shared -fPIC \
		`pkg-config --cflags --libs gtk+-3.0 glib-2.0 webkit2gtk-4.0` \
		$(DDEBUG)


install:all
	install -d $(INSTALLDIR)/usr/share/surfer
	install -d $(INSTALLDIR)/usr/lib
	install -d $(INSTALLDIR)/usr/lib/surfer
#	install -m0644 ext.so $(INSTALLDIR)/usr/lib/surfer/
	install -Dm755 surfer $(DESTDIR)/usr/bin/surfer
	install -Dm644 surfer.desktop $(DESTDIR)/usr/share/applications/surfer.desktop
	install -Dm644 surfer.1 $(DESTDIR)/usr/share/man/man1/surfer.1
	install -Dm644 black.css $(DESTDIR)/usr/share/surfer/black.css

  








