
#include "reconstructor_impl.h"
#include <boost/uuid/nil_generator.hpp>

#include <boost/uuid/uuid_io.hpp>


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

EventImpl::EventImpl(const Id & order_id, const Timestamp & timestamp, const EventNo & event_no, const Price & price, const Volume & volume, EventType event_type) {
  order_id_ = order_id;
  timestamp_ = timestamp;
  event_no_ = event_no;
  price_ = price;
  volume_ = volume;
  event_type_ = event_type;
  boost::uuids::nil_generator gen;
  trade_id_ = gen();
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

void MessageHandler::ExchangeMessage::setRemainingSize(Volume value) {
  remaining_size_ = value;
}

void MessageHandler::ExchangeMessage::setChangeSize(Volume value) {
  change_size_ = value;
}

string MessageHandler::ExchangeMessage::toString() {
  std::stringstream buf;
  buf << Message::toString() << " " << order_id_ << " " << (side_ ==
          kBid
      ? 'B'
      : 'A') << price_ << " " << remaining_size_;
  return buf.str();
}

bool MessageHandler::Filled::accept(MessageHandler* mh) {
  return mh->filled();
  
}

string MessageHandler::Filled::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " Filled";
  return buf.str();
  
}

bool MessageHandler::Opened::accept(MessageHandler* mh) {
  return mh->opened();
  
}

string MessageHandler::Opened::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " Opened";
  return buf.str();
  
}

//By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
std::unique_ptr<Event> MessageHandler::Opened::toEvent() {
  return make_unique<EventImpl>(getOrderId(), getTimestamp(), getEventNo(), 
      getPrice(), getRemainingSize(), kActivation);
}

string MessageHandler::VolumeIncremented::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " VolumeIncremented";
  return buf.str();
}

string MessageHandler::PartiallyCanceled::toString() {
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

bool MessageHandler::FullyCanceled::accept(MessageHandler * mh) {
  return mh->fullyCanceled();
}

string MessageHandler::FullyCanceled::toString() {
  std::stringstream buf;
  buf << ExchangeMessage::toString() << " FullyCancelled";
  return buf.str();
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

MessageHandler::Elapsed::~Elapsed() {
  std::cout << "Elapsed deleted\n";
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

bool MessageHandler::create() {
}

void ReconstructorImplementation::process(const boost::property_tree::ptree & message) {
  save(cleanse(extract(message)));
  
}

vector<std::unique_ptr<MessageHandler::Message>> ReconstructorImplementation::cleanse( vector<std::unique_ptr<MessageHandler::Message>> && messages) {
  auto output = std::move(size_deducer_->handle(std::move(messages)));
  return deduplicator_->handle(std::move(output));
}


} // namespace oberon::core

} // namespace oberon
