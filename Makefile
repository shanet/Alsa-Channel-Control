# alsa-control-server
# shane tully (shane@shanetully.com)
# shanetully.com
# https://github.com/shanet/Alsa-Channel-Control

CC=g++
LANG=cpp

PROJ_NAME=alsa-server
SERVER_BINARY=alsa-server
CLIENT_BINARY=alsa-client
ANDROID_CLIENT_BINARY=android-client.apk
INIT_SCRIPT=alsa-server

INSTALL_SCRIPT=install.sh
UNINSTALL_SCRIPT=uninstall.sh

INSTALL_DIR=/usr/sbin/
BIN_TAR=alsa-server-bin.tar.gz

SERVER_SOURCES=$(wildcard src/server/*.h) $(wildcard src/server/*.cpp)
CLIENT_SOURCES=$(wildcard src/client/*.h) $(wildcard src/client/*.cpp)

INCLUDE_PATHS=

SERVER_OBJECTS=$(SERVER_SOURCES:.$(LANG)=.o)
CLIENT_OBJECTS=$(CLIENT_SOURCES:.$(LANG)=.o)

CFLAGS=$(CLFLAGS) -c
CLFLAGS=-I$(INCLUDE_PATHS) -Wall -Wextra -O2


all: server client android

server: $(SERVER_OBJECTS)
	$(CC) $(CLFLAGS) -o bin/$(SERVER_BINARY) $(SERVER_OBJECTS)

client: $(CLIENT_OBJECTS)
	$(CC) $(CLFLAGS) -o bin/$(CLIENT_BINARY) $(CLIENT_OBJECTS)

android:
	ant -f src/android_client/build.xml debug
	cp src/android_client/bin/Main-debug.apk bin/$(ANDROID_CLIENT_BINARY)

install: server client
	cp src/server/$(INIT_SCRIPT) /etc/init.d/$(INIT_SCRIPT)
	chmod 744 /etc/init.d/$(INIT_SCRIPT)
	cp bin/$(SERVER_BINARY) $(INSTALL_DIR)$(SERVER_BINARY)
	cp bin/$(CLIENT_BINARY) $(INSTALL_DIR)$(CLIENT_BINARY)
	update-rc.d $(INIT_SCRIPT) defaults

install_android: android
	adb -d install bin/${ANDROID_CLIENT_BINARY}

remove:
	service $(INIT_SCRIPT) stop
	rm $(INSTALL_DIR)$(SERVER_BINARY)
	rm $(INSTALL_DIR)$(CLIENT_BINARY)
	rm /etc/init.d/$(INIT_SCRIPT)


remove_android:
	adb -d uninstall com.shanet.alsa_control

bin_tar: all
	mkdir -p $(PROJ_NAME)/bin
	cp bin/$(SERVER_BINARY) $(PROJ_NAME)/bin/$(SERVER_BINARY)
	cp bin/$(CLIENT_BINARY) $(PROJ_NAME)/bin/$(CLIENT_BINARY)
	cp bin/$(ANDROID_CLIENT_BINARY) $(PROJ_NAME)/bin/$(ANDROID_CLIENT_BINARY)
	cp src/server/$(INIT_SCRIPT) $(PROJ_NAME)/$(INIT_SCRIPT)

	cp src/$(INSTALL_SCRIPT) $(PROJ_NAME)/$(INSTALL_SCRIPT)
	cp src/$(UNINSTALL_SCRIPT) $(PROJ_NAME)/$(UNINSTALL_SCRIPT)
	chmod 774 $(PROJ_NAME)/$(INSTALL_SCRIPT) $(PROJ_NAME)/$(UNINSTALL_SCRIPT)
	
	cp README $(PROJ_NAME)
	cp CHANGELOG $(PROJ_NAME)
	
	tar -czf $(BIN_TAR) $(PROJ_NAME)
	
	rm -rf $(PROJ_NAME)

.$(LANG).o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(wildcard src/server/*.o)
	rm -f $(wildcard src/client/*.o)
	rm -f bin/$(SERVER_BINARY) bin/$(CLIENT_BINARY) bin/$(ANDROID_CLIENT_BINARY)
	rm -f $(BIN_TAR)
