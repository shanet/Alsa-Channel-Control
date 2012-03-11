#! /bin/bash
# alsa-control-server
# shane tully (shane@shanetully.com)
# shanetully.com
# https://github.com/shanet/Alsa-Channel-Control

SERVER_BINARY=bin/alsa-server
CLIENT_BINARY=bin/alsa-client
ANDROID_BINARY=bin/android-client.apk
INIT_SCRIPT=alsa-server

# Force root
if [ "$(id -u)" != "0" ]; then
   echo "You must be root!" 1>&2
   exit 1
fi

echo "Installing server..."
cp $SERVER_BINARY /usr/sbin/$SERVER_BINARY
echo "Installing client..."
cp $CLIENT_BINARY /usr/sbin/$CLIENT_BINARY
echo "Installing startup script..."
cp  /etc/init.d/$INIT_SCRIPT
echo "Setting startup script permissions..."
chmod 744 /etc/init.d/$INIT_SCRIPT

echo -n "Install Android client? (y,N) "
read android

if[ $android -eq "y" || $android -eq "Y" ]; then
    echo "Make sure your Android device is plugged in to your computer and is configured to sideload apps. Press enter to continue."
    read junk
    adb -d install $ANDROID_CLIENT
fi

echo "Installation finished. Use \"sudo service alsa-server start\" to start the server."
echo -n "Start server now? (y,N) "
read start

if[ $start -eq "y" || $start -eq "Y" ]; then
    service $INIT_SCRIPT start
fi

exit 0
