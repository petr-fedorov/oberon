
#include "coinbase.h"
#include <vector>

#include <boost/uuid/string_generator.hpp>

#include <cmath>


#include <iostream>
#include "date.h"
namespace oberon {

namespace core {

CoinbaseReconstructor::CoinbaseMessage::CoinbaseMessage(const CoinbaseReconstructor & reconstructor): reconstructor_ {reconstructor} {
}

const Volume CoinbaseReconstructor::CoinbaseMessage::getBaseMinSize() const {
  return reconstructor_.base_min_size_;
  
}

const Volume CoinbaseReconstructor::CoinbaseMessage::getBaseIncrement() const {
  return reconstructor_.base_increment_;
  
}

CoinbaseReconstructor::CoinbaseReceived::CoinbaseReceived(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor): CoinbaseMessage{reconstructor} {
  using namespace date;
  using namespace std;
  boost::uuids::string_generator gen;
  std::istringstream ss{tree.get<string>("time")};
  ss >> parse("%FT%TZ", timestamp_);
  ss.str(tree.get<string>("local_timestamp"));
  ss >> parse("%FT%TZ", local_timestamp_);
  order_id_ = gen(tree.get<string>("order_id"));
  price_ = stod(tree.get<string>("price"));
  remaining_size_ = stod(tree.get<string>("size"));
  change_size_ = 0;
  event_no_ = 0;
  side_ = tree.get<string>("side") == "buy" ? kBid : kAsk;
  
}

CoinbaseReconstructor::CoinbaseMatch::CoinbaseMatch(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor) : CoinbaseMessage{reconstructor} {
  using namespace date;
  using namespace std;
  boost::uuids::string_generator gen;
  std::istringstream ss{tree.get<string>("time")};
  ss >> parse("%FT%TZ", timestamp_);
  ss.str(tree.get<string>("local_timestamp"));
  ss >> parse("%FT%TZ", local_timestamp_);
  price_ = stod(tree.get<string>("price"));
  change_size_ = stod(tree.get<string>("size"));
  trade_id_ = stol(tree.get<string>("trade_id"));
  side_ = (tree.get<string>("side") == "buy" ? kBid : kAsk);
  order_id_ = gen(tree.get<string>("maker_order_id"));
  taker_order_id_ = gen(tree.get<string>("taker_order_id"));
}

CoinbaseReconstructor::CoinbaseDoneCanceled::CoinbaseDoneCanceled(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor): CoinbaseMessage{reconstructor}{
  using namespace date;
  using namespace std;
  boost::uuids::string_generator gen;
  std::istringstream ss{tree.get<string>("time")};
  ss >> parse("%FT%TZ", timestamp_);
  ss.str(tree.get<string>("local_timestamp"));
  ss >> parse("%FT%TZ", local_timestamp_);
  order_id_ = gen(tree.get<string>("order_id"));
  price_ = stod(tree.get<string>("price"));
  remaining_size_ = stod(tree.get<string>("remaining_size"));
  change_size_ = 0;
  side_ = tree.get<string>("side") == "buy" ? kBid : kAsk;
  
}

CoinbaseReconstructor::CoinbaseOpen::CoinbaseOpen(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor): CoinbaseMessage{reconstructor} {
  using namespace date;
  using namespace std;
  boost::uuids::string_generator gen;
  std::istringstream ss{tree.get<string>("time")};
  ss >> parse("%FT%TZ", timestamp_);
  ss.str(tree.get<string>("local_timestamp"));
  ss >> parse("%FT%TZ", local_timestamp_);
  order_id_ = gen(tree.get<string>("order_id"));
  price_ = stod(tree.get<string>("price"));
  remaining_size_ = stod(tree.get<string>("remaining_size"));
  change_size_ =
      0; // Open message signifies the first appearance of the order in LOB
  event_no_ = 1;
  side_ = tree.get<string>("side") == "buy" ? kBid : kAsk;
}

vector<std::unique_ptr<MessageHandler::Message>> CoinbaseReconstructor::extract(const boost::property_tree::ptree & tree) {
  using namespace date;
  using namespace std;
  Timestamp timestamp = toTimestamp(tree.get<string>("time"));
  vector<unique_ptr<Message>> output;
  string type = tree.get<string>("type");
  if (type == "elapsed")
    output.push_back(make_unique<Elapsed>(timestamp));
  else if (type == "received")
    output.push_back(make_unique<CoinbaseReceived>(tree, *this));
  else if (type == "open")
    output.push_back(make_unique<CoinbaseOpen>(tree, *this));
  else if (type == "done") {
    if (tree.get<string>("reason") == "canceled")
      output.push_back(make_unique<CoinbaseDoneCanceled>(tree, *this));
  } else if (type == "match") {
    output.push_back(make_unique<CoinbaseMatch>(tree, *this));
  }
  return size_deducer_->handle(move(output));
}

CoinbaseReconstructor::CoinbaseReconstructor( Store * store, const Volume & base_min_size, const Volume & base_increment, bool extract_only) {
  store_ = store;
  base_min_size_ = base_min_size;
  base_increment_ = base_increment;
  extract_only_ = extract_only;
  size_deducer_ = std::make_unique<Deduce_Size_Coinbase>();
  size_deducer_->create();
  // Deduplication is not needed for CoinBase
  deduplicator_ = std::unique_ptr<MessageHandler>();
  event_number_generator_ = std::make_unique<EventNumberGenerator>();
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

// the current state doesn't manage the event opened, give it to the upper state
void Deduce_Size_Coinbase::AnyState::opened(Deduce_Size_Coinbase & stm) {
    AnyState * st = _upper(stm);
  
    if (st != 0)
      st->opened(stm);
#ifdef VERBOSE_STATE_MACHINE
    else
      std::cout << "DEBUG : transition opened not expected" << std::endl;
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

void Deduce_Size_Coinbase::AnyState::create(Deduce_Size_Coinbase &) {
}

Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::~Wait_State() {
}

// to manage the event opened
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::opened(Deduce_Size_Coinbase & stm) {
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

// to manage the event received
void Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Wait_State::received(Deduce_Size_Coinbase & stm) {
    {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition received" << std::endl;
#endif
  stm.received_.reset(nullptr);  }
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
      std::cout << "DEBUG : execute activity of transition output saved" << std::endl;
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
    stm.previous_sizes_[stm.em_->getOrderId()] = stm.em_->getRemainingSize();
  else
    stm.previous_sizes_.erase(stm.em_->getOrderId());
}

// returns the state containing the current
Deduce_Size_Coinbase::AnyState * Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Save_State::_upper(Deduce_Size_Coinbase & stm) {
    return &stm._deduce_size_coinbase_state;
}

Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Size_change_State::~Size_change_State() {
}

bool Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Size_change_State::_completion(Deduce_Size_Coinbase & stm) {
    if (!std::isnan(stm.previous_size_)) {
      stm._deduce_size_coinbase_state._deduce_state._doexit(stm);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition success" << std::endl;
#endif
  stm.em_->setChangeSize(stm.previous_size_ - stm.em_->getRemainingSize());
  
      stm._set_currentState(stm._deduce_size_coinbase_state._save_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Save" << std::endl;
#endif
      stm._deduce_size_coinbase_state._save_state.create(stm);
      return (bool) 1;
    }
    else if (std::isnan(stm.previous_size_)) {
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
  	_completion(stm);
}

// returns the state containing the current
Deduce_Size_Coinbase::AnyState * Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Size_change_State::_upper(Deduce_Size_Coinbase & stm) {
    return &stm._deduce_size_coinbase_state._deduce_state;
}

Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Remaining_size_State::~Remaining_size_State() {
}

bool Deduce_Size_Coinbase::Deduce_Size_Coinbase_State::Deduce_State::Remaining_size_State::_completion(Deduce_Size_Coinbase & stm) {
    if (!std::isnan(stm.previous_size_)) {
      stm._deduce_size_coinbase_state._deduce_state._doexit(stm);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : execute activity of transition success" << std::endl;
#endif
  stm.em_->setRemainingSize(stm.previous_size_ - stm.em_->getChangeSize());
      stm._set_currentState(stm._deduce_size_coinbase_state._save_state);
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : current state is now .Deduce Size Coinbase.Save" << std::endl;
#endif
      stm._deduce_size_coinbase_state._save_state.create(stm);
      return (bool) 1;
    }
    else if (std::isnan(stm.previous_size_)) {
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
      std::cout << "DEBUG : execute activity of transition output message" << std::endl;
#endif
  stm.output_.push_back(std::move(stm.em_));    stm._set_currentState(stm._deduce_size_coinbase_state._wait_state);
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
  stm.previous_size_ = Event::kNaVolume;
  
  try {
    stm.previous_size_ = stm.previous_sizes_.at(stm.em_->getOrderId());
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

// the operation you call to signal the event opened
bool Deduce_Size_Coinbase::opened() {
    if (_current_state != 0) {
#ifdef VERBOSE_STATE_MACHINE
      std::cout << "DEBUG : fire trigger opened" << std::endl;
#endif
      _current_state->opened(*this);
    }
    return (_current_state != 0);
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


} // namespace oberon::core

} // namespace oberon
