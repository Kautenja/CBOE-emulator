# CBOE Emulator

[![Build Status][BuildStatus]][CIServer]

[CIServer]: https://travis-ci.com/Kautenja/CBOE-emulator
[BuildStatus]: https://travis-ci.com/Kautenja/CBOE-emulator.svg?token=FCkX2qMNHzx2qWEzZZMP&branch=master

This project implements the limit order book as an application on top of the
network stack using protocols modeled after those deployed by
[CBOE](https://www.cboe.com/). The system is asynchronous (through
[Asio](https://think-async.com/Asio/AsioStandalone.html)) and capable of
running in real-time with appropriate hardware configuration. The system
supports human interaction through command line interfaces and algorithmic
trading using the binary order entry system and data feed.

## Requirements

-   [g++](https://gcc.gnu.org/) (or [clang++](https://clang.llvm.org/))
    -   C++17 support
-   [scons](https://scons.org/)
-   uniform endian-ness of networked machines
    -   network packets are sent in host byte order for speed

## Network Optimization

Depending on your platform and environment, some setup may be required to
achieve high input throughput from listening data feeds.

### Debian

1.  maximize the UDP buffer size (check this value using `sudo sysctl`).

```shell
sudo sysctl -w net.core.rmem_max=26214400
```

2.  force explicit CPU affinity using `taskset` for each process.

### MacOS

No addition setup is necessary for MacOS platforms.

## Usage

### Vendor code

Many vendor submodules are necessary to accomplish this project, be sure to
clone them all using the following command from the top-level of the project:

```shell
git submodule init
git submodule update
```

### Testing

To run the unit test-suite for the project, run:

```shell
scons test
```

### Compilation

To compile all the application included in the project, run:

```shell
scons
```

## Order Entry Server

The order entry system presented in this document is modeled after
[Version 2.3.6 of the CBOE US Equities Binary Order Entry Specification](docs/pdf/BOE.pdf).
The order entry server application takes a single command line parameter, a
path to a JSON configuration file following the schema:

```json
{
    "port": 6001,
    "feed": {
        "group": "239.0.0.1",
        "port": 30001,
        "ttl": 0,
        "outbound_interface": "localhost"
    },
    "accounts": [
        {"username": "USR1", "password": "abadpassword"},
        {"username": "USR2", "password": "abadpassword"}
    ]
}
```

-   `port` is the port number for the order entry service that clients will
    connect to
-   `feed` is and object with parameters for the data feed
    -   `group` is the IP multi-cast group for the data feed
    -   `port` is the port to target for the multi-cast data feed
    -   `ttl` is the TTL for the data feed
    -   `outbound_interface` is the IP address of the interface to send
        packets on
-   `accounts` is a list of users that can connect and trade on the server

## Command Line Order Entry Application

The order entry system presented in this document is modeled after
[Version 2.3.6 of the CBOE US Equities Binary Order Entry Specification](docs/pdf/BOE.pdf).
The command line order entry application takes a single command line parameter,
a path to a JSON configuration file following the schema:

```json
{
    "host": "localhost",
    "port": 6001
}
```

-   `host` is the order entry server IP address to connect to
-   `port` is the port that the service is running at on the remote host

## Data Feed Receivers

The depth of book data feed presented in this document is modeled after
[Version 2.40.28 of the CBOE US Equities/Options Multicast Depth of Book (PITCH) Specification](docs/pdf/PITCH.pdf).
All the multi-cast receivers accept a JSON file with the same schema:

```json
{
    "listen": "0.0.0.0",
    "group": "239.0.0.1",
    "port": 30001
}
```

-   `listen` is the interface to listen for packets on
-   `group` is the multi-cast group to join
-   `port` is the port to bind to and receive packets on

### Applications

-   `echo` logs all messages received to the terminal
    -   the book at each tick is output as well
-   `heartbeat` refreshes the terminal with the top of book data every 1s
-   `csv` writes a CSV with tick-wise L1 bars to standard output

## Algorithmic Strategies

-   [Liquidity Consumer](include/strategies/liquidity_consumer.hpp)
-   [Binary Market Arbitrage](include/strategies/market_arbitrage.hpp)
-   [Market Maker](include/strategies/market_maker.hpp)
-   [Mean Reversion](include/strategies/mean_reversion.hpp)
-   [Momentum](include/strategies/momentum.hpp)
-   [Noise](include/strategies/noise.hpp)
