# logger-huffman

logger-huffman is a library and command line tool intended for decode compressed data
received from temperature logger.

The payload is decompressed and decoded according to the logger version 4 specification.

The logger passes the measurement time, the serial number of the plume (unique within a year),
the year it was manufactured (since 2000), and the temperature in degrees Celsius temperature in an array.
The array size is 1..28.

The thermometer bus voltages and the battery voltage are also transmitted.

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

- Library usage (C, C++)
- logger-huffman-print utility

### Library

Static library liblogger-huffman.a (-l logger-huffman).

There are two ways to use a library:

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

C++ source code example:

```c++
#include "logger-sql-clause.h"
...
// create table clause 
std::string r = createTableSQLClause(OUTPUT_FORMAT_SQL, dialect);
...
// load packet
LoggerKosaCollector c;
std::vector<std::string> s;
    s.push_back(hex2binString(packet0));
    LOGGER_PACKET_TYPE t = c.put(s);

...
// insert into clause
r = parsePacketsToSQLClause(OUTPUT_FORMAT_SQL, dialect, *c.koses.begin());

```

#### Second way

This way is suitable for C source code.

Header file: logger-parse.h

Source code example:

```c++
#include "logger-parse.h"

void logCallback(void *env, int level, int modulecode, int errorcode, const std::string &message)
{
      // packet parser error code and error description
}

// initialize packet parser (first parameter specify directory where logger passport resides)
void *env = initLoggerParser("", logCallback);

// Optional, get SQL "CREATE TABLE .." clauses to create an database tables 
std::vector <std::string> sqlClauses;
sqlCreateTable(sqlClauses, SQL_POSTGRESQL);

// Put received packet from the device in the loop
while (true) {
      // LoraWAN device address used for packet identification 
      uint32_t addr = ..;
      // put received packet to the parser
      parsePacket(env, addr, binaryDataString);
}

..

// process collected packets. Do it in another thread. 

std::vector <std::string> sqlInsertClauses;
sqlInsertPackets(env, sqlInsertClauses, SQL_POSTGRESQL);
for (auto it(clauses.begin()); it != clauses.end(); it++) {
    std::cout << "Insert: " << *it << std::endl;
}

// Remove processed or expired packets
rmCompletedOrExpired(env);

// finish packet parser
flushLoggerParser(env);
doneLoggerParser(env);
```

#### LoggerKosaLoader implementation

You need override load() method of LoggerKosaPacketsLoader abstract class to
implement load last "full" packets to calculate delta packet temperature.

```c++
#include "logger-kosa-loader.h"

class MyLoggerKosaPacketsLoader: public LoggerKosaPacketsLoader {
public:
    bool load(LoggerKosaPackets &retVal, uint32_t addr) override;
};

```

See "Set packet loader class"

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

##### Set packet loader class

In example below DumbLoggerKosaPacketsLoader class is set to load packets from external storage like database.
```c
#include "dumb-logger-loader.h"
...
    void *env = initLoggerParser("../logger-passport/tests/passport", nullptr);
    LoggerKosaCollector *c = (LoggerKosaCollector*) getLoggerKosaCollection(env);

    // set "base" loader
    LoggerKosaCollector lkcBase;
    lkcBase.put(42, hex2binString(packetsBase));

    DumbLoggerKosaPacketsLoader lkl;
    lkl.setCollection(&lkcBase);
    c->setLoggerKosaPacketsLoader(&lkl);
```

##### Get base SQL SELECT statement

Delta packet requires "base" packet(s).

If packet stored in SQL database, you can retrieve "base" packets with SQL statement like this:
```sql

SELECT "raw" FROM "logger_lora" WHERE "loraadr" = '2a' AND ("raw" LIKE '4a%' OR "raw" LIKE '00%' ) ORDER BY id DESC LIMIT 1;
```

buildSQLBaseMeasurementSelect() return this SQL clause using specified SQL dialect 
```c++
std::string sql = buildSQLBaseMeasurementSelect(SQL_POSTGRESQL, DEV_ADDR_UINT);
```
DEV_ADDR_UINT is LoRaWAN device address integer.

##### Prepare hex string read from database to be loaded

Call parseSQLBaseMeasurement() e.g.
```c++
#include "logger-parse.h"
std::vector<std::string> binPackets;
parseSQLBaseMeasurement(binPackets, "4a0080000207261300011512010115261300003e3d710002 4b26020200cf06aa01e6ff0002deff0003eaff0004dcff00 ...");
```

