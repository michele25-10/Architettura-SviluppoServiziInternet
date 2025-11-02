FROM gcc:latest

RUN apt-get update && apt-get install -y gdb

WORKDIR /workspace