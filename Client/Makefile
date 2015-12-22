
CC=cc


BUILD_CONFIG = -g -DDEBUG

CFLAGS= BUILD_CONFIG -fPIC -c -Wall -I$(shell pwd) -std=gnu99 -pedantic -D_XOPEN_SOURCE=700 -D _BSD_SOURCE

LDFLAGS= -L../libDaemon/ -lFlyLab -static -lpthread
#-lFlyLab 

SOURCES= src/testClient.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE= testClient

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) 

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm $(EXECUTABLE)
	rm src/*.o