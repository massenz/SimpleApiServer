# SimpleApiServer - C++ REST Server


 apiserver  | 0.1.0
:-------    | ---------------------------------:
 Author     | [M. Massenzio](https://www.linkedin.com/in/mmassenzio)
 Updated    | 2017-12-09

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


# Install & Build

## Conan packages

To build the project, you need to first donwload/build the necessary binary dependencies, as
listed in `conanfile.text`.

This is done as follows:

```bash
sudo -H pip install -U conan
mkdir .conan && cd .conan
conan install .. -s compiler=clang -s compiler.version=4.0 \
    -s compiler.libcxx=libstdc++11 --build=missing
```
See [conan.io](http://conan.io) for more information.

## HTTP Server

To serve the REST API, we use [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/), "a small C library that is supposed to make it easy to run an HTTP server as part of another application."

This can be either installed directly as a package under most Linux distributions (for example, on Ubuntu, `sudo apt-get install libmicrohttpd-dev` will do the needful), or can be built from source:

```
wget http://open-source-box.org/libmicrohttpd/libmicrohttpd-0.9.55.tar.gz
tar xfz libmicrohttpd-0.9.55.tar.gz
cd libmicrohttpd-0.9.55/
./configure --prefix ${INSTALL}/libmicrohttpd
make && make install
```

The include file and libraries will be, respectively, in `${INSTALL}/libmicrohttpd/include` and `${INSTALL}/libmicrohttpd/lib` folders.

See [the tutorial](https://www.gnu.org/software/libmicrohttpd/tutorial.html) for more information about usage.


## Build & testing

To build and test it:

    $ ./bin/build && ./bin/test

or to simply run a subset of the tests with full debug logging:

    $ GLOG_v=2 ./build/tests/bin/unit_tests --gtest_filter=FilterThis*

See also the other binaries in the `build/bin` folder for more options.

Define the following env vars to specify, respectively, an install dir for libs & includes; and
a shared cmake utility functions file:

    ${LOCAL_INSTALL_DIR}
    ${COMMON_UTILS_DIR}


# API Documentations

All the classes are documented using [Doxygen](https://massenz.github.io/apiserver/).

# Demo server

  TODO
