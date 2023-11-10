#!/bin/bash

PROGRAM_NAME="server.exe"

git pull

if [ $? -ne 0 ]; then
    rm -rf build
    mkdir build
    cmake .. -D build -C Release -GNinja
    ninja -C build

    if pgrep "$PROGRAM_NAME" > /dev/null; then
       pkill "$PROGRAM_NAME"
       sleep 2
    fi

    ./build/Release/$PROGRAM_NAME
fi