#### Compose packets

The LoggerBuilder class creates one or more binary packages (see build() methods).

The maximum length of one binary packet is 24 bytes to be sent in the LoRaWAN network.

```c++
#include "logger-builder.h"
..
```

### Command line tools

- logger-huffman-print get packets in hex and print out temperature values

#### logger-huffman-print

logger-huffman-print read binary packets from stdin:
```
cat packet-data.bin | ./logger-huffman-print
```

Also, you can pass packet(s) in the command line as hex string:
```
./logger-huffman-print <hex-data>
```

logger-huffman-print parses packets and outputs sensor temperature to stdout as JSON string.

Each sensor value must have correct with approximation as passport specify.

Option -p <path> provide path to the passport directory.

logger-huffman-print utility print out packet data to stdout using JSON or other formats (-f option):

- json - JSON string
- text - plain text
- table - tab delimited data

Output format expands with -v option.

Logger send 3 types of packets:

- 4a/4b base
- 48/49 diff
- 4c/4d huffman compressed diff

To restore "diff" packets logger-huffman-print requires "base" packets sent earlier to calc differences.

In this case you need provide "base" packets using one or more -b <hex-base-packet> options:

```
./logger-huffman-print -f json 4c620a00020126130100467cbff9fe73e67f -b 4a0080000207261300011512010115261300003e3d710002 -b 4b26020200cf06aa01e6ff0002deff0003eaff0004dcff00 -b 4b26020305e3ff0006e0ff0007e2ff0008ddff0009e2ff00 -b 4b2602040adeff000bdaff000cdfff000debff000ee8ff00 -b 4b2602050fcdff0010e6ff0011dfff0012dcff0013e1ff00 -b 4b26020614dcff0015dcff0016eaff0017e5ff0018dfff00 -b 4b26020719dfff001adaff001be6ff00  -p ../logger-passport/tests/passport
```

You can concatenate all -b option in one -b option

```
 ./logger-huffman-print -f json 4c620a00020126130100467cbff9fe73e67f -b 4a0080000207261300011512010115261300003e3d7100024b26020200cf06aa01e6ff0002deff0003eaff0004dcff004b26020305e3ff0006e0ff0007e2ff0008ddff0009e2ff004b2602040adeff000bdaff000cdfff000debff000ee8ff004b2602050fcdff0010e6ff0011dfff0012dcff0013e1ff004b26020614dcff0015dcff0016eaff0017e5ff0018dfff004b26020719dfff001adaff001be6ff00  -p ../logger-passport/tests/passport
```

Remove any spaces in -b option value. 

logger-huffman-print utility can print out SQL INSERT clause. Option -f specify SQL dialect:

- postgresql
- mysql
- firebird
- sqlite

Option -f values 
- compress
- decompress 

intended for debug purposes. 

Option "-f compress" encode data, Option "-f decompress" decode data. Data read from stdin if no hex values is provided 
in command line.  

Option -c print out SQL statements to create table in the database.

For instance, get SQL statements to create table in SQLite database:
```
./logger-huffman-print -c -f sqlite
CREATE TABLE "logger_raw"("id" integer PRIMARY KEY AUTOINCREMENT, "raw" text);
CREATE TABLE "logger_lora"("id" integer PRIMARY KEY AUTOINCREMENT, "kosa" integer, "year" integer, "no" integer, "measured" integer, "parsed" integer, "vcc" real, "vbat" real, "t" text, "tp" text, "raw" text, "th" text);
```
Option -f <sql-dialect> select target SQL dialect. 

First table "logger_raw" keep any received packet.

Second table "logger_lora" keep successfully parsed packets in one record.

Each record has 
- "t" sensor temperature (as is, no approximation)
- "tp" corrected sensor temperature (approximated by the sensor passport)
- "th" read sensor value as hex string
- "vcc" bus voltage
- "vbat" battery voltage
- "measured" measurement time, unix epoch time (seconds since Jan 1 1970)
- "parsed" received by network server time, unix epoch time (seconds since Jan 1 1970)
- "raw" packets as hex strings
- "kosa" serial number
- "year" production year since 2000 year

Examples:

