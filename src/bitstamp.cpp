
#include "bitstamp.h"
#include <boost/uuid/string_generator.hpp>

#include <iomanip>


#include <iostream>
#include "date.h"
namespace oberon {

namespace core {

Dedup::AnyState::~AnyState() {
}

// perform the 'do activity'
void Dedup::AnyState::_do(Dedup &) {
}

// the current state doesn't manage the event filled, give it to the upper state
void Dedup::AnyState::filled(Dedup & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->filled(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition filled not expected" << std::endl;
#endif
}

// the current state doesn't manage the event cancelled, give it to the upper state
void Dedup::AnyState::cancelled(Dedup & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->cancelled(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition cancelled not expected" << std::endl;
#endif
}

// the current state doesn't manage the event stop, give it to the upper state
void Dedup::AnyState::stop(Dedup & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->stop(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition stop not expected" << std::endl;
#endif
}

// the current state doesn't manage the event message, give it to the upper state
void Dedup::AnyState::message(Dedup & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->message(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition message not expected" << std::endl;
#endif
}

void Dedup::AnyState::create(Dedup &) {
}

// the current state doesn't manage the event elapsed, give it to the upper state
void Dedup::AnyState::elapsed(Dedup & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->elapsed(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition elapsed not expected" << std::endl;
#endif
}

Dedup::Dedup_State::Pristine_State::~Pristine_State() {
}

// to manage the event filled
void Dedup::Dedup_State::Pristine_State::filled(Dedup & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition elapsed" << std::endl;
#endif
  stm.deferExchangeMessage(true);
      stm._dedup_state._contaminated_state._doentry(stm);
      stm._set_currentState(stm._dedup_state._contaminated_state._fills_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Dedup.Contaminated.Fills" << std::endl;
#endif
    }
}

// to manage the event cancelled
void Dedup::Dedup_State::Pristine_State::cancelled(Dedup & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition cancelled" << std::endl;
#endif
  stm.deferExchangeMessage(true);
      stm._dedup_state._contaminated_state._doentry(stm);
      stm._set_currentState(stm._dedup_state._contaminated_state._cancellations_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Dedup.Contaminated.Cancellations" << std::endl;
#endif
    }
}

// to manage the event stop
void Dedup::Dedup_State::Pristine_State::stop(Dedup & stm) {
    {
      stm._set_currentState(stm._dedup_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Dedup" << std::endl;
#endif
      stm._final();
    }
}

// to manage the event message
void Dedup::Dedup_State::Pristine_State::message(Dedup & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition message" << std::endl;
#endif
  stm.output_.push_back(std::move(stm.received_));
    }
}

// to manage the event create
void Dedup::Dedup_State::Pristine_State::create(Dedup & stm) {
  	_doentry(stm);
}

// perform the 'entry behavior'
void Dedup::Dedup_State::Pristine_State::_doentry(Dedup & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute entry behavior of .Dedup.Pristine" << std::endl;
#endif
  stm.contamination_period_[0] = Timestamp();
  stm.contamination_period_[1] = Timestamp();
}

// returns the state containing the current
Dedup::AnyState * Dedup::Dedup_State::Pristine_State::_upper(Dedup & stm) {
    return &stm._dedup_state;
}

Dedup::Dedup_State::Contaminated_State::Fills_and_Cancellations_State::~Fills_and_Cancellations_State() {
}

// to manage the event elapsed
void Dedup::Dedup_State::Contaminated_State::Fills_and_Cancellations_State::elapsed(Dedup & stm) {
    if (stm.received_->getTimestamp() > stm.contamination_period_[1] + stm.wait_duration_
  ) {
      stm._dedup_state._contaminated_state._doexit(stm);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition elapsed, cleanse" << std::endl;
#endif
  stm.cleanseMessages();    stm._set_currentState(stm._dedup_state._pristine_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Dedup.Pristine" << std::endl;
#endif
      stm._dedup_state._pristine_state.create(stm);
    }
}

// to manage the event filled
void Dedup::Dedup_State::Contaminated_State::Fills_and_Cancellations_State::filled(Dedup & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition <transition>" << std::endl;
#endif
  stm.deferExchangeMessage(true);  }
}

// to manage the event cancelled
void Dedup::Dedup_State::Contaminated_State::Fills_and_Cancellations_State::cancelled(Dedup & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition <transition>" << std::endl;
#endif
  stm.deferExchangeMessage(true);  }
}

// returns the state containing the current
Dedup::AnyState * Dedup::Dedup_State::Contaminated_State::Fills_and_Cancellations_State::_upper(Dedup & stm) {
    return &stm._dedup_state._contaminated_state;
}

Dedup::Dedup_State::Contaminated_State::Fills_State::~Fills_State() {
}

// to manage the event cancelled
void Dedup::Dedup_State::Contaminated_State::Fills_State::cancelled(Dedup & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition cancelled" << std::endl;
#endif
  stm.deferExchangeMessage(true);    stm._set_currentState(stm._dedup_state._contaminated_state._fills_and_cancellations_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Dedup.Contaminated.Fills and Cancellations" << std::endl;
#endif
    }
}

// to manage the event filled
void Dedup::Dedup_State::Contaminated_State::Fills_State::filled(Dedup & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition <transition>" << std::endl;
#endif
  stm.deferExchangeMessage(true);  }
}

// returns the state containing the current
Dedup::AnyState * Dedup::Dedup_State::Contaminated_State::Fills_State::_upper(Dedup & stm) {
    return &stm._dedup_state._contaminated_state;
}

Dedup::Dedup_State::Contaminated_State::Cancellations_State::~Cancellations_State() {
}

// to manage the event filled
void Dedup::Dedup_State::Contaminated_State::Cancellations_State::filled(Dedup & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition filled" << std::endl;
#endif
  stm.deferExchangeMessage(true);    stm._set_currentState(stm._dedup_state._contaminated_state._fills_and_cancellations_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Dedup.Contaminated.Fills and Cancellations" << std::endl;
#endif
    }
}

// to manage the event cancelled
void Dedup::Dedup_State::Contaminated_State::Cancellations_State::cancelled(Dedup & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition <transition>" << std::endl;
#endif
  stm.deferExchangeMessage(true);  }
}

// returns the state containing the current
Dedup::AnyState * Dedup::Dedup_State::Contaminated_State::Cancellations_State::_upper(Dedup & stm) {
    return &stm._dedup_state._contaminated_state;
}

Dedup::Dedup_State::Contaminated_State::~Contaminated_State() {
}

// to manage the event elapsed
void Dedup::Dedup_State::Contaminated_State::elapsed(Dedup & stm) {
    if (stm.received_->getTimestamp() <= stm.contamination_period_[1] + stm.wait_duration_
  ) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition elapsed, ignored" << std::endl;
#endif
  //do nothing
    }
    else {
      stm._dedup_state._contaminated_state._doexit(stm);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition elapsed, pass" << std::endl;
#endif
  stm.passDeferred();    stm._set_currentState(stm._dedup_state._pristine_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Dedup.Pristine" << std::endl;
#endif
      stm._dedup_state._pristine_state.create(stm);
    }
}

// perform the 'entry behavior'
void Dedup::Dedup_State::Contaminated_State::_doentry(Dedup & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute entry behavior of .Dedup.Contaminated" << std::endl;
#endif
  stm.contamination_period_[0] = stm.received_->getTimestamp();
  stm.contamination_period_[1] = stm.contamination_period_[0];
  stm.orchestrator_->setOutputBoundary(stm.contamination_period_[0]);
}

// perform the 'exit behavior'
void Dedup::Dedup_State::Contaminated_State::_doexit(Dedup & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute exit behavior of .Dedup.Contaminated" << std::endl;
#endif
  stm.orchestrator_->releaseOutputBoundary(stm.contamination_period_[0]);
  stm.contamination_period_[0] = Timestamp();
  stm.contamination_period_[1] = Timestamp();
  
}

// returns the state containing the current
Dedup::AnyState * Dedup::Dedup_State::Contaminated_State::_upper(Dedup & stm) {
    return &stm._dedup_state;
}

// to manage the event message
void Dedup::Dedup_State::Contaminated_State::message(Dedup & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition message" << std::endl;
#endif
  ExchangeMessage *ptr = dynamic_cast<ExchangeMessage *>(stm.received_.release());
  assert(ptr);
  stm.deferred_.push_back(std::unique_ptr<ExchangeMessage>(ptr));
  ;
    }
}

Dedup::Dedup_State::~Dedup_State() {
}

// to manage the event create
void Dedup::Dedup_State::create(Dedup & stm) {
    {
      stm._set_currentState(stm._dedup_state._pristine_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Dedup.Pristine" << std::endl;
#endif
      stm._dedup_state._pristine_state.create(stm);
    }
}

// returns the state containing the current
Dedup::AnyState * Dedup::Dedup_State::_upper(Dedup &) {
    return 0;
}

Dedup::Dedup() {
    _current_state = 0;
}

Dedup::Dedup(DedupOrchestrator * orchestrator) : Dedup() {
  orchestrator_ = orchestrator;
}

Dedup::~Dedup() {
  // std::cout << "Dedup deleted\n";
}

// the operation you call to signal the event elapsed
bool Dedup::elapsed() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger elapsed" << std::endl;
#endif
      _current_state->elapsed(*this);
    }
    return (_current_state != 0);
}

bool Dedup::received() {
  return message();
}

bool Dedup::opened() {
  return message();
}

bool Dedup::volumeIncremented() {
  return message();
}

bool Dedup::priceAdvanced() {
  return message();
}

bool Dedup::priceReceded() {
  return message();
}

// the operation you call to signal the event filled
bool Dedup::filled() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger filled" << std::endl;
#endif
      _current_state->filled(*this);
    }
    return (_current_state != 0);
}

