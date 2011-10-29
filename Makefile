CC=g++
LANG=cpp

SOURCES=downstairsVolume.cpp
OBJECTS=$(SOURCES:.$(LANG)=.o)
EXECUTABLE=downstairsVolume

CFLAGS=-c -Wall -O2
#MACROS=-DMODELDIR=\"`pkg-config --variable=modeldir pocketsphinx`\"
#PKG_CONFIG=`pkg-config --cflags --libs pocketsphinx sphinxbase`


all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
		$(CC) -o $(EXECUTABLE) $(OBJECTS) $(PKG_CONFIG)

.$(LANG).o:
		$(CC) $(CFLAGS) $(MACROS) $< -o $@

install:
		mv $(EXECUTABLE) /usr/sbin/

clean:
		rm /usr/sbin/$(EXECUTABLE)
