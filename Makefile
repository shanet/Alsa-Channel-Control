# shane tully
# shanetully.com

CC=g++
LANG=cpp

SERVER_BINARY=alsa_server
CLIENT_BINARY=alsa_client
ANDROID_CLIENT_BINARY=android_client.apk

INITRD_SCRIPT=alsa_server

SERVER_SOURCES=$(wildcard src/server/*.h) $(wildcard src/server/*.cpp)
CLIENT_SOURCES=$(wildcard src/client/*.h) $(wildcard src/client/*.cpp)

INCLUDE_PATHS=

SERVER_OBJECTS=$(SERVER_SOURCES:.$(LANG)=.o)
CLIENT_OBJECTS=$(CLIENT_SOURCES:.$(LANG)=.o)

CFLAGS=$(CLFLAGS) -c
CLFLAGS=-I$(INCLUDE_PATHS) -Wall -Wextra -O2


all: $(SERVER_BINARY) $(CLIENT_BINARY) $(ANDROID)

server: $(SERVER_OBJECTS)
	$(CC) $(CLFLAGS) -o bin/$(SERVER_BINARY) $(SERVER_OBJECTS)

client: $(CLIENT_OBJECTS)
	$(CC) $(CLFLAGS) -o bin/$(CLIENT_BINARY) $(CLIENT_OBJECTS)

android:
	ant -f src/android_client/build.xml
	cp src/android_client/bin/android_client.apk bin/$(ANDROID_CLIENT_BINARY)

install: $(SERVER_BINARY)
	cp src/server/$(INITRD_SCRIPT) /etc/init.d/$(INITRD_SCRIPT)
	chmod 744 /etc/init.d/$(INITRD_SCRIPT)
	cp bin/$(SERVER_BINARY) /usr/sbin/$(SERVER_BINARY)

remove:
	rm /etc/init.d/alsa_server
	rm /usr/sbin/alsa_server

.$(LANG).o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(wildcard src/server/*.o)
	rm -f $(wildcard src/client/*.o)
	rm -f bin/{$(SERVER_BINARY),$(CLIENT_BINARY),$(ANDROID_CLIENT_BINARY)}
