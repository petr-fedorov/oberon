
#include "reconstructor_impl.h"
#include <boost/uuid/nil_generator.hpp>

#include <boost/uuid/uuid_io.hpp>

#include <cmath>


#include <iostream>
#include "date.h"
namespace oberon {

namespace core {

bool PriceSide::operator <(const PriceSide & rhs) const {
if (side_ == rhs.side_)
  return price_ < rhs.price_;
else {
  if (side_ == kBid)
    return true;
  else
    return false;
}
}

PriceSide::PriceSide(const Price & price, BookSide side) {
  price_ = price;
  side_ = side;
}

PriceSide::PriceSide() {
  price_ = 0;
  side_ = kBid;
}

EventImpl::EventImpl(const OrderId & order_id, const Timestamp & timestamp, const Timestamp & local_timestamp, const EventNo & event_no, const Price & price, const Volume & volume, const Volume & delta_volume, OrderState state) {
  order_id_ = order_id;
  timestamp_ = timestamp;
  local_timestamp_ = local_timestamp;
  event_no_ = event_no;
  price_ = price;
  volume_ = volume;
  delta_volume_ = delta_volume;
  state_ = state;
  trade_id_ = 0;
  taker_order_id_ = boost::uuids::nil_uuid();
}

EventImpl::EventImpl(const OrderId & order_id, const Timestamp & timestamp, const Timestamp & local_timestamp, const EventNo & event_no, const Price & price, const Volume & volume, const Volume & delta_volume, OrderState state, const TradeId & trade_id, const OrderId & taker_order_id) {
  order_id_ = order_id;
  timestamp_ = timestamp;
  local_timestamp_ = local_timestamp;
  event_no_ = event_no;
  price_ = price;
  volume_ = volume;
  delta_volume_ = delta_volume;
  state_ = state;
  trade_id_ = trade_id;
  taker_order_id_ = taker_order_id;
}

const Timestamp EventImpl::timestamp() const {
  return timestamp_;
}

const OrderId EventImpl::orderId() const {
  return order_id_;
}

const EventNo EventImpl::eventNo() const {
  return event_no_;
}

const OrderState EventImpl::state() const {
  return state_;
}

const Price EventImpl::price() const {
  return price_;
}

const Volume EventImpl::volume() const {
  return volume_;
}

const TradeId EventImpl::tradeId() const {
  return trade_id_;
}

const Volume EventImpl::deltaVolume() const {
  return delta_volume_;
}

OrderId EventImpl::takerOrderId() {
  return taker_order_id_;
}

const Timestamp EventImpl::localTimestamp() const {
  return local_timestamp_;
}

//By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
std::unique_ptr<Event> MessageHandler::Message::toEvent() {
  return std::unique_ptr<Event>();
}

string MessageHandler::Message::toString() {
  return date::format("%F %T", timestamp_);
}

MessageHandler::Message::~Message() {
}

PriceSide MessageHandler::ExchangeMessage::getPriceSide() {
return PriceSide {price_, side_};
}

void MessageHandler::ExchangeMessage::setRemainingSize(const Volume & value) {
  if(std::abs(value) > getBaseMinSize())
    remaining_size_ = value;
  else
    remaining_size_ = 0;
}

void MessageHandler::ExchangeMessage::setChangeSize(Volume value) {
  change_size_ = value;
}

void MessageHandler::ExchangeMessage::setEventNo(EventNo value) {
  event_no_ = value;
}

string MessageHandler::ExchangeMessage::toString() {
  std::stringstream buf;
  buf << Message::toString() << " " << order_id_ << " " << event_no_ << " "
      << (side_ == kBid ? 'B' : 'A') << price_ << " " << remaining_size_ << " "
      << change_size_; 
  return buf.str();
}

const Volume MessageHandler::ExchangeMessage::roundToBaseIncrement(const Volume & volume) const {
  return std::round(volume/getBaseIncrement())*getBaseIncrement();
}

//By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
std::unique_ptr<Event> MessageHandler::Created::toEvent() {
  int sign = getSide() == kAsk ? -1 : 1;
  return make_unique<EventImpl>(
      getOrderId(), getTimestamp(), getLocalTimestamp(), getEventNo(), getPrice(),
      roundToBaseIncrement(getRemainingSize()) * sign,
      roundToBaseIncrement(getChangeSize()) * sign, kActive);
}

bool MessageHandler::Created::accept(MessageHandler* mh) {
  return mh->created();
}

//By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
std::unique_ptr<Event> MessageHandler::Changed::toEvent() {
  int sign = getSide() == kAsk ? -1 : 1;
  return make_unique<EventImpl>(
      getOrderId(), getTimestamp(), getLocalTimestamp(), getEventNo(), getPrice(),
      roundToBaseIncrement(getRemainingSize()) * sign,
      roundToBaseIncrement(getChangeSize()) * sign, kActive);
}

bool MessageHandler::Changed::accept(MessageHandler* mh) {
  return mh->changed();
}

bool MessageHandler::Filled::accept(MessageHandler* mh) {
  return mh->filled();
  
}

string MessageHandler::Filled::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " " << trade_id_ << " Filled";
  return buf.str();
}

//By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
std::unique_ptr<Event> MessageHandler::Filled::toEvent() {
  int sign = getSide() == kAsk ? -1 : 1;
  OrderState state;
  if (std::isnan(getRemainingSize()))
    state = kNA;
  else if (getRemainingSize())
    state = kActive;
  else
    state = kFinished;
  return make_unique<EventImpl>(getOrderId(), getTimestamp(), getLocalTimestamp(), getEventNo(),
                                getPrice(),
                                roundToBaseIncrement(getRemainingSize()) * sign,
                                roundToBaseIncrement(getChangeSize()) * sign,
                                state, trade_id_, taker_order_id_);
}

bool MessageHandler::Opened::accept(MessageHandler* mh) {
  return mh->opened();
  
}

string MessageHandler::Opened::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " Opened";
  return buf.str();
  
}