bool Dedup::partiallyCanceled() {
  return cancelled();
}

bool Dedup::fullyCanceled() {
  return cancelled();
}

void Dedup::deferExchangeMessage(bool update_contaminated_period) {
}

void Dedup::cleanseMessages() {
}

void Dedup::passDeferred() {
}

// the operation you call to signal the event cancelled
bool Dedup::cancelled() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger cancelled" << std::endl;
#endif
      _current_state->cancelled(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event stop
bool Dedup::stop() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger stop" << std::endl;
#endif
      _current_state->stop(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event message
bool Dedup::message() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger message" << std::endl;
#endif
      _current_state->message(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event create
bool Dedup::create() {
  if (_current_state == 0)
    (_current_state = &(*this)._dedup_state)->create(*this);
  return (_current_state != 0);
}

// to execute the current state 'do activity'
void Dedup::doActivity() {
    _current_state->_do(*this);
}

// execution done, internal
void Dedup::_final() {
    _current_state = 0;
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : final state reached" << std::endl;
#endif
}

string Dedup::getHandlerName() {
  return "Dedup";
}

DedupOrchestrator::AnyState::~AnyState() {
}

// perform the 'do activity'
void DedupOrchestrator::AnyState::_do(DedupOrchestrator &) {
}

// the current state doesn't manage the event stop, give it to the upper state
void DedupOrchestrator::AnyState::stop(DedupOrchestrator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->stop(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition stop not expected" << std::endl;
#endif
}

// the current state doesn't manage the event message, give it to the upper state
void DedupOrchestrator::AnyState::message(DedupOrchestrator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->message(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition message not expected" << std::endl;
#endif
}

// the current state doesn't manage the event elapsed, give it to the upper state
void DedupOrchestrator::AnyState::elapsed(DedupOrchestrator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->elapsed(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition elapsed not expected" << std::endl;
#endif
}

void DedupOrchestrator::AnyState::create(DedupOrchestrator &) {
}

DedupOrchestrator::DedupOrchestrator_State::Wait_State::~Wait_State() {
}

// to manage the event stop
void DedupOrchestrator::DedupOrchestrator_State::Wait_State::stop(DedupOrchestrator & stm) {
    {
      stm._set_currentState(stm._deduporchestrator_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .DedupOrchestrator" << std::endl;
#endif
      stm._final();
    }
}

// to manage the event message
void DedupOrchestrator::DedupOrchestrator_State::Wait_State::message(DedupOrchestrator & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition message" << std::endl;
#endif
  ExchangeMessage *ptr = dynamic_cast<ExchangeMessage *>(stm.received_.release());
  assert(ptr);
  stm.episode_.push_back(std::unique_ptr<ExchangeMessage>(ptr));
    }
}

// to manage the event elapsed
void DedupOrchestrator::DedupOrchestrator_State::Wait_State::elapsed(DedupOrchestrator & stm) {
    {
      stm._set_currentState(stm._deduporchestrator_state._deduplicate_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .DedupOrchestrator.Deduplicate" << std::endl;
#endif
      stm._deduporchestrator_state._deduplicate_state.create(stm);
    }
}

// returns the state containing the current
DedupOrchestrator::AnyState * DedupOrchestrator::DedupOrchestrator_State::Wait_State::_upper(DedupOrchestrator & stm) {
    return &stm._deduporchestrator_state;
}

DedupOrchestrator::DedupOrchestrator_State::Deduplicate_State::~Deduplicate_State() {
}

bool DedupOrchestrator::DedupOrchestrator_State::Deduplicate_State::_completion(DedupOrchestrator & stm) {
    {
      stm._deduporchestrator_state._deduplicate_state._doexit(stm);
      stm._set_currentState(stm._deduporchestrator_state._wait_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .DedupOrchestrator.Wait" << std::endl;
#endif
      return (bool) 1;
    }
}

// to manage the event create
void DedupOrchestrator::DedupOrchestrator_State::Deduplicate_State::create(DedupOrchestrator & stm) {
  	_doentry(stm);
  	_completion(stm);
}

// perform the 'entry behavior'
void DedupOrchestrator::DedupOrchestrator_State::Deduplicate_State::_doentry(DedupOrchestrator & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute entry behavior of .DedupOrchestrator.Deduplicate" << std::endl;
#endif
  for (auto &msg : stm.episode_) {
    ExchangeMessage *emsg = dynamic_cast<ExchangeMessage *>(msg.get());
    Dedup *dedup;
    try {
      dedup = stm.dedups_.at(emsg->getPriceSide()).get();
    } catch (const std::out_of_range &) {
      auto new_dedup(std::make_unique<Dedup>(&stm));
      new_dedup->create();
      dedup = (stm.dedups_[emsg->getPriceSide()] = std::move(new_dedup)).get();
    }
    vector<std::unique_ptr<Message>> arg;
    arg.push_back(std::move(msg));
    auto output = dedup->handle(std::move(arg));
    std::move(output.begin(), output.end(), std::back_inserter(stm.output_));
  }
  stm.episode_.clear();
}

// perform the 'exit behavior'
void DedupOrchestrator::DedupOrchestrator_State::Deduplicate_State::_doexit(DedupOrchestrator & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute exit behavior of .DedupOrchestrator.Deduplicate" << std::endl;
#endif
  // Dedup *dedup = stm.contaminated_iter_->second;
  // dedup->timeElapsed(exchange_timestamp_);
  // if (!dedup->is_contaminated_)
  //  stm.contaminated_.erase(stm.contaminated_iter_);
}

// returns the state containing the current
DedupOrchestrator::AnyState * DedupOrchestrator::DedupOrchestrator_State::Deduplicate_State::_upper(DedupOrchestrator & stm) {
    return &stm._deduporchestrator_state;
}

DedupOrchestrator::DedupOrchestrator_State::~DedupOrchestrator_State() {
}

// to manage the event create
void DedupOrchestrator::DedupOrchestrator_State::create(DedupOrchestrator & stm) {
    {
      stm._set_currentState(stm._deduporchestrator_state._wait_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .DedupOrchestrator.Wait" << std::endl;
#endif
    }
}

// returns the state containing the current
DedupOrchestrator::AnyState * DedupOrchestrator::DedupOrchestrator_State::_upper(DedupOrchestrator &) {
    return 0;
}

// the operation you call to signal the event message
bool DedupOrchestrator::message() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger message" << std::endl;
#endif
      _current_state->message(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event elapsed
bool DedupOrchestrator::elapsed() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger elapsed" << std::endl;
#endif
      _current_state->elapsed(*this);
    }
    return (_current_state != 0);
}

void DedupOrchestrator::setOutputBoundary(const Timestamp & timestamp) {
}

void DedupOrchestrator::releaseOutputBoundary(const Timestamp & timestamp) {
}

// the operation you call to signal the event stop
bool DedupOrchestrator::stop() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger stop" << std::endl;
#endif
      _current_state->stop(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event create
bool DedupOrchestrator::create() {
  if (_current_state == 0)
    (_current_state = &(*this)._deduporchestrator_state)->create(*this);
  return (_current_state != 0);
}

// to execute the current state 'do activity'
void DedupOrchestrator::doActivity() {
    _current_state->_do(*this);
}

// execution done, internal
void DedupOrchestrator::_final() {
    _current_state = 0;
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : final state reached" << std::endl;
#endif
}

string DedupOrchestrator::getHandlerName() {
  return "DedupOrch";
}

const Volume BitstampReconstructor::BitstampMessage::getBaseMinSize() const {
  return reconstructor_.base_min_size_;
  
}

const Volume BitstampReconstructor::BitstampMessage::getBaseIncrement() const {
  return reconstructor_.base_increment_;
  
}

BitstampReconstructor::BitstampMessage::BitstampMessage(const BitstampReconstructor & reconstructor): reconstructor_{reconstructor} {
}

const boost::uuids::uuid BitstampReconstructor::BitstampMessage::toUuid(string id)
{
  std::stringstream buf;
  buf << std::setw(32) << std::setfill('0') << std::right << std::stol(id);
  return boost::uuids::string_generator{}(buf.str());
  
}

const Timestamp BitstampReconstructor::BitstampMessage::toTimestamp(string timestamp)
{
  using namespace date;
  boost::uuids::string_generator gen;
  std::istringstream ss{timestamp};
  Timestamp output;
  ss >> parse("%FT%TZ", output);
  return output;
}

BitstampReconstructor::BitstampCreated::BitstampCreated(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor): BitstampMessage{reconstructor} {
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

BitstampReconstructor::BitstampReconstructor( Store * store, const Volume & base_min_size, const Volume & base_increment, bool extract_only) {
  store_ = store;
  base_min_size_ = base_min_size;
  base_increment_ = base_increment;
  extract_only_ = extract_only;
  size_deducer_ =std::make_unique<MessageHandler>();
  size_deducer_->create();
  // Deduplication is not needed for CoinBase
  deduplicator_ = std::unique_ptr<MessageHandler>();
  event_number_generator_ = std::make_unique<EventNumberGenerator>();
  
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
    output.push_back(make_unique<BitstampCreated>(tree, *this));
  else if (type == "order_changed")
    output.push_back(make_unique<BitstampChanged>(tree, *this));
  else if (type == "order_deleted") {
      output.push_back(make_unique<BitstampCanceled>(tree, *this));
  } else if (type == "match") {
    output.push_back(make_unique<BitstampFilled>(tree, *this));
  }
  return output;
  
}


} // namespace oberon::core

} // namespace oberon
