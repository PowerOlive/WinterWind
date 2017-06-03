# WinterWind library

[![build status](https://gitlab.com/WinterWind/WinterWind/badges/master/build.svg)](https://gitlab.com/WinterWind/WinterWind/commits/master)

WinterWind is a high level C++ API which provides interesting high level features.

WinterWind required C++11 at least.

You can found library documentation (Doxygen) [here](https://winterwind.gitlab.io/WinterWind/)

# Compilation

You need a full C++11 compliant compiler to use this library (GCC 5.1 or Clang 3.4)

## Library requirements

* jsoncpp
* libxml2
* libcurl
* libressl/openssl
* libpq
* hiredis
* websocketpp
* asio

## Build

```bash
mkdir build && cd build && cmake .. && make -j X && make install
```

# Components

## WinterWind Core

### Multithreading

* Semaphores
* Threading interface
* ThreadPool
* ThreadPool working queue (multiple providers and multiple consumers queue IN/OUT)

### Web

* Client (High level cURL interface)
* HTTP daemon (based on libmicrohttpd)
* JsonWebToken encoder & decoder/validator

### Databases

* Redis client
* MySQL client
* PostgreSQL client

### Misc

* High level XML parser (based on libxml2)
* Console thread

### Lua bindings

* httpclient
* pgclient (experimental)
* string manipulations helpers
* xmlparser

## Compilation options

* __ENABLE_CONSOLE__: enable ConsoleThread support
* __ENABLE_READLINE__: enable Readline library support (requires ENABLE_CONSOLE)
* __ENABLE_POSTGRESQL__: enable Postgresql C++ client
* __ENABLE_MYSQL__: enable MySQL C++ client
* __ENABLE_HTTPCLIENT__: enable HTTP client support
* __ENABLE_OAUTHCLIENT__: enable OAuth client support (requires ENABLE_HTTPCLIENT)
* __ENABLE_REDIS__: enable Redis client support
* __ENABLE_HTTPSERVER__: enable HTTP server support
* __ENABLE_JWT__: enable JsonWebTokens support
* __ENABLE_LUA_ENGINE__: enable Lua engine with bindings (depending on previous options)
* __ENABLE_EXTRAS__: enable WinterWind extras library compilation

## WinterWind Extras

### Web

* Client (High level cURL interface)
	* Caldav client (WIP)
	* Elasticsearch Client (WIP)
	* Gitlab client
	* Jira client
	* RATP client (for RER only)
	* Slack client
	* Twitter client

### Misc

* Name generator

### Lua bindings

* jiraclient
* ratpclient

## Compilation options

* __ENABLE_CALDAV__: enable caldav client support (requires ENABLE_HTTPCLIENT)
* __ENABLE_ELASTICSEARCH__: enable Elasticsearch client support (requires ENABLE_HTTPCLIENT)
* __ENABLE_GITLAB__: enable Gitlab client support (requires ENABLE_HTTPCLIENT)
* __ENABLE_JIRACLIENT__: enable Jira client support (required ENABLE_HTTPCLIENT)
* __ENABLE_NAMEGENERATOR__: enable Name Generator support
* __ENABLE_RATPCLIENT__: enable RATP client support (requires ENABLE_HTTPCLIENT)
* __ENABLE_SLACKCLIENT__: enable Slack client support (requires ENABLE_HTTPCLIENT & websocketpp library)
* __ENABLE_TWITTERCLIENT__: enable Twitter client support (requires ENABLE_HTTPCLIENT & ENABLE_OAUTHCLIENT)

