# source files.
SRC =  	src/Dispatch.c \
		src/DispatchThread.c \

 
OBJ = $(SRC:.c=.o)
 
OUT = libFlyLab.a
 
# include directories
INCLUDES = -I. -I./include/  	-I/usr/local/include
 
# C++ compiler flags (-g -O2 -Wall)
CFLAGS = -g -Wall -lm
 
# compiler
CCC = cc -std=c99
 
# library paths
LIBS = -L../ -L/usr/local/lib -lm
 
# compile flags
LDFLAGS = -g
 
.SUFFIXES: .c
 
default: dep $(OUT)
 
.c.o:
	$(CCC) $(INCLUDES) $(CFLAGS) -c $< -o $@
 
$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)
 
depend: dep
 
dep:
	makedepend -Y -- $(CFLAGS) -- $(INCLUDES) $(SRC)
 
clean:
	rm -f $(OBJ) $(OUT) Makefile.bak# DO NOT DELETE

src/Dispatch.o: src/Dispatch.h src/DispatchThread.h src/IPCMessage.h
src/Dispatch.o: include/UAVTalk.h
src/DispatchThread.o: src/Dispatch.h src/DispatchThread.h src/IPCMessage.h
src/DispatchThread.o: include/UAVTalk.h
# DO NOT DELETE

src/Dispatch.o: src/Dispatch.h src/DispatchThread.h src/IPCMessage.h
src/Dispatch.o: include/UAVTalk.h
src/DispatchThread.o: src/Dispatch.h src/DispatchThread.h src/IPCMessage.h
src/DispatchThread.o: include/UAVTalk.h
