# SimpleApiServer - C++ REST Server


Project   | apiserver
:---      | ---:
Author    | [M. Massenzio](https://bitbucket.org/marco)
Release   | 0.2.0
Updated   | 2020-03-07


[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# Objective

While searching for a simple-to-use REST API server for my [distlib](https://github.com/massenz/distlib) C++ Gossip server, it emerged that there doesn't appear to be any such thing.

The closest I could get was [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/tutorial.html), but that one was cumberome to use; too much C-like; and really unsuitable to writing modern REST APIs (suffice to say, its POST model is based around HTTP Forms, and who uses **those** these days?).

This implementation really tries to be minimalist and it is **definitely not meant for Production-class, high-volume requests handling**: if you need such a thing, the good folks at Facebook have open-sourced [Folly](https://www.facebook.com/notes/facebook-engineering/folly-the-facebook-open-source-library/10150864656793920/); use that one (and good luck compiling and building it: I gave up after two days of trying).

Hence, the goals of this project are:

- simple to use; compile; and add to your own C++ project;
- keep dependencies to a minimum;
- provide a very simple programming model, based on modern C++ concepts (most notably, lambdas).

See the [Demo Server](#demo-server) section below for usage examples.

The full API documentation (created with [Doxygen](http://www.stack.nl/~dimitri/doxygen/index.html)) is available on [the Project's GH-Pages](https://massenz.github.io/SimpleApiServer/).


# Usage

We define the concept of a `Handler` (in practice, a Lambda function) that will be registered to handle a particular `{endpoint, method}` HTTP request:

```cpp
using Handler = std::function<Response(const Request &)>;
```

The `ApiServer` class adds utility methods to register handlers:

```cpp
  void AddGet(const std::string &resource, const Handler &handler) {
    AddMethodHandler("GET", resource, handler);
  }

  void AddPost(const std::string &resource, const Handler &handler) {
    AddMethodHandler("POST", resource, handler);
  }

  void AddPut(const std::string &resource, const Handler &handler) {
    AddMethodHandler("PUT", resource, handler);
  }

  void AddDelete(const std::string &resource, const Handler &handler) {
    AddMethodHandler("DELETE", resource, handler);
  }
```

A `Handler` receives a `Request` (containing, as appropriate, headers, query args and a body) and will return a `Response` (equally containing headers and a body, as well as a status code).

For an example of adding REST endpoints to your program, see the `server_demo.cpp` example:

```cpp
  api::rest::ApiServer server(port);
  server.AddGet("demo", [](const api::rest::Request& req) {
    auto query = req.GetQueryArg("q");

    auto resp = api::rest::Response::ok();
    resp.AddHeader("Content-Type", "text/plain");

    if (!query.empty()) {
      resp.set_body("Your query: " + query);
    } else {
      resp.set_body("Use the `q` query argument to ask me anything");
    }
    return resp;
  });
  server.AddGet("stop", [=](const api::rest::Request& req) {
    ::stopped.store(true);
    return api::rest::Response::ok();
  });

  server.Start();
  ```

# API Documentation

All the classes are documented using [Doxygen](http://www.doxygen.nl/); simply run

    $ doxygen

from the project's top directory, and the HTML docs will be installed in `docs/apidocs` (this folder is explicityly ignored by `git`).


# Build & Test

## Pre-requisites

The scripts in this repository take advantage of shared common utility functions
in [this common utils repository](https://bitbucket.org/marco/common-utils): clone it
somewhere, and make `$COMMON_UTILS_DIR` point to it:

```shell script
git clone git@bitbucket.org:marco/common-utils.git
export COMMON_UTILS_DIR="$(pwd)/common-utils"
```

To build/test the project, link to the scripts there:

```shell script
ln -s ${COMMON_UTILS_DIR}/build.sh bin/build
ln -s ${COMMON_UTILS_DIR}/test.sh bin/test
```


## Installation directory

In order to find its dependencies (most notably, `libmicrohttpd`) and to be found by dependent projects, we need to define an installation directory in `$INSTALL_DIR`; make sure it is defined in your environment and exists.

Header files and shared libraries will be installed/looked up in `$INSTALL_DIR/include` and `$INSTALL_DIR/lib` respectively.


## HTTP Server

To serve the REST API, we use [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/), "a small C library that is supposed to make it easy to run an HTTP server as part of another application."

This can be either installed directly as a package under most Linux distributions (for example, on Ubuntu, `sudo apt-get install libmicrohttpd-dev` will do the needful), or can be built from source:

```
wget http://open-source-box.org/libmicrohttpd/libmicrohttpd-0.9.55.tar.gz
tar xfz libmicrohttpd-0.9.55.tar.gz
cd libmicrohttpd-0.9.55/
./configure --prefix ${INSTALL_DIR}
make && make install
```

See [the tutorial](https://www.gnu.org/software/libmicrohttpd/tutorial.html) for more information about usage.


## Build, Test & Install

To build the project:

```shell script
$ export INSTALL_DIR=/some/path
$ ./bin/build && ./bin/test
```

or to simply run a subset of the tests with full debug logging:

    $ ./bin/test -v --gtest_filter="Simple*"

To install the generated binaries (`.so` or `.dylib` shared libraries)
and headers so that other projects can find them:

    $ cd build && make install

See the scripts in the `${COMMON_UTILS_DIR}` folder for more options.
