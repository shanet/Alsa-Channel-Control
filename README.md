# Alsa Control Server
### Shane Tully (shane@shanetully.com)
### shanetully.com

https://github.com/shanet/Alsa-Channel-Control


## Compiling
The server and client will compile with just a basic development enviroment set up.
* The Android client requires the Android SDK and Ant to be installed.
* You need the -xorg-dev (or similar) package installed.
* While not required at compile-time, the Alsa program "amixer" is required for the server to work properly.
* In order to communicate with the X server, the startup script needs to change the user the server is running as to a user that is logged in to X. At the very least, it needs changed to a user that exists on the system. In `src/alsa-server` change the line `USER=shane` to a user on the target system.

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
Copyright (C) 2012-2013 Shane Tully

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

