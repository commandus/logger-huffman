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

There are two way to use a library:

- include headers with types (first way)
- include only one "void *" type header (second way)

#### First way

Header file: logger-sql-clause.h

Optional headers:

- logger-collection.h
- logger-huffman.h
- logger-passport.h
- util-compress.h
- util-time-fmt.h

Source code example:

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

#### Second way

Header file: logger-parse.h

Source code example:

```
#include "logger-parse.h"
...
void *env = initLoggerParser();

sqlCreateTable(SQL_POSTGRESQL); // SQL_POSTGRESQL - 0
parsePacket(env, binaryDataString));

std::vector <std::string> clauses;
sqlInsertPackets(env, clauses, dialect);
for (auto it(clauses.begin()); it != clauses.end(); it++) {
    std::cout << *it << std::endl << std::endl;
}

flushLoggerParser(env);
doneLoggerParser(env);```
```

##### Calls

First you need create a descriptor by initLoggerParser() call. It allocates
memory to store received packets;

Call parsePacket() save packet in the memory.

Lora device send 5-10 packets in about 100-300 seconds.

Packets are collected in a memory. When last packet has been
received, it can be stored in the database and removed from the memory.

When some packets are missed especially first ones, it is impossible to
assembly them in the database record.

In this case, memory must be free implicitly by sqlInsertPackets() or parsePacket() calls
or explicitly by flushLoggerParser() call. These calls check received time and if it
is later than 5 minutes, packets trying to store in the database(if sqlInsertPackets()
called) then it removes from the memory.

- initLoggerParser() return an descriptor
- sqlCreateTable() return "CREATE TABLE .." SQL statement string
- parsePacket() trying to parse received packet. Packets stored in memory.  
- sqlInsertPackets() find completed packets
- flushLoggerParser()
- doneLoggerParser()

### Command line tools

- logger-huffman-print get packets in hex and print out temperature values

#### logger-huffman-print

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

```
./logger-huffman-print -f table 002614121f0c14261300003d3d71000100cf06aa01e6ff0002deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff0008ddff0009e3ff000adeff000bdaff000cdfff000debff000ee8ff000fcdff0010e7ff0011dfff0012ddff0013e1ff0014dcff0015dcff0016ebff0017e6ff0018dfff0019dfff001adaff001be6ff00

./logger-huffman-print -f table 00011512010115261300003e3d71000200cf06aa01e6ff0002deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff0008ddff0009e2ff000adeff000bdaff000cdfff000debff000ee8ff000fcdff0010e6ff0011dfff0012dcff0013e1ff0014dcff0015dcff0016eaff0017e5ff0018dfff0019dfff001adaff001be6ff00

./logger-huffman-print -f table 4a00800001072613002614121f0c14261300003d3d710001 4b26010200cf06aa01e6ff0002deff0003eaff0004dcff00 4b26010305e3ff0006e0ff0007e2ff0008ddff0009e3ff00 4b2601040adeff000bdaff000cdfff000debff000ee8ff00 4b2601050fcdff0010e7ff0011dfff0012ddff0013e1ff00 4b26010614dcff0015dcff0016ebff0017e6ff0018dfff00 4b26010719dfff001adaff001be6ff00

./logger-huffman-print -f table  4a0080000207261300011512010115261300003e3d710002 4b26020200cf06aa01e6ff0002deff0003eaff0004dcff00 4b26020305e3ff0006e0ff0007e2ff0008ddff0009e2ff00 4b2602040adeff000bdaff000cdfff000debff000ee8ff00 4b2602050fcdff0010e6ff0011dfff0012dcff0013e1ff00 4b26020614dcff0015dcff0016eaff0017e5ff0018dfff00 4b26020719dfff001adaff001be6ff00
```

```
echo "002614121f0c14261300003d3d71000100cf06aa01e6ff0002deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff0008ddff0009e3ff000adeff000bdaff000cdfff000debff000ee8ff000fcdff0010e7ff0011dfff0012ddff0013e1ff0014dcff0015dcff0016ebff0017e6ff0018dfff0019dfff001adaff001be6ff00" | xxd -r -p | nc -q1 -4u 84.237.104.128 5000

echo "00011512010115261300003e3d71000200cf06aa01e6ff0002deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff0008ddff0009e2ff000adeff000bdaff000cdfff000debff000ee8ff000fcdff0010e6ff0011dfff0012dcff0013e1ff0014dcff0015dcff0016eaff0017e5ff0018dfff0019dfff001adaff001be6ff00" | xxd -r -p | nc -q1 -4u 84.237.104.128 5000

echo "4a00800001072613002614121f0c14261300003d3d710001 4b26010200cf06aa01e6ff0002deff0003eaff0004dcff00 4b26010305e3ff0006e0ff0007e2ff0008ddff0009e3ff00 4b2601040adeff000bdaff000cdfff000debff000ee8ff00 4b2601050fcdff0010e7ff0011dfff0012ddff0013e1ff00 4b26010614dcff0015dcff0016ebff0017e6ff0018dfff00 4b26010719dfff001adaff001be6ff00" | xxd -r -p | nc -q1 -4u 84.237.104.128 5000

echo "4a0080000207261300011512010115261300003e3d710002 4b26020200cf06aa01e6ff0002deff0003eaff0004dcff00 4b26020305e3ff0006e0ff0007e2ff0008ddff0009e2ff00 4b2602040adeff000bdaff000cdfff000debff000ee8ff00 4b2602050fcdff0010e6ff0011dfff0012dcff0013e1ff00 4b26020614dcff0015dcff0016eaff0017e5ff0018dfff00 4b26020719dfff001adaff001be6ff00" | xxd -r -p | nc -q1 -4u 84.237.104.128 5000

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


### 
----------------OOSSmmhhddMMYYkkyyr1r2vcvbPCused
4a00280002031c140038100f160216000000003981190002 4b1c02020006cfaa0101a8000201a8000301a9000401a900 4b1c02030501a900 - 56 bytes
T ST    MMPPKKYY================================
    size общая длина данных 28h = 40 (без заголовка в 16 байт?)
        MM measure = 2 мл. Байт номера замера, lsb used (или addr_used?)
	      PP packets = 3 количество пакетов в замере
	        KK kosa = 1ch 28 идентификатор косы (номер, дата)
	          YY kosa_year = 14h = 20  год косы + 2000 Идентификатор прибора берется из паспорта косы при формате логгера, пишется из епром логгера, пишется в шапку замера.
================
                OO = 0 memblockoccupation
	              SS = 38h = 56 seconds         22.02.2022 15:16:56
	                mm = 10h = 16 minutes
	                  hh = 0fh = 15 hours
	                    dd = 16h = 22
	                      MM = 2 month 1..12
	                        YY = 16h = 22 year
	                          kk = 00 номер косы в году
	                            yy = 00 kosa_year год косы - 2000 (номер года последние 2 цифры)
	                              r1 = 0 reserved
                                    r2 = 0 reserved
	                                  vc = 39h = 57 V cc bus voltage, V
	                                    vb = 81h = 129 vbat V battery, V
	                                      PC = 19h = 25 pcnt pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
	                                        used = 512?,record number, 1..65535
