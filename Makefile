CC=g++
LANG=cpp

SERVER_BINARY=server
CLIENT_BINARY=client
LOCAL_BINARY=local

SERVER_SOURCES=$(wildcard src/server/*.h) $(wildcard src/server/*.cpp)
CLIENT_SOURCES=$(wildcard src/client/*.h) $(wildcard src/client/*.cpp)
LOCAL_SOURCES=$(wildcard src/local/*.h) $(wildcard src/local/*.cpp)
LIBRARY_SOURCES=$(wildcard lib/*.cpp) $(wildcard lib/*.h)

INCLUDE_PATHS=lib/

SERVER_OBJECTS=$(SERVER_SOURCES:.$(LANG)=.o)
CLIENT_OBJECTS=$(CLIENT_SOURCES:.$(LANG)=.o)
LOCAL_OBJECTS=$(LOCAL_SOURCES:.$(LANG)=.o)
LIBRARY_OBJECTS=$(LIBRARY_SOURCES:.$(LANG)=.o)

LIBRARY=$(LIBRARY_OBJECTS:.o=.a)

CFLAGS=$(CLFLAGS) -c
CLFLAGS=-I$(INCLUDE_PATHS) -Wall -O2


all: $(SERVER_BINARY) $(CLIENT_BINARY) $(LOCAL_BINARY)


$(SERVER_BINARY): $(LIBRARY_OBJECTS) $(SERVER_OBJECTS)
		ar -cvq $(LIBRARY) $(LIBRARY_OBJECTS)
		$(CC) $(CLFLAGS) -o bin/$(SERVER_BINARY) $(SERVER_OBJECTS) $(LIBRARY)

$(CLIENT_BINARY): $(CLIENT_OBJECTS)
		$(CC) $(CLFLAGS) -o bin/$(CLIENT_BINARY) $(CLIENT_OBJECTS)

$(LOCAL_BINARY): $(LIBRARY_OBJECTS) $(LOCAL_OBJECTS)
		ar -cvq $(LIBRARY) $(LIBRARY_OBJECTS)
		$(CC) $(CLFLAGS) -o bin/$(LOCAL_BINARY) $(LOCAL_OBJECTS) $(LIBRARY)

.$(LANG).o:
		$(CC) $(CFLAGS) $< -o $@

clean:
		rm -f $(wildcard src/server/*.o)
		rm -f $(wildcard src/client/*.o)
		rm -f $(wildcard src/local/*.o)
		rm -f $(wildcard lib/*.o)
		rm -f $(wildcard lib/*.a)
		rm -f bin/{$(SERVER_BINARY),$(CLIENT_BINARY),$(LOCAL_BINARY)}
