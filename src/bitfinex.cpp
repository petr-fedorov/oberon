
#include "bitfinex.h"
#include <vector>


#include "date.h"
namespace oberon {

namespace core {

bool DeduceSizeBitfinex::created() {
  auto msg = dynamic_cast<ExchangeMessage *>(received_.get());
  sizes_[msg->getOrderId()] = msg->getRemainingSize();
  output_.push_back(std::move(received_));
  return true;
}

bool DeduceSizeBitfinex::changed() {
  auto msg = dynamic_cast<ExchangeMessage *>(received_.get());
  try {
    msg->setChangeSize(sizes_.at(msg->getOrderId()) -
                             msg->getRemainingSize());
  } catch (const std::out_of_range &) {
  };
  sizes_[msg->getOrderId()] = msg->getRemainingSize();
  output_.push_back(std::move(received_));
  
  return true;
}

bool DeduceSizeBitfinex::canceled() {
  auto msg = dynamic_cast<ExchangeMessage *>(received_.get());
  try {
    msg->setChangeSize(sizes_.at(msg->getOrderId()) -
                             msg->getRemainingSize());
    sizes_.erase(msg->getOrderId());
  } catch (const std::out_of_range &) {
  };
  output_.push_back(std::move(received_));
  return true;
}

BitfinexReconstructor::BitfinexReconstructor( Store * store, const Volume & base_min_size, const Volume & base_increment, bool extract_only) {
  store_ = store;
  base_min_size_ = base_min_size;
  base_increment_ = base_increment;
  extract_only_ = extract_only;
  size_deducer_ =std::make_unique<DeduceSizeBitfinex>();
  size_deducer_->create();
  // Deduplication is not needed for CoinBase
  deduplicator_ = std::unique_ptr<MessageHandler>();
  
}

vector<std::unique_ptr<MessageHandler::Message>> BitfinexReconstructor::extract(const boost::property_tree::ptree & tree) {
  using namespace date;
  using namespace std;
  string type = tree.get<string>("event");
  vector<unique_ptr<Message>> output;
  if (type == "elapsed") {
    Timestamp timestamp;
    std::istringstream ss{tree.get<string>("timestamp")};
    ss >> parse("%FT%TZ", timestamp);
    output.push_back(make_unique<Elapsed>(timestamp));
  } else if (type == "order_changed")
    output.push_back(make_unique<BitfinexChanged>(tree, *this));
  else if (type == "order_deleted") {
    output.push_back(make_unique<BitfinexCanceled>(tree, *this));
  
  } else if (type == "match") {
    output.push_back(make_unique<BitfinexFilled>(tree, *this));
  }
  if (size_deducer_)
    return size_deducer_->handle(std::move(output));
  else
    return output;
}

const Volume BitfinexReconstructor::BitfinexMessage::getBaseMinSize() const {
  return reconstructor_.base_min_size_;
  
}

const Volume BitfinexReconstructor::BitfinexMessage::getBaseIncrement() const {
  return reconstructor_.base_increment_;
  
}

BitfinexReconstructor::BitfinexMessage::BitfinexMessage(const BitfinexReconstructor & reconstructor): reconstructor_{reconstructor} {
}

BitfinexReconstructor::BitfinexChanged::BitfinexChanged(const boost::property_tree::ptree & tree, const BitfinexReconstructor & reconstructor) : BitfinexMessage {reconstructor} {
  timestamp_ = toTimestamp(tree.get<string>("exchange_timestamp"));
  local_timestamp_ = toTimestamp(tree.get<string>("local_timestamp"));
  order_id_ = toUuid(tree.get<string>("order_id"));
  price_ = std::stod(tree.get<string>("price"));
  remaining_size_ = std::stod(tree.get<string>("amount"));
  if (remaining_size_ < 0) {
    remaining_size_ *= -1;
    side_ = kAsk;
  } else
    side_ = kBid;
}

MessageHandler::Changed* BitfinexReconstructor::BitfinexChanged::clone() {
  return new BitfinexChanged(*this);
}

BitfinexReconstructor::BitfinexCanceled::BitfinexCanceled(const boost::property_tree::ptree & tree, const BitfinexReconstructor & reconstructor) : BitfinexMessage {reconstructor} {
  timestamp_ = toTimestamp(tree.get<string>("exchange_timestamp"));
  local_timestamp_ = toTimestamp(tree.get<string>("local_timestamp"));
  order_id_ = toUuid(tree.get<string>("order_id"));
  if(std::stod(tree.get<string>("amount")) < 0) {
    side_ = kAsk;
  } else
    side_ = kBid;
}

MessageHandler::Canceled* BitfinexReconstructor::BitfinexCanceled::clone() {
  return new BitfinexCanceled(*this);
}

BitfinexReconstructor::BitfinexFilled::BitfinexFilled(const boost::property_tree::ptree & tree, const BitfinexReconstructor & reconstructor) : BitfinexMessage{reconstructor} {
  timestamp_ = toTimestamp(tree.get<string>("mts"));
  local_timestamp_ = toTimestamp(tree.get<string>("local_timestamp"));
  price_ = std::stod(tree.get<string>("price"));
  change_size_ = std::stod(tree.get<string>("amount"));
  if(change_size_ < 0) {
    change_size_ *= -1;
    side_ = kBid;
  }
  else
   side_ = kAsk;
  trade_id_ = std::stol(tree.get<string>("id"));
  
}

MessageHandler::Filled* BitfinexReconstructor::BitfinexFilled::clone() {
  return new BitfinexFilled(*this);
}


} // namespace oberon::core

} // namespace oberon
