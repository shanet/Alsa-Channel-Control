# Alsa Control Server
### Author: Shane Tully (shane@shanetully.com)
### shanetully.com

https://github.com/shanet/Alsa-Channel-Control


## Compiling
The server and client will compile with just a basic development enviroment set up.
The Android client requires the Android SDK and Ant to be installed.
While not required at compile-time, the Alsa program "amixer" is required for the server to
work properly.

For the server and client:
make

sudo make install

For the Android client:
make android

sudo make install_android  <-- You should have your Android device plugged in


## Usage
The server was written and tested on a Debian-based system. It should work on other Linux distros,
but no guarantees are made.

First, start the server:
sudo service alsa-server start

Either use the client (alsa-client) or the Android application to communicate with the server.

See "alsa-client -h" and "alsa-server -h" for specific usage info.


## Support
This program is a little side project and carries no warranty or support
from its author. However, bugs and feature requests may be submitted to the GitHub repo
linked to above.


## Legal
This program is open source software. It is free to distribute, modify, and use
with the exception of it being made closed source and sold for commercial purposes
without the consent of the author. However, the author simply requests that if you 
do something cool with it, you let him check it out by emailing shane@shanetully.com 
or just let him know you find it useful.