string MessageHandler::VolumeIncremented::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " VolumeIncremented";
  return buf.str();
}

string MessageHandler::VolumeDecremented::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " PartiallyCancelled";
  return buf.str();
}

string MessageHandler::PriceAdvanced::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " PriceAdvanced";
  return buf.str();
}

string MessageHandler::PriceReceded::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " PriceReceded";
  return buf.str();
}

bool MessageHandler::Canceled::accept(MessageHandler * mh) {
  return mh->canceled();
}

string MessageHandler::Canceled::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " FullyCancelled";
  return buf.str();
}

//By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
std::unique_ptr<Event> MessageHandler::Canceled::toEvent() {
  int sign = getSide() == kAsk ? -1 : 1;
  return make_unique<EventImpl>(
      getOrderId(), getTimestamp(), getLocalTimestamp(), getEventNo(), getPrice(),
      roundToBaseIncrement(getRemainingSize()) * sign,
      roundToBaseIncrement(getChangeSize()) * sign, kFinished);
}

bool MessageHandler::Received::accept(MessageHandler * mh) {
  return mh->received();
}

string MessageHandler::Received::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " Received";
  return buf.str();
}

bool MessageHandler::Elapsed::accept( MessageHandler * mh) {
  return mh->elapsed();
}

MessageHandler::Elapsed::Elapsed(const Timestamp & exchange_time) {
  timestamp_ = exchange_time;
}

string MessageHandler::Elapsed::toString() {
  std::stringstream buf;
  buf << Message::toString() << " elapsed";
  return buf.str();
  
}

bool MessageHandler::Era::accept(MessageHandler* mh) {
}

MessageHandler::Era::Era() {
}

MessageHandler::Era::~Era() {
}

string MessageHandler::Era::toString() {
  std::stringstream buf;
  buf << Message::toString() << " era";
  return buf.str();
  
}

