  # logger-huffman

logger-huffman is a library intented for decode compressed data
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
