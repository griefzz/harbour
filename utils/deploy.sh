#!/bin/bash

NAME="harbour"

cd ..
git pull

if [ $? -ne 0 ]; then
    rm -rf build
    mkdir build
    cd build 
    cmake .. -DCMAKE_BUILD_TYPE=Release -GNinja
    ninja

    if pgrep "$NAME" > /dev/null; then
       pkill "$NAME"
       sleep 2
    fi

    ./$NAME 2>&1 | tee -a log.txt
fi