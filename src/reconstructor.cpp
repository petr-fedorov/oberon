
#include "reconstructor.h"
#include <boost/uuid/random_generator.hpp>

#include "reconstructor_impl.h"
#include "coinbase.h"
#include "bitstamp.h"
#include "bitfinex.h"

#include <cmath>
#include <limits>
namespace oberon {

namespace core {

const double Price::kPricePrecision = 0.000001;

const double Price::kPricePrecisionFraction = kPricePrecision/4;

Price::Price(): price_ {0.0} {
}

Price::Price(double price): price_ {price} {
}

Price Price::alignUp(double tick_size) const {
  return Price{tick_size > kPricePrecision
                   ? std::ceil((price_ - kPricePrecisionFraction) / tick_size) *
                         tick_size
                   : price_};
}

std::unique_ptr<Event> Event::create(const OrderId & order_id, const Timestamp & timestamp, const Timestamp & local_timestamp, const EventNo & event_no, const Price & price, const Volume & volume, const Volume & delta_volume, OrderState state, const TradeId & trade_id, const OrderId & taker_order_id, bool is_deleted)
{
  return std::make_unique<EventImpl>(order_id, timestamp, local_timestamp, event_no, price, volume,
                   delta_volume, state, trade_id, taker_order_id, is_deleted);
}

const Volume Event::kNaVolume = std::nan("1954");

const TradeId Event::kNaTradeId = std::numeric_limits<TradeId>::min();

const Volume Event::kNaPrice = std::nan("1954");

const OrderId Event::kNaOrderId = boost::uuids::random_generator()();

const EventNo Event::kNaEventNo = std::numeric_limits<EventNo>::min();

Event::~Event() {
}

Store::~Store() {
}

std::unique_ptr<Reconstructor> Reconstructor::create(Exchanges exchange, string pair_name, Store * store, double delay, bool extract_only)
{
  if(exchange == kCoinbase)
    return std::make_unique<CoinbaseReconstructor>(store, 0.001, 0.00000001, extract_only);
  else if(exchange == kBitstamp)
    return std::make_unique<BitstampReconstructor>(store, 0.001, 0.00000001, extract_only);
  else if(exchange == kBitfinex)
    return std::make_unique<BitfinexReconstructor>(store, 0.001, 0.00000001, extract_only);
  return std::unique_ptr<Reconstructor>();
}

Reconstructor::~Reconstructor() {
}


} // namespace oberon::core

} // namespace oberon
