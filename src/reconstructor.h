#ifndef OBERON_CORE_RECONSTRUCTOR_H
#define OBERON_CORE_RECONSTRUCTOR_H


#include <chrono>

#include <cmath>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/nil_generator.hpp>

#include <stdexcept>

#include <string>
using namespace std;
#include <boost/property_tree/ptree.hpp>


namespace oberon { namespace core { class EventImpl; }  } 
namespace oberon { namespace core { class CoinbaseReconstructor; }  } 
namespace oberon { namespace core { class BitstampReconstructor; }  } 
namespace oberon { namespace core { class BitfinexReconstructor; }  } 

namespace oberon {

namespace core {

typedef std::chrono::microseconds Duration;
typedef std::chrono::time_point<std::chrono::system_clock, Duration> Timestamp;
typedef int32_t EventNo;
typedef double Volume;
class Price {
  private:
    static const double kPricePrecision;

    static const double kPricePrecisionFraction;

    double price_;


  public:
    Price();

    Price(double price);

    inline const bool operator ==(const Price & other) const;

    inline const bool operator <(const Price & other) const;

    inline const bool operator >(const Price & other) const;

    inline  operator double() const;

    Price alignUp(double tick_size) const;

    inline Price alignDown(double tick_size) const;

};
inline const bool Price::operator ==(const Price & other) const {
  return price_ == other.price_;
}

inline const bool Price::operator <(const Price & other) const {
  return price_ < other.price_;
}

inline const bool Price::operator >(const Price & other) const {
  return price_ > other.price_;
}

inline  Price::operator double() const {
  return price_;
}

inline Price Price::alignDown(double tick_size) const {
  return Price{tick_size > kPricePrecision
                   ? std::floor((price_ + kPricePrecisionFraction) / tick_size) *
                         tick_size
                   : price_};
}

typedef boost::uuids::uuid OrderId;
typedef int32_t TradeId;
enum OrderState {
  kActive = 1,
  kFinished = 0,
  kNA = std::numeric_limits<std::int32_t>::min()
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
    static std::unique_ptr<Event> create(const OrderId & order_id, const Timestamp & timestamp, const Timestamp & local_timestamp, const EventNo & event_no, const Price & price, const Volume & volume, const Volume & delta_volume, OrderState state, const TradeId & trade_id, const OrderId & taker_order_id, bool is_deleted);

    static const Volume kNaVolume;

    static const TradeId kNaTradeId;

    static const Volume kNaPrice;

    static const OrderId kNaOrderId;

    static const EventNo kNaEventNo;

    virtual const Timestamp timestamp() const = 0;

    virtual const OrderId orderId() const = 0;

    virtual const EventNo eventNo() const = 0;

    virtual const OrderState state() const = 0;

    virtual const Price price() const = 0;

    virtual const Volume volume() const = 0;

    virtual const TradeId tradeId() const = 0;

    virtual const Volume deltaVolume() const = 0;

    virtual OrderId takerOrderId() = 0;

    virtual const Timestamp localTimestamp() const = 0;

    virtual const bool isDeleted() const = 0;

    virtual ~Event();

    virtual string toString() const = 0;

};
class Store {
  public:
    virtual void transmit( std::unique_ptr<Event> && event) = 0;

    virtual ~Store();

};
// implement the state machine Reconstructor
class Reconstructor {
  public:
    static std::unique_ptr<Reconstructor> create(Exchanges exchange, string pair_name, Store * store, double delay, bool extract_only = false);

    virtual void process(const boost::property_tree::ptree & message) = 0;

    virtual ~Reconstructor();

};

} // namespace oberon::core

} // namespace oberon
#endif