Print as text
```
./logger-huffman-print -f text 486226000203261301001900000000010000000000000000
 49260202000000ff00000000000000000000000000000000
 492602030000 
-b 4a0080000207261300011512010115261300003e3d710002 -b 4b26020200cf06aa01e6ff0002deff0003eaff0004dcff00 
-b 4b26020305e3ff0006e0ff0007e2ff0008ddff0009e2ff00 -b 4b2602040adeff000bdaff000cdfff000debff000ee8ff00
-b 4b2602050fcdff0010e6ff0011dfff0012dcff0013e1ff00 -b 4b26020614dcff0015dcff0016eaff0017e5ff0018dfff00
-b 4b26020719dfff001adaff001be6ff00
```

Specify logger passport directory:
```
./logger-huffman-print -f sqlite 486226000203261301001900000000010000000000000000
 49260202000000ff00000000000000000000000000000000
 492602030000 
-b 4a0080000207261300011512010115261300003e3d710002 -b 4b26020200cf06aa01e6ff0002deff0003eaff0004dcff00 
-b 4b26020305e3ff0006e0ff0007e2ff0008ddff0009e2ff00 -b 4b2602040adeff000bdaff000cdfff000debff000ee8ff00
-b 4b2602050fcdff0010e6ff0011dfff0012dcff0013e1ff00 -b 4b26020614dcff0015dcff0016eaff0017e5ff0018dfff00
-b 4b26020719dfff001adaff001be6ff00 
-p ../logger-passport/tests/passport
INSERT INTO "logger_lora"("kosa", "year", "no", "measured", "parsed", "vcc", "vbat", "t", "tp", "raw") VALUES (28, 20, 2, 1645510616, 1650587407, 4.75, 3.30, '0,26.5,26.5,26.5625,26.5625,26.5625', '0,26.5,26.5,26.5625,26.5625,26.5625', '4a00280002031c140038100f1602161c1400003981190002 4b1c02020006cfaa0101a8000201a8000301a9000401a900 4b1c02030501a90000000000000000000000000000000000'); INSERT INTO "logger_lora"("kosa", "year", "no", "measured", "parsed", "vcc", "vbat", "t", "tp", "raw") VALUES (28, 20, 3, 1645510916, 1650587407, 4.75, 3.30, '0,26.5,26.5625,26.625,26.5,26.5625', '0,26.5,26.5625,26.625,26.5,26.5625', '4a00280003031c140038150f1602161c1400003981190003 4b1c03020006cfaa0101a8000201a9000301aa000401a800 4b1c03030501a90000000000000000000000000000000000');
```

```
./logger-huffman-print -f json 4A00280002031C140038100F160216000000003981190002 4B1C02020006CFAA0101A8000201A8000301A9000401A900 4B1C02030501A900 4A00280003031C140038150F160216000000003981190003 4B1C03020006CFAA0101A8000201A9000301AA000401A800 4B1C03030501A900
[{"id": {"kosa": 28, "measure": 2, "packet": -1, "kosa_year": 20}, "start": 1649898291, "expired": false, "completed": true, "measurement_header": {"memblockoccupation": 0, "time": 1645510616, "localtime": "2022-02-22T15:16:56", "gmt": "2022-02-22T06:16:56" , "kosa": 28, "kosa_year": 20, "vcc": 4.75, "vbat": 3.30, "pcnt": 25, "used": 2}, "packets": [{"item": {"first_packet": {"type": 74, "size": 40, "status": 0, "data_bytes": 0, "command_change": 0, "measure": 2, "packets": 3, "kosa": 28, "kosa_year": 20}, "measurement_header": {"memblockoccupation": 0, "time": 1645510616, "localtime": "2022-02-22T15:16:56", "gmt": "2022-02-22T06:16:56" , "kosa": 28, "kosa_year": 20, "vcc": 4.75, "vbat": 3.30, "pcnt": 25, "used": 2}
}}, {"item": {"second_packet": {"type": 75, "kosa": 28, "measure": 2, "packet": 2}, "measurements": [{"sensor": 0, "t": -783.6250}, {"sensor": 1, "t": -1407.9375}, {"sensor": 2, "t": -1407.9375}, {"sensor": 3, "t": -1391.9375}, {"sensor": 4, "t": -1391.9375}]}}, {"item": {"second_packet": {"type": 75, "kosa": 28, "measure": 2, "packet": 3}, "measurements": [{"sensor": 5, "t": -1391.9375}, {"sensor": 0, "t": 0.0000}, {"sensor": 0, "t": 0.0000}, {"sensor": 0, "t": 0.0000}, {"sensor": 0, "t": 0.0000}]}}]}, {"id": {"kosa": 28, "measure": 3, "packet": -1, "kosa_year": 20}, "start": 1649898291, "expired": false, "completed": true, "measurement_header": {"memblockoccupation": 0, "time": 1645510916, "localtime": "2022-02-22T15:21:56", "gmt": "2022-02-22T06:21:56" , "kosa": 28, "kosa_year": 20, "vcc": 4.75, "vbat": 3.30, "pcnt": 25, "used": 3}, "packets": [{"item": {"first_packet": {"type": 74, "size": 40, "status": 0, "data_bytes": 0, "command_change": 0, "measure": 3, "packets": 3, "kosa": 28, "kosa_year": 20}, "measurement_header": {"memblockoccupation": 0, "time": 1645510916, "localtime": "2022-02-22T15:21:56", "gmt": "2022-02-22T06:21:56" , "kosa": 28, "kosa_year": 20, "vcc": 4.75, "vbat": 3.30, "pcnt": 25, "used": 3}
}}, {"item": {"second_packet": {"type": 75, "kosa": 28, "measure": 3, "packet": 2}, "measurements": [{"sensor": 0, "t": -783.6250}, {"sensor": 1, "t": -1407.9375}, {"sensor": 2, "t": -1391.9375}, {"sensor": 3, "t": -1375.9375}, {"sensor": 4, "t": -1407.9375}]}}, {"item": {"second_packet": {"type": 75, "kosa": 28, "measure": 3, "packet": 3}, "measurements": [{"sensor": 5, "t": -1391.9375}, {"sensor": 0, "t": 0.0000}, {"sensor": 0, "t": 0.0000}, {"sensor": 0, "t": 0.0000}, {"sensor": 0, "t": 0.0000}]}}]}]
```

