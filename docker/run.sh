#!/bin/bash
docker run  -d --rm -v /tmp/.X11-unix:/tmp/.X11-unix -v ${HOME}/repos/oberon:/oberon -v /tmp/oberon:/tmp/oberon -v /tmp/sm:/tmp/sm -v bouml:/root -e DISPLAY=${DISPLAY} -e QT_X11_NO_MITSHM=1 bouml bouml