bool MessageHandler::create() {
  return true;
}

vector<std::unique_ptr<MessageHandler::Message>> MessageHandler::handle(vector<std::unique_ptr<MessageHandler::Message>> && messages) {
  for(auto& msg: messages) {
    if(msg) {
      received_ = std::move(msg);
#ifdef VERBOSE_STATE_MACHINE
      // Here I produced the log record in BoUML style 
      std::cout << "DEBUG : " << getHandlerName() << " received " << received_->toString() << "\n";
#endif
      received_->accept(this);
    }
  }
  return std::move(output_);
}

string MessageHandler::getHandlerName() {
  return "Default MessageHandler";
}

bool MessageHandler::message() {
  output_.push_back(std::move(received_));
  return true;
}

bool MessageHandler::elapsed() {
  return message();
}

bool MessageHandler::exchangeMessage() {
  return message();
}

bool MessageHandler::created() {
  return exchangeMessage();
  
}

bool MessageHandler::received() {
  return created();
}

bool MessageHandler::opened() {
  return created();
}

bool MessageHandler::changed() {
  return exchangeMessage();
  
}

bool MessageHandler::volumeIncremented() {
  return changed();
}

bool MessageHandler::volumeDecremented() {
  return changed();
}

bool MessageHandler::priceAdvanced() {
  return changed();
}

bool MessageHandler::priceReceded() {
  return changed();
}

bool MessageHandler::filled() {
  return exchangeMessage();
}

bool MessageHandler::canceled() {
  return exchangeMessage();
}

MessageHandler::MessageHandler() {
}

MessageHandler::~MessageHandler() {
}

bool EventNumberGenerator::exchangeMessage() {
  ExchangeMessage *em = dynamic_cast<ExchangeMessage *>(received_.get());
  try {
    em->setEventNo(eventNumbers_.at(em->getOrderId()) + 1);
    eventNumbers_[em->getOrderId()] = em->getEventNo();
  } catch (const std::out_of_range &) {
  }
  output_.push_back(std::move(received_));
  return true;
}

bool EventNumberGenerator::received() {
  ExchangeMessage *em = dynamic_cast<ExchangeMessage *>(received_.get());
  eventNumbers_[em->getOrderId()] = 0;
  em->setEventNo(0);
  output_.push_back(std::move(received_));
  return true;
}

bool EventNumberGenerator::fullyCanceled() {
  ExchangeMessage *em = dynamic_cast<ExchangeMessage *>(received_.get());
  try {
    em->setEventNo(eventNumbers_.at(em->getOrderId()) + 1);
    eventNumbers_.erase(em->getOrderId());
  } catch (const std::out_of_range &) {
  }
  output_.push_back(std::move(received_));
  return true;
}

bool EventNumberGenerator::filled() {
  ExchangeMessage *em = dynamic_cast<ExchangeMessage *>(received_.get());
  try {
    em->setEventNo(eventNumbers_.at(em->getOrderId()) + 1);
    if (!em->getRemainingSize())
      eventNumbers_.erase(em->getOrderId());
    else
      eventNumbers_[em->getOrderId()] = em->getEventNo();
  } catch (const std::out_of_range &) {
  }
  output_.push_back(std::move(received_));
  return true;
}

string EventNumberGenerator::getHandlerName() {
  return "EventNumberGenerator";
}

void ReconstructorImplementation::process(const boost::property_tree::ptree & message) {
  if (extract_only_)
    transmit(extract(message));
  else
    transmit(cleanse(extract(message)));
}

vector<std::unique_ptr<MessageHandler::Message>> ReconstructorImplementation::cleanse( vector<std::unique_ptr<MessageHandler::Message>> && messages) {
  auto output = size_deducer_->handle(move(messages));
  if(deduplicator_)
    output = deduplicator_->handle(std::move(output));
  output = event_number_generator_->handle(std::move(output));
  return output;
}


} // namespace oberon::core

} // namespace oberon
