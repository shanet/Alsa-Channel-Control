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
echo "    Alsa Server uninstall script     "
echo "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="

# Force root
if [ "$(id -u)" != "0" ]; then
   echo "We have to remove binaries from $INSTALL_DIR. You must be root!" 1>&2
   exit 1
fi

echo "Stopping server..."
service $INIT_SCRIPT stop
echo "Removing server..."
rm $INSTALL_DIR$SERVER_BINARY
echo "Removing client..."
rm $INSTALL_DIR$CLIENT_BINARY
echo "Removing startup script..."
rm /etc/init.d/$INIT_SCRIPT

echo -n "Uninstall Android client? (y,N) "
read android

if [[ $android = "y" || $android = "Y" ]] ; then
    echo "Make sure your Android device is plugged in to your computer and is configured to sideload apps. Press enter to continue."
    read junk
    adb -d uninstall com.shanet.alsa_control
fi

echo "Alsa Server uninstall finished."
exit 0
