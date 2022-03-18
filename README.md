# logger-huffman

logger-huffman is a library and command line tools intended for decode compressed data
received from temperature logger.

The payload is decompressed and decoded according to the logger version 4 specification.

This library is a part of [lorawan-network-server](https://github.com/commandus/lorawan-network-server).

Clone [logger-huffman](https://github.com/commandus/logger-huffman) repository:

```
git clone git@github.com:commandus/logger-huffman.git
```
or
```
git clone https://github.com/commandus/logger-huffman.git
```

## Usage

### Library

Static library liblogger-huffman.a

Optional headers:
 
- logger-huffman.h
- logger-huffman-impl.h
- utilcompress.h
- util-time-fmt.h

Usage:
```
-l logger-huffman
```

### Command line tools

- logger-huffman-print get packets in hex and print out temperature values

#### logger-huffman-print

logger-huffman-print get packets from command line (in hex) and print out
contents in JSON format or as tab delimited fields.

```
./logger-huffman-print -f json 002614121f0c14261300003d3d71000100cf06aa01e6ff00 02deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff00 08ddff0009e3ff000adeff000bdaff000cdfff000debff00 0ee8ff000fcdff0010e7ff0011dfff0012ddff0013e1ff00 14dcff0015dcff0016ebff0017e6ff0018dfff0019dfff00 1adaff001be6ff00
```

## Build

You can use

- Automake
- CMake

build system.

Make sure you have automake installed:
```
apt install autoconf build-essential libtool
```

### Automake/Autotools

Generate automake files, configure and make:
```
cd logger-huffman
autogen.sh
```

Configure and make project using Autotools:
```
cd logger-huffman
./configure
make
```

If you prefer use clang instead of gcc:

```
./configure CC=clang CXX=clang++
```

You can install library and utilities in the system:
```
sudo make install
```

### Cmake

or cam use CMake:
```
cd logger-huffman
mkdir build
cd build
cmake ..
make
```

If you prefer use clang instead of gcc:
```
cd logger-huffman
mkdir build
cd build
export CC=/usr/bin/clang;export CXX=/usr/bin/clang++;cmake ..
make
```

## Tools

- logger-huffman-print

### logger-huffman-print utility

logger-huffman-print utility prints packet data tos stdout.

Pass packet as hex string in command line:
```
./logger-huffman-print <hex-data>
```

or pass binary data:

```
cat packet-data.bin | ./logger-huffman-print
```

### Windows

You need install vcpkg. Do not forget integrate vcpkg with Visual Studio:

```
.\vcpkg\vcpkg integrate install
```

Then build solution:
```
mkdir build
cd build
cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=C:/git/vcpkg/scripts/buildsystems/vcpkg.cmake ..
```

### References

- [lorawan-network-server](https://github.com/commandus/lorawan-network-server)
