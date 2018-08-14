[![Build Status](https://api.travis-ci.org/janbar/noson.svg?branch=master)](http://travis-ci.org/janbar/noson)

## About NOSON

This project is intended to create a easy client interface to
drive SONOS player. Its development started from November 2015
and today the API supports basic features to browse music index
and control playback in any zones.

## Building

### Linux, BSD, OSX

Configure, make and install

<pre><code>cmake -DCMAKE_BUILD_TYPE=Release $NOSON_PROJECT_DIR
make
sudo make install</code></pre>

### Windows

Start by installing VC studio 2012 and CMAKE tool

To build open a command tool CMD.EXE from the project folder and execute the following
<pre><code>mkdir build_vc
cd build_vc
cmake ..
cmake --build .</code></pre>

## Running a test

./test/nosontest

## Generate the documentation

sudo apt-get install graphviz

doxygen <root path of noson>/docs/doxygen-dev.cfg

