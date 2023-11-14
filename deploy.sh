#!/bin/bash

NAME="server.exe"

git pull

if [ $? -ne 0 ]; then
    rm -rf build
    mkdir build
    cd build 
    cmake .. -C Release -GNinja
    ninja -C build

    if pgrep "$NAME" > /dev/null; then
       pkill "$NAME"
       sleep 2
    fi

    ./build/Release/$NAME
fi