```
./logger-huffman-print -f table 4a00800001072613002614121f0c14261300003d3d7100014b26010200cf06aa01e6ff0002deff0003eaff0004dcff004b26010305e3ff0006e0ff0007e2ff0008ddff0009e3ff004b2601040adeff000bdaff000cdfff000debff000ee8ff004b2601050fcdff0010e7ff0011dfff0012ddff0013e1ff004b26010614dcff0015dcff0016ebff0017e6ff0018dfff004b26010719dfff001adaff001be6ff00
38	2019	1	0	1614438000	2021-02-28T00:00:00	2021-02-27T15:00:00	0	0	0	0	0	0	0	108.938	1	-1.625	2	-2.125	3	-1.375	4	-2.25	5	-1.8125	6	-2	7	-1.875	8	-2.1875	9	-1.8125	10	-2.125	11	-2.375	12	-2.0625	13	-1.3125	14	-1.5	15	-3.1875	16	-1.5625	17	-2.0625	18	-2.1875	19	-1.9375	20	-2.25	21	-2.25	22	-1.3125	23	-1.625	24	-2.0625	25	-2.0625	26	-2.375	27	-1.625	
```

```
./logger-huffman-print -f table 002614121f0c14261300003d3d71000100cf06aa01e6ff00 02deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff00 08ddff0009e3ff000adeff000bdaff000cdfff000debff00 0ee8ff000fcdff0010e7ff0011dfff0012ddff0013e1ff00 14dcff0015dcff0016ebff0017e6ff0018dfff0019dfff00 1adaff001be6ff00
N1-0 #0	2020-12-31T18:20:38+09	-783.6250	-400.0625	-528.0625	-336.0625	-560.0625	-448.0625	-496.0625	-464.0625	-544.0625	-448.0625	-528.0625	-592.0625	-512.0625	-320.0625	-368.0625	-800.0625	-384.0625	-512.0625	-544.0625	-480.0625	-560.0625	-560.0625	-320.0625	-400.0625	-512.0625	-512.0625	-592.0625	-400.0625
```
More examples:
```
./logger-huffman-print -f table 00011512010115261300003e3d71000200cf06aa01e6ff0002deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff0008ddff0009e2ff000adeff000bdaff000cdfff000debff000ee8ff000fcdff0010e6ff0011dfff0012dcff0013e1ff0014dcff0015dcff0016eaff0017e5ff0018dfff0019dfff001adaff001be6ff00
N2-0 #0	2021-01-01T18:21:01+09	-783.6250	-400.0625	-528.0625	-336.0625	-560.0625	-448.0625	-496.0625	-464.0625	-544.0625	-464.0625	-528.0625	-592.0625	-512.0625	-320.0625	-368.0625	-800.0625	-400.0625	-512.0625	-560.0625	-480.0625	-560.0625	-560.0625	-336.0625	-416.0625	-512.0625	-512.0625	-592.0625	-400.0625

./logger-huffman-print -f table 4a00800001072613002614121f0c14261300003d3d710001 4b26010200cf06aa01e6ff0002deff0003eaff0004dcff00 4b26010305e3ff0006e0ff0007e2ff0008ddff0009e3ff00 4b2601040adeff000bdaff000cdfff000debff000ee8ff00 4b2601050fcdff0010e7ff0011dfff0012ddff0013e1ff00 4b26010614dcff0015dcff0016ebff0017e6ff0018dfff00 4b26010719dfff001adaff001be6ff00
./logger-huffman-print -f table 4a00800001072613002614121f0c14261300003d3d710001 4b26010200cf06aa01e6ff0002deff0003eaff0004dcff00 4b26010305e3ff0006e0ff0007e2ff0008ddff0009e3ff00 4b2601040adeff000bdaff000cdfff000debff000ee8ff00 4b2601050fcdff0010e7ff0011dfff0012ddff0013e1ff00 4b26010614dcff0015dcff0016ebff0017e6ff0018dfff00 4b26010719dfff001adaff001be6ff00
N19-38 #1	2021-01-31T18:20:38+09	-783.6250	-400.0625	-528.0625	-336.0625	-560.0625	-448.0625	-496.0625	-464.0625	-544.0625	-448.0625	-528.0625	-592.0625	-512.0625	-320.0625	-368.0625	-800.0625	-384.0625	-512.0625	-544.0625	-480.0625	-560.0625	-560.0625	-320.0625	-400.0625	-512.0625	-512.0625	-592.0625	-400.0625

./logger-huffman-print -f table  4a0080000207261300011512010115261300003e3d710002 4b26020200cf06aa01e6ff0002deff0003eaff0004dcff00 4b26020305e3ff0006e0ff0007e2ff0008ddff0009e2ff00 4b2602040adeff000bdaff000cdfff000debff000ee8ff00 4b2602050fcdff0010e6ff0011dfff0012dcff0013e1ff00 4b26020614dcff0015dcff0016eaff0017e5ff0018dfff00 4b26020719dfff001adaff001be6ff00
N19-38 #2	2021-02-01T18:21:01+09	-783.6250	-400.0625	-528.0625	-336.0625	-560.0625	-448.0625	-496.0625	-464.0625	-544.0625	-464.0625	-528.0625	-592.0625	-512.0625	-320.0625	-368.0625	-800.0625	-400.0625	-512.0625	-560.0625	-480.0625	-560.0625	-560.0625	-336.0625	-416.0625	-512.0625	-512.0625	-592.0625	-400.0625
```

