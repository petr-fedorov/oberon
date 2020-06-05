
#include "reconstructor.h"
#include "coinbase.h"

namespace oberon {

namespace core {

Event::~Event() {
}

Store::~Store() {
}

std::unique_ptr<Reconstructor> Reconstructor::create(Exchanges exchange, string pair_name, Store * store, double delay)
{
  //if(exchange == kCoinbase)
    return std::make_unique<CoinbaseReconstructor>(store);
}

Reconstructor::~Reconstructor() {
}


} // namespace oberon::core

} // namespace oberon
