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

```shell
sudo -H pip install -U conan
mkdir .conan && cd .conan
conan install .. -s compiler=clang -s compiler.version=4.0 \
    -s compiler.libcxx=libstdc++11 --build=missing
```

See [conan.io](http://conan.io) for more information.

## MacOS

Building this project on MacOS turns out to be a considerable pain.

For a start, for reasons completely unclear, using Conan to build `gtest` fails with missing references, even though the library (and include files) are found in the correct places: ultimately, the only fix I could find was to remove the reference to `gtest` from `conanfile.txt` and building `gtest` from sources.

Download [gtest 1.8.0 tarbal](https://github.com/google/googletest/archive/release-1.8.0.tar.gz) to your local machine, untar into `$GTEST_DIR` (see below) and build it with the following:

```shell
GTEST_DIR=${LOCAL_DIR}/gtest-1.8.0

cd ${GTEST_DIR}
mkdir build && cd build
cmake -G"Unix Makefiles" ..
make

ln -s ${GTEST_DIR}/googletest/include/gtest ${INSTALL_DIR}/include/gtest
ln -s ${GTEST_DIR}/build/googlemock/gtest/libgtest.a ${INSTALL_DIR}/lib/libgtest.a
```

Then, building `libuv` fails for unknown reasons (again, downloading the library, and building it fails to generate the `.dylib`) however, using `brew` it is possible to generate the necessary files (they have been added to the `third_party` folder for ease of build - and also because I would not want to inflict `brew` onto anyone); however, even if the build then succeeds, executing the tests fails, as the library location is "hard-coded" as in `/usr/local/lib`; thus this is also necessary:

    ln -s ${BASEDIR}/third_party/SimpleHttpRequest/lib/libhttp_parser.2.7.1.dylib \
        /usr/local/lib/libhttp_parser.2.7.dylib

Finally, it appears that the configured location for the Conan-built `glog` library is not correctly set in the binaries; if you encounter a `dylib not found` error, do this:

```shell
  # Workaround for MacOS which fails to use Conan's install directories
  LIBGLOG="${INSTALL_DIR}/lib/libglog.dylib"
  if [[ $(uname -a | cut -f 1 -d ' ') == "Darwin" && ! -e ${LIBGLOG} ]]; then
    find ~/.conan/data -iname libglog.dylib | head -n 1 | \
        xargs -I % ln -s % ${LIBGLOG}
  fi
```

## HTTP Server

To serve the REST API, we use [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/), "a small C library that is supposed to make it easy to run an HTTP server as part of another application."

This can be either installed directly as a package under most Linux distributions (for example, on Ubuntu, `sudo apt-get install libmicrohttpd-dev` will do the needful), or can be built from source:

```
wget http://open-source-box.org/libmicrohttpd/libmicrohttpd-0.9.55.tar.gz
tar xfz libmicrohttpd-0.9.55.tar.gz
cd libmicrohttpd-0.9.55/
./configure --prefix ${INSTALL}
make && make install
```

The include file and libraries will be, respectively, in `${INSTALL}/include` and `${INSTALL}/lib` folders.

See [the tutorial](https://www.gnu.org/software/libmicrohttpd/tutorial.html) for more information about usage.


## Build & testing

To build and test it:

    $ ./bin/build && ./bin/test

or to simply run a subset of the tests with full debug logging:

    $ GLOG_v=2 ./build/tests/bin/unit_tests --gtest_filter=FilterThis*

See also the other binaries in the `build/bin` folder for more options.

Define the following env vars to specify, respectively, an install dir for libs & includes; and
a shared cmake utility functions file (`common.cmake`):

    ${INSTALL_DIR}
    ${COMMON_UTILS_DIR}


# API Documentations

All the classes are documented using [Doxygen](https://massenz.github.io/apiserver/).

# Demo server

  TODO
