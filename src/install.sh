#! /bin/bash
# alsa-control-server
# shane tully (shane@shanetully.com)
# shanetully.com
# https://github.com/shanet/Alsa-Channel-Control

SERVER_BINARY=alsa-server
CLIENT_BINARY=alsa-client
ANDROID_BINARY=android-client.apk
INIT_SCRIPT=alsa-server

INSTALL_DIR=/usr/sbin/

echo "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="
echo "    Alsa Server install script     "
echo "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="

# Force root
if [ "$(id -u)" != "0" ]; then
   echo "We have to copy binaries into $INSTALL_DIR. You must be root!" 1>&2
   exit 1
fi

echo "Installing server..."
cp bin/$SERVER_BINARY $INSTALL_DIR$SERVER_BINARY
echo "Installing client..."
cp bin/$CLIENT_BINARY $INSTALL_DIR$CLIENT_BINARY
echo "Installing startup script..."
cp  $INIT_SCRIPT /etc/init.d/$INIT_SCRIPT
echo "Setting startup script permissions..."
chmod 744 /etc/init.d/$INIT_SCRIPT

echo -n "Install Android client? (y,N) "
read android

if [[ $android = "y" || $android = "Y" ]] ; then
    echo -n "Make sure your Android device is plugged in to your computer and is configured to sideload apps. Press enter to continue."
    read junk
    adb -d install $ANDROID_CLIENT
fi

echo "Installation finished. Use \"sudo service alsa-server start\" to start the server."
echo -n "Start server now? (y,N) "
read start

if [[ $start = "y" || $start = "Y" ]] ; then
    service $INIT_SCRIPT start
fi

exit 0
