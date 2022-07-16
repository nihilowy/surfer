DESTDIR=/usr
EXTENSION_DIR=$(DESTDIR)/lib/surfer/

ifeq ($(DEBUG), 1)
	CFLAGS += -Wall -g
else
	DEBUG = 0
#	CFLAGS += -march=x86-64  -mtune=generic -O2 -pipe -fno-plt -fexceptions -Wp,-D_FORTIFY_SOURCE=2 -Wformat -Werror=format-security -fstack-clash-protection -fcf-protection -Wformat  -Werror=format-security -fstack-clash-protection -fcf-protection -Wno-deprecated-declarations


CFLAGS   += -std=c99 -pipe -Wall -fPIC
endif
DDEBUG=-DDEBUG=${DEBUG}

all: surfer ephy-scripts.so adblock.so

surfer: surfer.c Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< \
		`pkg-config --cflags --libs libnotify gtk+-3.0 glib-2.0 webkit2gtk-4.0` \
		-DEXTENSION_DIR=\"$(EXTENSION_DIR)\" \
		$(DDEBUG) -lm

adblock.so: ext.c Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -shared -fPIC \
		`pkg-config --cflags --libs libnotify gtk+-3.0 glib-2.0 webkit2gtk-4.0` \
		$(DDEBUG)



ephy-scripts.so: ephy-scripts.c Makefile
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -shared -fPIC \
		`pkg-config --cflags --libs libnotify gtk+-3.0 glib-2.0 webkit2gtk-4.0` \
		$(DDEBUG)


install:all
	install -d $(DESTDIR)/share/surfer
	install -d $(DESTDIR)/lib
	install -d $(DESTDIR)/lib/surfer
	install -Dm644 ephy-scripts.so $(DESTDIR)/lib/surfer/
#	install -Dm644 adblock.so $(DESTDIR)/lib/surfer/
	install -Dm755 surfer $(DESTDIR)/bin/surfer
	install -Dm644 surfer.desktop $(DESTDIR)/share/applications/surfer.desktop
	install -Dm644 surfer.1 $(DESTDIR)/share/man/man1/surfer.1
	install -Dm644 black.css $(DESTDIR)/share/surfer/black.css
	install -Dm644 icons/24x24/surfer.png $(DESTDIR)/share/icons/hicolor/24x24/surfer.png
	install -Dm644 icons/32x32/surfer.png $(DESTDIR)/share/icons/hicolor/32x32/surfer.png
	install -Dm644 icons/48x48/surfer.png $(DESTDIR)/share/icons/hicolor/48x48/surfer.png
#	install -Dm644 icons/surfer.svg $(DESTDIR)/share/icons/hicolor/scalable/apps/surfer.svg

uninstall:
	$(RM) $(DESTDIR)bin/surfer
	$(RM) $(DESTDIR)/man/man1/surfer.1
	$(RM) $(DESTDIR)//share/surfer/black.css
	$(RM) $(DESTDIR)/share/applications/surfer.desktop








