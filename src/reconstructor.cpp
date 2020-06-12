
#include "reconstructor.h"
#include <boost/uuid/random_generator.hpp>

#include "coinbase.h"

#include <cmath>
#include <limits>
namespace oberon {

namespace core {

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
  //if(exchange == kCoinbase)
    return std::make_unique<CoinbaseReconstructor>(store, 0.001, 0.00000001, extract_only);
}

Reconstructor::~Reconstructor() {
}


} // namespace oberon::core

} // namespace oberon
