CC= cc

MAJOR=3
MINOR=3
REVISON=0
LIB_NAME = FlyLab
LIBDIR=/usr/lib
BASENAME=lib$(LIB_NAME).a
EXECUTABLE=$(BASENAME)

#.$(MAJOR).$(MINOR).$(REVISON)

BUILD_CONFIG = -g -DDEBUG

CFLAGS= BUILD_CONFIG -fPIC -c -Wall -I$(shell pwd) -std=gnu99 -pedantic -D_XOPEN_SOURCE=700 -D _BSD_SOURCE
LDFLAGS=-static -ldl -Wl,-soname,$(EXECUTABLE) -lpthread

SOURCES=$(shell echo src/*.c)

SOURCES+= ../PrivateAPI/IPCComm.c

OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	ar rcs $(EXECUTABLE) $(OBJECTS) 
	#$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	echo $(VERSION)
	rm -rf *.o $(EXECUTABLE)

install:
	mkdir -p -m 655 /usr/include/$(LIB_NAME)
	install -c $(EXECUTABLE) $(LIBDIR)
	ln -sf $(LIBDIR)/$(EXECUTABLE) /usr/include/$(BASENAME)
	ln -sf $(LIBDIR)/$(EXECUTABLE) /usr/include/$(LIB_NAME)/$(BASENAME)
	ldconfig
	cp -Rn . /usr/include/$(LIB_NAME)
	find /usr/include/$(LIB_NAME) ! -name "*.h" -delete

uninstall:
	rm -rf $(LIBDIR)/$(EXECUTABLE)
	rm -rf /usr/include/$(LIB_NAME)
	rm -rf /usr/include/$(BASENAME)