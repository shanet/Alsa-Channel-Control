# Shane Tully

CC=g++
LANG=cpp

SOURCES=alsa_control.cpp
OBJECTS=$(SOURCES:.$(LANG)=.o)
EXECUTABLE=alsa_control

CFLAGS=-c -Wall -O2

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
		$(CC) -o $(EXECUTABLE) $(OBJECTS)

.$(LANG).o:
		$(CC) $(CFLAGS) $< -o $@

install:
		mv $(EXECUTABLE) /usr/sbin/

clean:
		rm /usr/sbin/$(EXECUTABLE)
