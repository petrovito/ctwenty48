#! /bin/bash
set -e
docker build . -t ctwenty48
xhost + local:
docker run --rm -v/tmp/.X11-unix:/tmp/.X11-unix -eDISPLAY=$DISPLAY -h$HOSTNAME -v$HOME/.Xauthority:/root/.Xauthority ctwenty48:latest

