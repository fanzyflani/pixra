#!/bin/sh
DIRNAME="$(pwd)/$(dirname "$0")"
echo dir: "$DIRNAME"
export LD_LIBRARY_PATH=${DIRNAME}/libs/:$LD_LIBRARY_PATH
export LD_PRELOAD=${DIRNAME}/libs/libSDL-1.2.so.0.11.4
cd /home/pi/pixra
${DIRNAME}/pixra $@

