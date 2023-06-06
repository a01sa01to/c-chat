# C Chat

Simple Chat Application written in C.

IT IS NOT SAFE. DO NOT USE.
THIS IS JUST A PRACTICE TO USE SOCKET AND THREAD.
I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE.
I DON'T RECOMMEND YOU TO COPY ANY PART OF THIS CODE.

(主に埼玉大学生向け)
本コードのコピーを禁止します。
レポートに一部分でも使うとコピペとなります。
コピペレポートは0点です。気を付けましょう。

**Copyright (c) Asa (a01sa01to) All Rights Reserved.**

## How to use

### Server

```bash
gcc server.c -o server -lpthread -D_GNU_SOURCE
./server <port>
# or just run "make run_server" to start server on port 50000
```

### Client

```bash
gcc client.c -o client -lpthread -D_GNU_SOURCE
./client <host> <port>
# or just run "make run_client" to connect to localhost:50000
```

## Features

- Send messages
- Up to 10 clients

## About Source

```plaintext
│
├── Makefile
├── README.md
├── client.c: client source
├── server.c: server source
├── common
│   ├── io.h: input/output functions
│   ├── string.h: my string struct and functions
│   ├── myutil.h: my utility functions
│   └── mleak.h: memory leak detection
├── server_h
│   ├── struct.h: server struct
│   ├── client.h: client handler
│   ├── receive.h: receive message handler
│   ├── send.h: send message handler
│   └── global_var.h: global variables
└── client_h
    ├── receive.h: receive message handler
    └── send.h: send message handler
```

## Known Issues

- Server crashes when client disconnects
- Hard to understand what command to use
- Memory leak
- Cannot send arbitrary length of message
