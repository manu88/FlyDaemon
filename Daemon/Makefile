
CC=cc
CFLAGS= -g -fPIC -c -Wall -I$(shell pwd) -std=gnu99 -pedantic -D_XOPEN_SOURCE=700 -D _BSD_SOURCE

LDFLAGS= -lpthread
#-L../libDaemon/ -lFlyLab -static 
#-lFlyLab 

SOURCES= src/main.c

SOURCES+= ../PrivateAPI/IPCComm.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE= Daemon

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) 

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm $(EXECUTABLE)
	rm src/*.o
	rm ../PrivateAPI/IPCComm.o
