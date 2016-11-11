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

## Databases

* High level redis client
* PostgreSQL client

## Misc

* High level XML parser
* Name generator

# Requirements

* libcurl
* jsoncpp
* libxml2
* libressl/openssl
* libpq
* hiredis

# Build

Install the required libraries and just do that:

```bash
mkdir build && cmake .. && make && make install
```