Read packet from stdin and send to the network server at address 84.237.104.128 port 5000:

```
echo "002614121f0c14261300003d3d71000100cf06aa01e6ff0002deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff0008ddff0009e3ff000adeff000bdaff000cdfff000debff000ee8ff000fcdff0010e7ff0011dfff0012ddff0013e1ff0014dcff0015dcff0016ebff0017e6ff0018dfff0019dfff001adaff001be6ff00" | xxd -r -p | nc -q1 -4u 84.237.104.128 5000

echo "00011512010115261300003e3d71000200cf06aa01e6ff0002deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff0008ddff0009e2ff000adeff000bdaff000cdfff000debff000ee8ff000fcdff0010e6ff0011dfff0012dcff0013e1ff0014dcff0015dcff0016eaff0017e5ff0018dfff0019dfff001adaff001be6ff00" | xxd -r -p | nc -q1 -4u 84.237.104.128 5000

echo "4a00800001072613002614121f0c14261300003d3d710001 4b26010200cf06aa01e6ff0002deff0003eaff0004dcff00 4b26010305e3ff0006e0ff0007e2ff0008ddff0009e3ff00 4b2601040adeff000bdaff000cdfff000debff000ee8ff00 4b2601050fcdff0010e7ff0011dfff0012ddff0013e1ff00 4b26010614dcff0015dcff0016ebff0017e6ff0018dfff00 4b26010719dfff001adaff001be6ff00" | xxd -r -p | nc -q1 -4u 84.237.104.128 5000

echo "4a0080000207261300011512010115261300003e3d710002 4b26020200cf06aa01e6ff0002deff0003eaff0004dcff00 4b26020305e3ff0006e0ff0007e2ff0008ddff0009e2ff00 4b2602040adeff000bdaff000cdfff000debff000ee8ff00 4b2602050fcdff0010e6ff0011dfff0012dcff0013e1ff00 4b26020614dcff0015dcff0016eaff0017e5ff0018dfff00 4b26020719dfff001adaff001be6ff00" | xxd -r -p | nc -q1 -4u 84.237.104.128 5000

```

