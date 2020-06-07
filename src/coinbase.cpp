
#include "coinbase.h"
#include <vector>

#include <boost/uuid/string_generator.hpp>


#include <iostream>
#include "date.h"
namespace oberon {

namespace core {

CoinbaseReconstructor::CoinbaseMessage::CoinbaseMessage(const CoinbaseReconstructor & reconstructor): reconstructor_ {reconstructor} {
}

CoinbaseReconstructor::CoinbaseReceived::CoinbaseReceived(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor): CoinbaseMessage{reconstructor} {
  using namespace date;
  using namespace std;
  boost::uuids::string_generator gen;
  std::istringstream ss{tree.get<string>("time")};
  ss >> parse("%FT%TZ", timestamp_);
  order_id_ = gen(tree.get<string>("order_id"));
  price_ = stod(tree.get<string>("price"));
  remaining_size_ = stod(tree.get<string>("size"));
  side_ = tree.get<string>("side") == "buy" ? kBid : kAsk;
  
}

const Volume CoinbaseReconstructor::CoinbaseReceived::getBaseMinSize() const {
  return reconstructor_.base_min_size_;
}

CoinbaseReconstructor::CoinbaseMatch::CoinbaseMatch(const boost::property_tree::ptree & tree, TradeRole role, const CoinbaseReconstructor & reconstructor) : CoinbaseMessage{reconstructor} {
  using namespace date;
  using namespace std;
  boost::uuids::string_generator gen;
  std::istringstream ss{tree.get<string>("time")};
  ss >> parse("%FT%TZ", timestamp_);
  price_ = stod(tree.get<string>("price"));
  change_size_ = stod(tree.get<string>("size"));
  trade_id_ = stol(tree.get<string>("trade_id"));
  remaining_size_ = 0;
  event_no_ = 1;
  role_ = role;
  if(role == kMaker) {
    side_ = (tree.get<string>("side") == "buy" ? kBid : kAsk);
    order_id_ = gen(tree.get<string>("maker_order_id"));
  }
  else {
    side_ = (tree.get<string>("side") == "buy" ? kAsk : kBid);
    order_id_ = gen(tree.get<string>("taker_order_id"));
  }
}

const Volume CoinbaseReconstructor::CoinbaseMatch::getBaseMinSize() const {
  return reconstructor_.base_min_size_;
}

CoinbaseReconstructor::CoinbaseDone::CoinbaseDone(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor): CoinbaseMessage{reconstructor}{
  using namespace date;
  using namespace std;
  boost::uuids::string_generator gen;
  std::istringstream ss{tree.get<string>("time")};
  ss >> parse("%FT%TZ", timestamp_);
  order_id_ = gen(tree.get<string>("order_id"));
  price_ = stod(tree.get<string>("price"));
  remaining_size_ = stod(tree.get<string>("remaining_size"));
  change_size_ = 0; // we don't know change_size yet
  event_no_ = 0; // we don't know event_no either
  side_ = tree.get<string>("side") == "buy" ? kBid : kAsk;
  
}

const Volume CoinbaseReconstructor::CoinbaseDone::getBaseMinSize() const {
  return 0.001;
}

CoinbaseReconstructor::CoinbaseOpen::CoinbaseOpen(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor): CoinbaseMessage{reconstructor} {
  using namespace date;
  using namespace std;
  boost::uuids::string_generator gen;
  std::istringstream ss{tree.get<string>("time")};
  ss >> parse("%FT%TZ", timestamp_);
  order_id_ = gen(tree.get<string>("order_id"));
  price_ = stod(tree.get<string>("price"));
  remaining_size_ = stod(tree.get<string>("remaining_size"));
  change_size_ = 0;
  event_no_ = 1;
  side_ = tree.get<string>("side") == "buy" ? kBid : kAsk;
}

const Volume CoinbaseReconstructor::CoinbaseOpen::getBaseMinSize() const {
  return 0.001;
}

vector<std::unique_ptr<MessageHandler::Message>> CoinbaseReconstructor::extract(const boost::property_tree::ptree & tree) {
  using namespace date;
  using namespace std;
  string type = tree.get<string>("type");
  Timestamp timestamp;
  std::istringstream ss{tree.get<string>("time")};
  ss >> parse("%FT%TZ", timestamp);
  vector<unique_ptr<Message>> output;
  if (type == "elapsed")
    output.push_back(make_unique<Elapsed>(timestamp));
  else if (type == "received")
    output.push_back(make_unique<CoinbaseReceived>(tree, *this));
  else if (type == "open")
    output.push_back(make_unique<CoinbaseOpen>(tree, *this));
  else if (type == "done" && tree.get<string>("reason") == "canceled")
    output.push_back(make_unique<CoinbaseDone>(tree, *this));
  else if (type == "match") {
    output.push_back(make_unique<CoinbaseMatch>(tree, kMaker, *this));
    output.push_back(make_unique<CoinbaseMatch>(tree, kTaker, *this));
  }
  return output;
}

CoinbaseReconstructor::CoinbaseReconstructor( Store * store, const Volume & base_min_size) {
  store_ = store;
  base_min_size_ = base_min_size;
  deduplicator_ = std::make_unique<Coinbase_Deduplicator>();
  deduplicator_->create();
  size_deducer_ = std::make_unique<Deduce_Size_Coinbase>();
  size_deducer_->create();
}

bool Deduce_Size_Coinbase_Extensions::volumeIncremented() {
  return message();
}

bool Deduce_Size_Coinbase_Extensions::priceAdvanced() {
  return message();
}

bool Deduce_Size_Coinbase_Extensions::priceReceded() {
  return message();
}

bool Deduce_Size_Coinbase_Extensions::partiallyCanceled() {
  return message();
}

bool Deduce_Size_Coinbase_Extensions::fullyCanceled() {
  return message();
}

bool Deduce_Size_Coinbase_Extensions::opened() {
  return message();
}

string Deduce_Size_Coinbase_Extensions::getHandlerName() {
  return "Deduce Size";
}

Deduce_Size_Coinbase::AnyState::~AnyState() {
}

// perform the 'do activity'
void Deduce_Size_Coinbase::AnyState::_do(Deduce_Size_Coinbase &) {
}

// the current state doesn't manage the event received, give it to the upper state
void Deduce_Size_Coinbase::AnyState::received(Deduce_Size_Coinbase & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->received(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition received not expected" << std::endl;
#endif
}

// the current state doesn't manage the event elapsed, give it to the upper state
void Deduce_Size_Coinbase::AnyState::elapsed(Deduce_Size_Coinbase & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->elapsed(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition elapsed not expected" << std::endl;
#endif
}

// the current state doesn't manage the event era, give it to the upper state
void Deduce_Size_Coinbase::AnyState::era(Deduce_Size_Coinbase & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->era(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition era not expected" << std::endl;
#endif
}

// the current state doesn't manage the event filled, give it to the upper state
void Deduce_Size_Coinbase::AnyState::filled(Deduce_Size_Coinbase & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->filled(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition filled not expected" << std::endl;
#endif
}

// the current state doesn't manage the event message, give it to the upper state
void Deduce_Size_Coinbase::AnyState::message(Deduce_Size_Coinbase & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->message(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition message not expected" << std::endl;
#endif
}

void Deduce_Size_Coinbase::AnyState::create(Deduce_Size_Coinbase &) {
}

Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::~Wait_State() {
}

// to manage the event received
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::received(Deduce_Size_Coinbase & stm) {
    {
      stm._deduce_size_coinbase_state._wait_state._doexit(stm);
      stm._set_currentState(stm._deduce_size_coinbase_state._save_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Save" << std::endl;
#endif
      stm._deduce_size_coinbase_state._save_state.create(stm);
    }
}

// to manage the event elapsed
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::elapsed(Deduce_Size_Coinbase & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition elapsed" << std::endl;
#endif
  stm.output_.push_back(std::move(stm.received_));
    }
}

// to manage the event era
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::era(Deduce_Size_Coinbase & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition era" << std::endl;
#endif
  // TBD reset order sizes 
  stm.output_.push_back(std::move(stm.received_));  }
}

// to manage the event filled
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::filled(Deduce_Size_Coinbase & stm) {
    {
      stm._deduce_size_coinbase_state._wait_state._doexit(stm);
      stm._deduce_size_coinbase_state._deduce_state._doentry(stm);
      stm._set_currentState(stm._deduce_size_coinbase_state._deduce_state._remaining_size_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Deduce.Remaining size" << std::endl;
#endif
      stm._deduce_size_coinbase_state._deduce_state._remaining_size_state.create(stm);
    }
}

// to manage the event message
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::message(Deduce_Size_Coinbase & stm) {
    {
      stm._deduce_size_coinbase_state._wait_state._doexit(stm);
      stm._deduce_size_coinbase_state._deduce_state._doentry(stm);
      stm._set_currentState(stm._deduce_size_coinbase_state._deduce_state._size_change_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Deduce.Size change" << std::endl;
#endif
      stm._deduce_size_coinbase_state._deduce_state._size_change_state.create(stm);
    }
}

// perform the 'exit behavior'
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::_doexit(Deduce_Size_Coinbase & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute exit behavior of .Deduce Size Coinbase.Wait" << std::endl;
#endif
  stm.em_ = std::unique_ptr<ExchangeMessage>(
      dynamic_cast<ExchangeMessage *>(stm.received_.release()));
}

// returns the state containing the current
Deduce_Size_Coinbase::AnyState * Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::_upper(Deduce_Size_Coinbase & stm) {
    return &stm._deduce_size_coinbase_state;
}

Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Save_State::~Save_State() {
}

bool Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Save_State::_completion(Deduce_Size_Coinbase & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition output" << std::endl;
#endif
  stm.output_.push_back(std::move(stm.em_));
      stm._set_currentState(stm._deduce_size_coinbase_state._wait_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Wait" << std::endl;
#endif
      return (bool) 1;
    }
}

// to manage the event create
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Save_State::create(Deduce_Size_Coinbase & stm) {
  	_doentry(stm);
  	_completion(stm);
}

// perform the 'entry behavior'
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Save_State::_doentry(Deduce_Size_Coinbase & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute entry behavior of .Deduce Size Coinbase.Save" << std::endl;
#endif
  if(stm.em_->getRemainingSize())
    stm.previousSizes_[stm.em_->getOrderId()] = stm.em_->getRemainingSize();
  else
    stm.previousSizes_.erase(stm.em_->getOrderId());
}

// returns the state containing the current
Deduce_Size_Coinbase::AnyState * Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Save_State::_upper(Deduce_Size_Coinbase & stm) {
    return &stm._deduce_size_coinbase_state;
}

Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Size_change_State::~Size_change_State() {
}

bool Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Size_change_State::_completion(Deduce_Size_Coinbase & stm) {
    if (stm.previousSize_) {
      stm._deduce_size_coinbase_state._deduce_state._doexit(stm);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition success" << std::endl;
#endif
  // set size change
      stm._set_currentState(stm._deduce_size_coinbase_state._save_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Save" << std::endl;
#endif
      stm._deduce_size_coinbase_state._save_state.create(stm);
      return (bool) 1;
    }
    else if (!stm.previousSize_) {
      stm._set_currentState(stm._deduce_size_coinbase_state._deduce_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Deduce" << std::endl;
#endif
      stm._deduce_size_coinbase_state._deduce_state._exit1(stm);
      return (bool) 1;
    }
    return (bool) 0;
}

// to manage the event create
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Size_change_State::create(Deduce_Size_Coinbase & stm) {
  	_doentry(stm);
  	_completion(stm);
}

// perform the 'entry behavior'
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Size_change_State::_doentry(Deduce_Size_Coinbase & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute entry behavior of .Deduce Size Coinbase.Deduce.Size change" << std::endl;
#endif
  if(stm.previousSize_)
    stm.em_->setChangeSize(stm.previousSize_ - stm.em_->getRemainingSize());
}

// returns the state containing the current
Deduce_Size_Coinbase::AnyState * Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Size_change_State::_upper(Deduce_Size_Coinbase & stm) {
    return &stm._deduce_size_coinbase_state._deduce_state;
}

Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Remaining_size_State::~Remaining_size_State() {
}

bool Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Remaining_size_State::_completion(Deduce_Size_Coinbase & stm) {
    if (stm.previousSize_) {
      stm._deduce_size_coinbase_state._deduce_state._doexit(stm);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition success" << std::endl;
#endif
  // set remaining size
      stm._set_currentState(stm._deduce_size_coinbase_state._save_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Save" << std::endl;
#endif
      stm._deduce_size_coinbase_state._save_state.create(stm);
      return (bool) 1;
    }
    else if (!stm.previousSize_) {
      stm._set_currentState(stm._deduce_size_coinbase_state._deduce_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Deduce" << std::endl;
#endif
      stm._deduce_size_coinbase_state._deduce_state._exit1(stm);
      return (bool) 1;
    }
    return (bool) 0;
}

// to manage the event create
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Remaining_size_State::create(Deduce_Size_Coinbase & stm) {
  	_doentry(stm);
  	_completion(stm);
}

// perform the 'entry behavior'
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Remaining_size_State::_doentry(Deduce_Size_Coinbase & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute entry behavior of .Deduce Size Coinbase.Deduce.Remaining size" << std::endl;
#endif
  if(stm.previousSize_) 
    stm.em_->setRemainingSize(stm.previousSize_ - stm.em_->getChangeSize());
}

// returns the state containing the current
Deduce_Size_Coinbase::AnyState * Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Remaining_size_State::_upper(Deduce_Size_Coinbase & stm) {
    return &stm._deduce_size_coinbase_state._deduce_state;
}

Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::~Deduce_State() {
}

// to manage the exit point 'exit', defined because probably shared
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::_exit1(Deduce_Size_Coinbase & stm) {
    _doexit(stm);
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition drop" << std::endl;
#endif
  stm.em_.reset(nullptr);    stm._set_currentState(stm._deduce_size_coinbase_state._wait_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Wait" << std::endl;
#endif
    }
}

// perform the 'entry behavior'
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::_doentry(Deduce_Size_Coinbase & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute entry behavior of .Deduce Size Coinbase.Deduce" << std::endl;
#endif
  stm.previousSize_ = 0;
  
  try {
    stm.previousSize_ = stm.previousSizes_.at(stm.em_->getOrderId());
  }
  catch (...) {};
}

// perform the 'exit behavior'
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::_doexit(Deduce_Size_Coinbase & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute exit behavior of .Deduce Size Coinbase.Deduce" << std::endl;
#endif
  
}

// returns the state containing the current
Deduce_Size_Coinbase::AnyState * Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::_upper(Deduce_Size_Coinbase & stm) {
    return &stm._deduce_size_coinbase_state;
}

Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::~Deduce_Size_Coinbase_State() {
}

// to manage the event create
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::create(Deduce_Size_Coinbase & stm) {
    {
      stm._set_currentState(stm._deduce_size_coinbase_state._wait_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Wait" << std::endl;
#endif
    }
}

// returns the state containing the current
Deduce_Size_Coinbase::AnyState * Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::_upper(Deduce_Size_Coinbase &) {
    return 0;
}

Deduce_Size_Coinbase::Deduce_Size_Coinbase() {
    _current_state = 0;
}

Deduce_Size_Coinbase::~Deduce_Size_Coinbase() {
}

// the operation you call to signal the event received
bool Deduce_Size_Coinbase::received() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger received" << std::endl;
#endif
      _current_state->received(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event elapsed
bool Deduce_Size_Coinbase::elapsed() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger elapsed" << std::endl;
#endif
      _current_state->elapsed(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event era
bool Deduce_Size_Coinbase::era() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger era" << std::endl;
#endif
      _current_state->era(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event filled
bool Deduce_Size_Coinbase::filled() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger filled" << std::endl;
#endif
      _current_state->filled(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event message
bool Deduce_Size_Coinbase::message() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger message" << std::endl;
#endif
      _current_state->message(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event create
bool Deduce_Size_Coinbase::create() {
  if (_current_state == 0)
    (_current_state = &(*this)._deduce_size_coinbase_state)->create(*this);
  return (_current_state != 0);
}

// to execute the current state 'do activity'
void Deduce_Size_Coinbase::doActivity() {
    _current_state->_do(*this);
}

// execution done, internal
void Deduce_Size_Coinbase::_final() {
    _current_state = 0;
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : final state reached" << std::endl;
#endif
}

Coinbase_Deduplicator::AnyState::~AnyState() {
}

// perform the 'do activity'
void Coinbase_Deduplicator::AnyState::_do(Coinbase_Deduplicator &) {
}

void Coinbase_Deduplicator::AnyState::create(Coinbase_Deduplicator &) {
}

// the current state doesn't manage the event received, give it to the upper state
void Coinbase_Deduplicator::AnyState::received(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->received(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition received not expected" << std::endl;
#endif
}

// the current state doesn't manage the event elapsed, give it to the upper state
void Coinbase_Deduplicator::AnyState::elapsed(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->elapsed(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition elapsed not expected" << std::endl;
#endif
}

// the current state doesn't manage the event opened, give it to the upper state
void Coinbase_Deduplicator::AnyState::opened(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->opened(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition opened not expected" << std::endl;
#endif
}

// the current state doesn't manage the event filled, give it to the upper state
void Coinbase_Deduplicator::AnyState::filled(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->filled(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition filled not expected" << std::endl;
#endif
}

// the current state doesn't manage the event fullyCanceled, give it to the upper state
void Coinbase_Deduplicator::AnyState::fullyCanceled(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->fullyCanceled(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition fullyCanceled not expected" << std::endl;
#endif
}

// the current state doesn't manage the event era, give it to the upper state
void Coinbase_Deduplicator::AnyState::era(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->era(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition era not expected" << std::endl;
#endif
}

// the current state doesn't manage the event stop, give it to the upper state
void Coinbase_Deduplicator::AnyState::stop(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->stop(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition stop not expected" << std::endl;
#endif
}

// the current state doesn't manage the event priceAdvanced, give it to the upper state
void Coinbase_Deduplicator::AnyState::priceAdvanced(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->priceAdvanced(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition priceAdvanced not expected" << std::endl;
#endif
}

// the current state doesn't manage the event priceReceded, give it to the upper state
void Coinbase_Deduplicator::AnyState::priceReceded(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->priceReceded(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition priceReceded not expected" << std::endl;
#endif
}

// the current state doesn't manage the event partiallyCanceled, give it to the upper state
void Coinbase_Deduplicator::AnyState::partiallyCanceled(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->partiallyCanceled(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition partiallyCanceled not expected" << std::endl;
#endif
}

// the current state doesn't manage the event volumeIncremented, give it to the upper state
void Coinbase_Deduplicator::AnyState::volumeIncremented(Coinbase_Deduplicator & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->volumeIncremented(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition volumeIncremented not expected" << std::endl;
#endif
}

Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::~Wait_State() {
}

// to manage the event received
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::received(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._output_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Output" << std::endl;
#endif
      stm._coinbase_deduplicator_state._output_state.create(stm);
    }
}

// to manage the event elapsed
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::elapsed(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._output_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Output" << std::endl;
#endif
      stm._coinbase_deduplicator_state._output_state.create(stm);
    }
}

// to manage the event opened
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::opened(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._output_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Output" << std::endl;
#endif
      stm._coinbase_deduplicator_state._output_state.create(stm);
    }
}

// to manage the event filled
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::filled(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._output_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Output" << std::endl;
#endif
      stm._coinbase_deduplicator_state._output_state.create(stm);
    }
}

// to manage the event fullyCanceled
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::fullyCanceled(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._output_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Output" << std::endl;
#endif
      stm._coinbase_deduplicator_state._output_state.create(stm);
    }
}

// to manage the event era
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::era(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._output_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Output" << std::endl;
#endif
      stm._coinbase_deduplicator_state._output_state.create(stm);
    }
}

// to manage the event stop
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::stop(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator" << std::endl;
#endif
      stm._final();
    }
}

// returns the state containing the current
Coinbase_Deduplicator::AnyState * Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::_upper(Coinbase_Deduplicator & stm) {
    return &stm._coinbase_deduplicator_state;
}

// to manage the event priceAdvanced
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::priceAdvanced(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._fail_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Fail" << std::endl;
#endif
    }
}

// to manage the event priceReceded
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::priceReceded(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._fail_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Fail" << std::endl;
#endif
    }
}

// to manage the event partiallyCanceled
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::partiallyCanceled(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._fail_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Fail" << std::endl;
#endif
    }
}

// to manage the event volumeIncremented
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Wait_State::volumeIncremented(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._fail_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Fail" << std::endl;
#endif
    }
}

Coinbase_Deduplicator::Coinbase_Deduplicator_State::Fail_State::~Fail_State() {
}

// to manage the event create
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Fail_State::create(Coinbase_Deduplicator & stm) {
  	_do(stm);
}

// perform the 'do activity'
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Fail_State::_do(Coinbase_Deduplicator & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute do behavior of .Coinbase Deduplicator.Fail" << std::endl;
#endif
  assert(false);
}

// returns the state containing the current
Coinbase_Deduplicator::AnyState * Coinbase_Deduplicator::Coinbase_Deduplicator_State::Fail_State::_upper(Coinbase_Deduplicator & stm) {
    return &stm._coinbase_deduplicator_state;
}

Coinbase_Deduplicator::Coinbase_Deduplicator_State::Output_State::~Output_State() {
}

bool Coinbase_Deduplicator::Coinbase_Deduplicator_State::Output_State::_completion(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._wait_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Wait" << std::endl;
#endif
      return (bool) 1;
    }
}

// to manage the event create
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Output_State::create(Coinbase_Deduplicator & stm) {
  	_do(stm);
  	_completion(stm);
}

// perform the 'do activity'
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::Output_State::_do(Coinbase_Deduplicator & stm) {
#ifdef VERBOSE_STATE_MACHINE
    std::cout << "DEBUG : execute do behavior of .Coinbase Deduplicator.Output" << std::endl;
#endif
  stm.output_.push_back(std::move(stm.received_));
}

// returns the state containing the current
Coinbase_Deduplicator::AnyState * Coinbase_Deduplicator::Coinbase_Deduplicator_State::Output_State::_upper(Coinbase_Deduplicator & stm) {
    return &stm._coinbase_deduplicator_state;
}

Coinbase_Deduplicator::Coinbase_Deduplicator_State::~Coinbase_Deduplicator_State() {
}

// to manage the event create
void Coinbase_Deduplicator::Coinbase_Deduplicator_State::create(Coinbase_Deduplicator & stm) {
    {
      stm._set_currentState(stm._coinbase_deduplicator_state._wait_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Coinbase Deduplicator.Wait" << std::endl;
#endif
    }
}

// returns the state containing the current
Coinbase_Deduplicator::AnyState * Coinbase_Deduplicator::Coinbase_Deduplicator_State::_upper(Coinbase_Deduplicator &) {
    return 0;
}

Coinbase_Deduplicator::Coinbase_Deduplicator() {
    _current_state = 0;
}

Coinbase_Deduplicator::~Coinbase_Deduplicator() {
}

// the operation you call to signal the event create
bool Coinbase_Deduplicator::create() {
  if (_current_state == 0)
    (_current_state = &(*this)._coinbase_deduplicator_state)->create(*this);
  return (_current_state != 0);
}

// the operation you call to signal the event received
bool Coinbase_Deduplicator::received() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger received" << std::endl;
#endif
      _current_state->received(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event elapsed
bool Coinbase_Deduplicator::elapsed() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger elapsed" << std::endl;
#endif
      _current_state->elapsed(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event opened
bool Coinbase_Deduplicator::opened() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger opened" << std::endl;
#endif
      _current_state->opened(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event filled
bool Coinbase_Deduplicator::filled() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger filled" << std::endl;
#endif
      _current_state->filled(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event fullyCanceled
bool Coinbase_Deduplicator::fullyCanceled() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger fullyCanceled" << std::endl;
#endif
      _current_state->fullyCanceled(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event era
bool Coinbase_Deduplicator::era() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger era" << std::endl;
#endif
      _current_state->era(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event stop
bool Coinbase_Deduplicator::stop() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger stop" << std::endl;
#endif
      _current_state->stop(*this);
    }
    return (_current_state != 0);
}

// to execute the current state 'do activity'
void Coinbase_Deduplicator::doActivity() {
    _current_state->_do(*this);
}

// execution done, internal
void Coinbase_Deduplicator::_final() {
    _current_state = 0;
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : final state reached" << std::endl;
#endif
}

// the operation you call to signal the event priceAdvanced
bool Coinbase_Deduplicator::priceAdvanced() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger priceAdvanced" << std::endl;
#endif
      _current_state->priceAdvanced(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event priceReceded
bool Coinbase_Deduplicator::priceReceded() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger priceReceded" << std::endl;
#endif
      _current_state->priceReceded(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event partiallyCanceled
bool Coinbase_Deduplicator::partiallyCanceled() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger partiallyCanceled" << std::endl;
#endif
      _current_state->partiallyCanceled(*this);
    }
    return (_current_state != 0);
}

// the operation you call to signal the event volumeIncremented
bool Coinbase_Deduplicator::volumeIncremented() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger volumeIncremented" << std::endl;
#endif
      _current_state->volumeIncremented(*this);
    }
    return (_current_state != 0);
}

string Coinbase_Deduplicator::getHandlerName() {
  return "CoinbaseDeduplicator";
}


} // namespace oberon::core

} // namespace oberon
