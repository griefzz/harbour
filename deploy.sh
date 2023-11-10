#!/bin/bash

PROGRAM_NAME="server.exe"

git pull

if [ $? -ne 0 ]; then
    rm -rf build
    mkdir build && cd build
    cmake .. -C Release -GNinja
    ninja

    if pgrep "$PROGRAM_NAME" > /dev/null; then
       pkill "$PROGRAM_NAME"
       sleep 2
    fi
    
    ./Release/$PROGRAM_NAME
fi