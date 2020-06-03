#ifndef OBERON_CORE_RECONSTRUCTOR_H
#define OBERON_CORE_RECONSTRUCTOR_H


#include <chrono>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>

#include <stdexcept>

#include <string>
using namespace std;
#include <boost/property_tree/ptree.hpp>


namespace oberon { namespace core { class CoinbaseReconstructor; }  } 

namespace oberon {

namespace core {

typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> Timestamp;
typedef unsigned long EventNo;
typedef double Volume;
typedef double Price;
typedef boost::uuids::uuid Id;
enum EventType {
  kActivation = 1,
  kDeactivation = 0
};
enum Exchanges {
  kCoinbase,
  kBitstamp,
  kBitfinex
};
//Thrown when a message is received out of time order 
class time_order_error : public std::logic_error {
};
class Event {
  public:
    virtual const Timestamp getTimestamp() const = 0;

    virtual const Id getOrderId() const = 0;

    virtual const EventNo getEventNo() const = 0;

    virtual const Price getPrice() const = 0;

    virtual const Volume getVolume() const = 0;

    virtual const Id getTradeId() const = 0;

    virtual const EventType getEventType() const = 0;

    virtual ~Event();

};
class Store {
  public:
    virtual void transmit( std::unique_ptr<Event> && event) = 0;

    virtual ~Store();

};
// implement the state machine Reconstructor
class Reconstructor {
  public:
    static std::unique_ptr<Reconstructor> create(Exchanges exchange, string pair_name, Store * store, double delay);

    virtual void process(const boost::property_tree::ptree & message) = 0;

    virtual ~Reconstructor();

};
typedef std::chrono::microseconds Duration;

} // namespace oberon::core

} // namespace oberon
#endif
