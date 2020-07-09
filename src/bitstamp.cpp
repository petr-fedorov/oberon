
#include "bitstamp.h"
#include <boost/uuid/string_generator.hpp>

#include <vector>


#include <iostream>
#include <cmath>
#include "date.h"
namespace oberon {

namespace core {

bool BitstampReconstructor::DeduceSizeBitstamp::created() {
  auto msg = dynamic_cast<ExchangeMessage *>(received_.get());
  sizes_[msg->getOrderId()] = msg->getRemainingSize();
  output_.push_back(std::move(received_));
  return true;
}

bool BitstampReconstructor::DeduceSizeBitstamp::changed() {
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

bool BitstampReconstructor::DeduceSizeBitstamp::canceled() {
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

bool BitstampReconstructor::Deduplicator::elapsed() {
#ifdef VERBOSE_BITSTAMP_DEDUPLICATOR
  std::cout << received_->toString() << "\n";
#endif
  for (auto i_other = std::begin(other_messages_);
       i_other != std::end(other_messages_); ++i_other) {
    auto change_size = (*i_other)->getChangeSize();
    // string order_id {boost::uuids::to_string((*i_other)->getOrderId())};
    if (std::isnan(change_size) || change_size > 0) {
      for (auto i_fills = std::begin(fills_); i_fills != std::end(fills_);
           ++i_fills) {
        if ((*i_other)->getOrderId() == (*i_fills)->getOrderId()) {
          if ((std::isnan(change_size) ||
               std::abs(change_size - (*i_fills)->getChangeSize()) <
                   reconstructor_.base_increment_)) {
            (*i_fills)->setRemainingSize((*i_other)->getRemainingSize());
#ifdef VERBOSE_BITSTAMP_DEDUPLICATOR
            std::cout << (*i_other)->toString() << " duplicate, dropped\n";
#endif
            //i_other = other_messages_.erase(i_other);
            (*i_other)->set_is_deleted(true);
            i_other = other_messages_.insert(i_other, std::move(*i_fills));
            fills_.erase(i_fills);
            break;
          } else {
#ifdef VERBOSE_BITSTAMP_DEDUPLICATOR
            std::cout << "Not matched\n";
            std::cout << (*i_other)->toString() << "\n";
            std::cout << (*i_fills)->toString() << "\n";
#endif
          }
        }
      }
    }
  }
  std::move(std::begin(other_messages_), std::end(other_messages_),
            std::back_inserter(output_));
  other_messages_.clear();
  output_.push_back(std::move(received_));
  if (fills_.empty())
    return true;
  else {
    for (auto &msg : fills_)
      std::cout << "F :" << msg->toString() << std::endl;
    for (auto &msg : output_)
      std::cout << "E: " << msg->toString() << std::endl;
    fills_.clear();
    return false;
  }
}

bool BitstampReconstructor::Deduplicator::filled() {
  fills_.push_back(std::unique_ptr<Filled>(dynamic_cast<Filled*>(received_.release())));
  return true;
}

bool BitstampReconstructor::Deduplicator::exchangeMessage() {
  other_messages_.push_back(std::unique_ptr<ExchangeMessage>(dynamic_cast<ExchangeMessage*>(received_.release())));
  return true;
}

BitstampReconstructor::Deduplicator::Deduplicator(const BitstampReconstructor & reconstructor): reconstructor_{reconstructor} {
}

const Volume BitstampReconstructor::BitstampMessage::getBaseMinSize() const {
  return reconstructor_.base_min_size_;
  
}

const Volume BitstampReconstructor::BitstampMessage::getBaseIncrement() const {
  return reconstructor_.base_increment_;
  
}

BitstampReconstructor::BitstampMessage::BitstampMessage(const BitstampReconstructor & reconstructor): reconstructor_{reconstructor} {
}

BitstampReconstructor::BitstampOpened::BitstampOpened(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor): BitstampMessage{reconstructor} {
  order_id_ = toUuid(tree.get<string>("id"));
  timestamp_ = toTimestamp(tree.get<string>("microtimestamp"));
  local_timestamp_ = toTimestamp(tree.get<string>("local_timestamp"));
  price_ = std::strtod(tree.get<string>("price").c_str(), nullptr);
  remaining_size_ = std::strtod(tree.get<string>("amount").c_str(), nullptr);
  change_size_ = 0;
  event_no_ = 1;
  if(tree.get<string>("order_type") == "buy")
  side_ = kBid;
  else
  side_ = kAsk;
  
}

BitstampReconstructor::BitstampOpened* BitstampReconstructor::BitstampOpened::clone() {
  return new BitstampOpened(*this);
}

BitstampReconstructor::BitstampCanceled::BitstampCanceled(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor): BitstampMessage{reconstructor} {
  order_id_ = toUuid(tree.get<string>("id"));
  timestamp_ = toTimestamp(tree.get<string>("microtimestamp"));
  local_timestamp_ = toTimestamp(tree.get<string>("local_timestamp"));
  price_ = std::strtod(tree.get<string>("price").c_str(), nullptr);
  remaining_size_ = std::strtod(tree.get<string>("amount").c_str(), nullptr);
  if (tree.get<string>("order_type") == "buy")
    side_ = kBid;
  else
    side_ = kAsk;
}

BitstampReconstructor::BitstampCanceled* BitstampReconstructor::BitstampCanceled::clone() {
  return new BitstampCanceled(*this);
}

BitstampReconstructor::BitstampFilled::BitstampFilled(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor): BitstampMessage{reconstructor} {
  order_id_ = toUuid(tree.get<string>("maker_order_id"));
  taker_order_id_ = toUuid(tree.get<string>("taker_order_id"));
  timestamp_ = toTimestamp(tree.get<string>("trade_timestamp"));
  local_timestamp_ = toTimestamp(tree.get<string>("local_timestamp"));
  price_ = std::strtod(tree.get<string>("price").c_str(), nullptr);
  change_size_ = std::strtod(tree.get<string>("amount").c_str(), nullptr);
  trade_id_ = std::stol(tree.get<string>("trade_id")); 
  if (tree.get<string>("order_type") == "buy")
    side_ = kBid;
  else
    side_ = kAsk;
}

BitstampReconstructor::BitstampFilled* BitstampReconstructor::BitstampFilled::clone() {
  return new BitstampFilled(*this);
}

BitstampReconstructor::BitstampChanged::BitstampChanged(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor): BitstampMessage{reconstructor} {
  order_id_ = toUuid(tree.get<string>("id"));
  timestamp_ = toTimestamp(tree.get<string>("microtimestamp"));
  local_timestamp_ = toTimestamp(tree.get<string>("local_timestamp"));
  price_ = std::strtod(tree.get<string>("price").c_str(), nullptr);
  remaining_size_ = std::strtod(tree.get<string>("amount").c_str(), nullptr);
  if (tree.get<string>("order_type") == "buy")
    side_ = kBid;
  else
    side_ = kAsk;
}

BitstampReconstructor::BitstampChanged* BitstampReconstructor::BitstampChanged::clone() {
  return new BitstampChanged(*this);
}

BitstampReconstructor::BitstampReconstructor( Store * store, const Volume & base_min_size, const Volume & base_increment, bool extract_only) {
  store_ = store;
  base_min_size_ = base_min_size;
  base_increment_ = base_increment;
  extract_only_ = extract_only;
  size_deducer_ =std::make_unique<DeduceSizeBitstamp>();
  deduplicator_ = std::make_unique<Deduplicator>(*this);
  taker_filter_ = std::make_unique<Classifier>(*this);
  
}

vector<std::unique_ptr<MessageHandler::Message>> BitstampReconstructor::extract(const boost::property_tree::ptree & tree) {
  using namespace date;
  using namespace std;
  string type = tree.get<string>("event");
  vector<unique_ptr<Message>> output;
  if (type == "elapsed") {
    Timestamp timestamp;
    std::istringstream ss{tree.get<string>("timestamp")};
    ss >> parse("%FT%TZ", timestamp);
    output.push_back(make_unique<Elapsed>(timestamp));
  } else if (type == "order_created")
    output.push_back(make_unique<BitstampOpened>(tree, *this));
  else if (type == "order_changed")
    output.push_back(make_unique<BitstampChanged>(tree, *this));
  else if (type == "order_deleted") {
      output.push_back(make_unique<BitstampCanceled>(tree, *this));
  } else if (type == "match") {
    output.push_back(make_unique<BitstampFilled>(tree, *this));
  }
  if (size_deducer_)
    return size_deducer_->handle(std::move(output));
  else
    return output;
}

BitstampReconstructor::Classifier::Classifier(const BitstampReconstructor & reconstructor): reconstructor_{reconstructor} {
}

bool BitstampReconstructor::Classifier::opened() {
  BitstampOpened *msg = dynamic_cast<BitstampOpened*>(received_.get());
  if(msg->getSide() == kBid)
    return classifyOpened(&bids_, &asks_);
  else
    return classifyOpened(&asks_, &bids_);
}

bool BitstampReconstructor::Classifier::changed() {
  BitstampChanged *msg = dynamic_cast<BitstampChanged *>(received_.get());
  if(msg->getSide() == kBid)
    return classifyChanged(&bids_, &asks_);
  else
    return classifyChanged(&asks_, &bids_);
}

bool BitstampReconstructor::Classifier::canceled() {
  BitstampCanceled *msg = dynamic_cast<BitstampCanceled *>(received_.get());
  if(msg->getSide() == kBid)
    return classifyCanceled(&bids_, &asks_);
  else
    return classifyCanceled(&asks_, &bids_);
  
}

bool BitstampReconstructor::Classifier::filled() {
  Filled *msg = dynamic_cast<Filled*>(received_.get());
  if(msg->getSide() == kBid)
    return classifyFilled(&bids_, &asks_);
  else
    return classifyFilled(&asks_, &bids_);
}


} // namespace oberon::core

} // namespace oberon
