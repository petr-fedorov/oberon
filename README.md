# Order Book Events ReconstructiON (OBERON)
A stand-alone application for collection of data that allows re-construction of a cryptocurrency exchange order book dynamics. Connects to an API of the specified exchange and produces `.csv` file containing reconstructed [order events]((https://petr-fedorov.github.io/oberon/methods.html#order-and-trade)) in the **exchange-independent** format described below:

|maker|ordinal|timestamp|mks|state|price|volume|change|trade|taker|heard|deleted|
| --- | ---| --- | ---| --- |---| ---| --- | ---| --- | ---| ---|
|[UUID](https://www.boost.org/doc/libs/1_67_0/libs/uuid/doc/index.html)|long| [ISO 8601](https://en.wikipedia.org/wiki/ISO_8601)|int| bool|double|double|double|long long|UUID|int|bool|

The columns contain the following data:

   * `maker` - UUID of the open order on the order book,
   * `ordinal` - a sequential number of the event (by `maker`). When an order opens on the order book, the related event has `ordinal` equals to 1; next event for `maker` will have 'ordinal' equals to 2 etc.,
   * `timestamp` - time the event occured on an exchange, up to a second; empty, if it is not changed since the previously received event,
   * `mks` - microseconds part of the time the event occured on an exchange
   * `state` - 1, if the order is open on the order book after the event, otherwise empty,
   * `price` - price of the order
   * `volume` - volume of the order; it is a negative value for asks and positive for bids,
   * `change` - a increase or decrease of the order volume relative to the previous event; it is an exchange-provided trade volume when the event is originated from a trade or a calculated value (the difference between `volume` of this and the previous event),
   * `trade` - an exchange-generated trade identification number; empty, if the event is not originated from a trade,
   * `taker` - UUID of the taker order in a trade when the event is originated from the trade, otherwise - empty,
   * `heard` - microseconds passed since the event occured on the exchange till it was received by OBERON,
   * `deleted` - 1, if the event was deleted by cleansing (for example, it was a duplicate event), otherwise - empty.

The unknown value of the field is shown by `NA`.

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

      oberon [options] <exchange name> <product>

Supported exchanges and products are shown in the table below:

| `<exchange name>` | `<product>`|
|----|---|
|Coinbase| Any pair supported by [the full channel of Coinbase's websocket feed](https://docs.pro.coinbase.com/#the-full-channel) |
|Bitstamp| Any pair supported by [Bitstamp's Websocket API v2](https://www.bitstamp.net/websocket/v2/) "Live ticker" and "Live orders" channels|
|Bitfinex| TBD|
|KuCoin|TBD|

Options are:

    -q [ --quote-increment ] arg (=0.01) specifies the minimum increment for the
                                         quote currency (i.e. USD in BTC-USD)
    -b [ --base-increment ] arg (=0.01)  specifies the minimum increment for the
                                         base currency (i.e. BTC in BTC-USD)
    -d [ --deleted ]                     output deleted events too
    -p [ --pong-wait-time ] arg (=1)     specifies the maximum waiting time in
                                         seconds for pong message after sending
                                         ping to an exchange

`quote-increment` and `base-increment` depend on the exchange and the product chosen.

Coinbase's [list of products](https://api.pro.coinbase.com/products/) contains parameters values for each pair. Bitstamp's [list of product](https://www.bitstamp.net/api/v2/trading-pairs-info/) provides  `counter_decimals` and `base_decimals` for each pair.


The output file will have the name in the following format: `<exchange name>_<product>_<timestamp>.csv`
where `timestamp` is the timestamp of an initial order book snapshot. The file starts from the events having this `timestamp` collectively producing the snapshot. The other messages in the file are updates of the snapshot.

There might be several files produced, with different `timestamp`s. That happens when an exchange websocket gets disconnected (i.e. when the exchainge does not respond to ping messages longer than `--pong-wait-time` seconds). In this case the application quitely restarts and creates a new output file.
