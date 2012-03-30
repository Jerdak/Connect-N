CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=ConnectN.cpp ConnectBoard.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=../bin/player

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
