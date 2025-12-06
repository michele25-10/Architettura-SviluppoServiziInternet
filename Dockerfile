FROM ubuntu:20.04

# Evita prompt interattivi durante l'installazione
ENV DEBIAN_FRONTEND=noninteractive

# Aggiorna i pacchetti e installa tutto il necessario
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    gdb \
    make \
    default-jdk \
    default-jre \
    zip \
    libunistring-dev \
    man \
    manpages \
    manpages-dev \
    man-db \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Crea directory fakebrew per il Makefile
RUN mkdir -p /fakebrew/lib /fakebrew/include

# Fake brew per il Makefile
RUN printf '#!/bin/sh\nif [ "$1" = "-v" ]; then echo "Homebrew 4.0.0"; else echo "/fakebrew"; fi\n' > /usr/local/bin/brew \
    && chmod +x /usr/local/bin/brew

RUN apt-get install -y manpages manpages-dev

# Imposta la working directory
WORKDIR /unix