xxd -r -p is used to convert hex string to binary.

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

Configure and make project using Autotools without logger passwords library:
```
cd logger-huffman
./configure
make
```

Configure and make project using Autotools without logger-password library:
```
cd logger-huffman
./configure --enable-logger-passport
make
```

Option --enable-logger-passord depends on liblogger-passport.a, add library to the ../logger-passord directory.

If you prefer use clang instead of gcc:

```
./configure CC=clang CXX=clang++
```

You can install library and utilities in the system:
```
sudo make install
```

### Cmake

On Windows, you need install vcpkg. Do not forget integrate vcpkg with Visual Studio:

```
.\vcpkg\vcpkg integrate install
```

Enable logger passport feature

```
cmake -DENABLE_LOGGER_PASSPORT=ON ..
```

Then build solution:

```
mkdir build
cd build
cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_TOOLCHAIN_FILE=C:/git/vcpkg/scripts/buildsystems/vcpkg.cmake ..
```


On Linux you can use CMake instead of Automake:

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

### Huffman codes
 
| Symbol | Code    | Prefix |
|--------|---------|--------|
| 0      | 1       |        |
| 1      | 0010    |        |
| 2      | 00001   |        |
| 3      | 000001  |        |
| 4      | 0000000 |        |
| fc     | 0000001 |        |
| fd     | 00010   |        |
| fe     | 00011   |        |
| ff     | 0011    |        |
| 8      | 01      |*       |

Prefix of the byte itself (which is not in the table)

### References

- [lorawan-network-server](https://github.com/commandus/lorawan-network-server)
- [lorawan-ws](https://github.com/commandus/lorawan-ws)


#### Calc bus and battery voltages

```
Single VccReal = Convert.ToSingle(94 - head[11]) * 0.05814F + 2.6F; 
Single VbatReal = (Single)((double)(head[12]* 4) * 1100.0 / 1023.0 * 6.1 / 1000.0 - 0.08);
```

#### Packet 4a example

```
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
```
#### Packet 4c example

First packet with 8 bytes uncompressed header and compressed measure header and data:

```
FIRST_HDR_8BYTES Huffman encoded data
T_STsizeMMPPKKYY UUss----------------
4c620a0002012613 0100467cbff9fe73e67f
                 UU = 01 used (measure number lat byte)
                   ss = 00 delta sec
                     --- encoded ---
```

After decompression data can have one extra data element. It is compression artefact, just skip it.

```
FIRST_HDR_8BYTES  MEASURE_HDR_10_BYTES                       10                  20
0 1 2 3 4 5 6 7   8 9 0 1 2 3 4 5 6 7    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
T_STsizeMMPPKKYY  OOSSmmhhddMMYYkkyyr1  
4c620a0002012613  01001900000000010000  000000000000000000ff000000000000ff00ff000000ffff0000000000
== First packet header ==
T_ = 4c = huffman delta packet 1
  ST = 62 = status
    size = 0a00h общая длина данных
        MM measure = 02 мл. Байт номера замера, lsb used (или addr_used?)
	      PP packets = 01 количество пакетов в замере
	        KK kosa = 26h идентификатор косы (номер, дата)
	          YY kosa_year = 13h = 19  год косы + 2000 Идентификатор прибора берется из паспорта косы при формате логгера, пишется из епром логгера, пишется в шапку замера.
                == Measure header ==
                OOSSmmhhddMMYYkkyyr1
                fd80000401ffff000000
                O1 = fd memblockoccupation
	              SS = 00h
	                mm = 19h minutes
	                  hh = 00h  hours
	                    dd = 10h
	                      MM = 0 month 1..12
	                        YY = 0h = 22 year
	                          kk = 01 номер косы в году
	                            yy = 00 kosa_year год косы - 2000 (номер года последние 2 цифры)
	                              r1 = 0 reserved
```
