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

Static library liblogger-huffman.a (-l logger-huffman).

Header file: logger-sql-clause.h

Optional headers:

- logger-collection.h
- logger-huffman.h
- logger-passport.h
- util-compress.h
- util-time-fmt.h

Usage:
```
#include "logger-sql-clause.h"
...
// create table clause 
std::string r = createTableSQLClause(OUTPUT_FORMAT_SQL, dialect);
...
// load packet
LoggerKosaCollection c;
std::vector<std::string> s;
    s.push_back(hex2binString(packet0));
    LOGGER_PACKET_TYPE t = c.put(s);

...
// insert into clause
r = parsePacketsToSQLClause(OUTPUT_FORMAT_SQL, dialect, *c.koses.begin());

```

### Command line tools

- logger-huffman-print get packets in hex and print out temperature values

#### logger-huffman-print

logger-huffman-print get packets from command line (in hex) and print out
contents in JSON format or as tab delimited fields.

```

```

## Build
./logger-huffman-print -f json 002614121f0c14261300003d3d71000100cf06aa01e6ff00 02deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff00 08ddff0009e3ff000adeff000bdaff000cdfff000debff00 0ee8ff000fcdff0010e7ff0011dfff0012ddff0013e1ff00 14dcff0015dcff0016ebff0017e6ff0018dfff0019dfff00 1adaff001be6ff00
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

Examples:
```
./logger-huffman-print -f table 4a00800001072613002614121f0c14261300003d3d7100014b26010200cf06aa01e6ff0002deff0003eaff0004dcff004b26010305e3ff0006e0ff0007e2ff0008ddff0009e3ff004b2601040adeff000bdaff000cdfff000debff000ee8ff004b2601050fcdff0010e7ff0011dfff0012ddff0013e1ff004b26010614dcff0015dcff0016ebff0017e6ff0018dfff004b26010719dfff001adaff001be6ff00
38	2019	1	0	1614438000	2021-02-28T00:00:00	2021-02-27T15:00:00	0	0	0	0	0	0	0	108.938	1	-1.625	2	-2.125	3	-1.375	4	-2.25	5	-1.8125	6	-2	7	-1.875	8	-2.1875	9	-1.8125	10	-2.125	11	-2.375	12	-2.0625	13	-1.3125	14	-1.5	15	-3.1875	16	-1.5625	17	-2.0625	18	-2.1875	19	-1.9375	20	-2.25	21	-2.25	22	-1.3125	23	-1.625	24	-2.0625	25	-2.0625	26	-2.375	27	-1.625	
```

```
./logger-huffman-print -f table 002614121f0c14261300003d3d71000100cf06aa01e6ff00 02deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff00 08ddff0009e3ff000adeff000bdaff000cdfff000debff00 0ee8ff000fcdff0010e7ff0011dfff0012ddff0013e1ff00 14dcff0015dcff0016ebff0017e6ff0018dfff0019dfff00 1adaff001be6ff00
38	2019	0	0	1606814438	2020-12-01T18:20:38	2020-12-01T09:20:38	38	19	4.61639	4.61639	0	0	0	0	1	-1.625	2	-2.125	3	-1.375	4	-2.25	5	-1.8125	6	-2	7	-1.875	8	-2.1875	9	-1.8125	10	-2.125	11	-2.375	12	-2.0625	13	-1.3125	14	-1.5	15	-3.1875	16	-1.5625	17	-2.0625	18	-2.1875	19	-1.9375	20	-2.25	21	-2.25	22	-1.3125	23	-1.625	24	-2.0625	25	-2.0625	26	-2.375	27	-1.625
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
