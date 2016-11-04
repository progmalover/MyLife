#!/bin/bash

case "$1" in

    -c)
        rm -f lambda-1
        ;;
    *)
        g++ -std=c++0x ./lambda-1.cpp -o lambda-1
        ;;
esac

exit 0
