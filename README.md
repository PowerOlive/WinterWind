# WinterWind library

This library provides high level interfaces to some interesting API or features

# Features
## Multithreading

* C++ semaphores
* C++11 threading interface

## Web

* High level CURL interface
* GitlabAPI client
* Caldav client (WIP)
* Elasticsearch Client (WIP)
* Slack client

## Databases

* High level redis client
* MySQL client
* PostgreSQL client

## Misc

* High level XML parser
* Name generator
* Console thread

# Requirements

## Compilation

You need a full C++11 compliant compiler to use this library (GCC 5.1 or Clang 3.4)

## Libraries

* jsoncpp
* libxml2
* libcurl
* libressl/openssl
* libpq
* hiredis
* websocketpp
* asio

# Compilation options

* __ENABLE_CONSOLE__: enable ConsoleThread support
* __ENABLE_READLINE__: enable Readline library support (requires ENABLE_CONSOLE)
* __ENABLE_POSTGRESQL__: enable Postgresql C++ client
* __ENABLE_HTTPCLIENT__: enable HTTP client support
* __ENABLE_CALDAV__: enable caldav client support (requires ENABLE_HTTPCLIENT)
* __ENABLE_ELASTICSEARCH__: enable Elasticsearch client support (requires ENABLE_HTTPCLIENT)
* __ENABLE_GITLAB__: enable Gitlab client support (requires ENABLE_HTTPCLIENT)
* __ENABLE_RATPCLIENT__: enable RATP client support (requires ENABLE_HTTPCLIENT)
* __ENABLE_SLACKCLIENT__: enable Slack client support (required ENABLE_HTTPCLIENT & websocketpp library)
* __ENABLE_NAMEGENERATOR__: enable Name Generator support
* __ENABLE_REDIS__: enable Redis client support

# Build

Install the required libraries and just execute that:

```bash
mkdir build && cmake .. && make && make install
```
