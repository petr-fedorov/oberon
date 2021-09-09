#!/bin/bash
docker build -t bouml:latest -<<EOF
FROM debian:buster
RUN echo "deb http://deb.debian.org/debian buster contrib" | tee -a /etc/apt/sources.list && apt-get update && apt-get install -y wget vim-gtk3 gnupg2 fonts-liberation ttf-mscorefonts-installer 
RUN echo "deb http://www.bouml.fr/apt/buster buster free" | tee -a /etc/apt/sources.list && wget -q https://www.bouml.fr/bouml_key.asc -O- | apt-key add - && apt-get update && apt-get install -y bouml 
EOF

