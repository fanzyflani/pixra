#!/bin/sh
# IndieCity's client tends to drop all permissions.
# Reinstate the most important ones.
chmod 755 pixra
chmod 755 libs/libSDL-1.2.so.0.11.4
mkdir /home/pi/pixra
chown pi:pi /home/pi/pixra
ln -s /home/pi/pixra h

