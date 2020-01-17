# Makefile for the smash program
CC = g++
CFLAGS = -g -Wall
CCLINK = $(CC)
OBJS = server.o
RM = rm -f
# Creating the  executable
Main: $(OBJS)
	$(CCLINK) -o ttftps $(OBJS) $(CFLAGS)
# Creating the object files
server.o: server.cpp 
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*

