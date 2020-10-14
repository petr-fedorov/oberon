# Order Book Events ReconstructiON (OBERON)
A stand-alone application for collection of data that allows re-construction of a cryptocurrency exchange order book dynamics. Connects to an API of the specified exchange and produces `.csv` file containing re-constructed *order events* in an exchange-independent format:

|i|n|t|s|p|v|f|delta_v|taker_i|local_t|
| --- | ---| --- | ---| --- | ---| --- | ---| --- | ---|
|[UUID](https://www.boost.org/doc/libs/1_67_0/libs/uuid/doc/index.html)|uint32_t|timestamp|bool|double|double|uint64_t|double|UUID|timestamp|

The content of the first seven columns is described in [Definition 2.11 (Order Event)](https://petr-fedorov.github.io/oberon/methods.html#order-and-trade). Three additional columns contain the following data:

   * `delta_v` - change of the order `i` volume relative to the previous event - a derived value that equals to a trade volume if the event is originated from the trade,
   * `taker_id` - if the event is originated from a trade then it is the id of the taker order in the trade, otherwise - empty,
   * `local_t` - a timestamp revealing the moment when information about the event was received by the application, so the delay of information delivery can be easily seen.


 See [here](https://petr-fedorov.github.io/oberon/) to understand how to re-construct the order book dynamics from these events.

## Installation

The installation instructions are for Debian 10 "buster".

The application is designed in BOUML -  a free UML 2 tool box including a modeler allowing you to specify and generate code in C++, Java, Idl, Php, Python and MySQL.

1. Install BOUML v7.10 or later as described [here](https://www.bouml.fr/download.html).
2. Install C++ Tool chain, OpenSSL, Boost and Qmake:

       apt install g++
       apt install qt4-qmake
       apt install libssl-dev
       apt install libboost-system-dev libboost-log-dev
3. Launch BOUML and open `bouml/oberon/oberon.prj` project
4. Generate C++ code for 'Standalone application' and `.pro` file for `<executable>oberon`. The code and .pro file will be generated in `/tmp/oberon/cpp` folder.

5. Make `build` folder and build the application:

       mkdir /tmp/oberon/build
       cd /tmp/oberon/build
       qmake /tmp/oberon/cpp/oberon.pro
       make

## Usage

      oberon <exchange name> <pair name>

Supported parameter pairs are shown below:

| `<exchange name>` | `<pair name>`|
|----|---|
|coinbase| Any pair supported by [the full channel of Coinbase's websocket feed](https://docs.pro.coinbase.com/#the-full-channel) |

The output file will have the name in the following format: `<exchange name>_<pair_name>_<timestamp>.csv`
where `timestamp` is the timestamp of an initial order book snapshot. The file starts from the events having this `timestamp` collectively producing the snapshot. The other messages in the file are updates of the snapshot.

There might be several files produced, with different `timestamp`s. That happens when an exchange websocket dries up. In this case the application quitely restarts and starts a new output file.
