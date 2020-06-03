#ifndef OBERON_CORE_RECONSTRUCTOR_IMPL_H
#define OBERON_CORE_RECONSTRUCTOR_IMPL_H


#include "reconstructor.h"
#include <vector>

#include <map>

#include <boost/property_tree/ptree.hpp>


namespace oberon { namespace core { class Store; }  } 

namespace oberon {

namespace core {

class MessageHandler;
class DedupOrchestrator;
enum BookSide {
  kBid,
  kAsk
};
enum OrderState {
  kActive,
  kOpen,
  kFinished
};
struct PriceSide {
    BookSide side_;

    Price price_;

    bool operator <(const PriceSide & rhs) const;

};
enum TradeRole {
  kMaker,
  kTaker
};
class MessageHandler {
  public:
    class Message {
      protected:
        Timestamp timestamp_;


      public:
        inline const Timestamp getTimestamp() const;

        //By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
        virtual std::unique_ptr<Event> toEvent();

        virtual bool accept(MessageHandler* mh) = 0;

        virtual ~Message();

    };
    
    class ExchangeMessage : public Message {
      protected:
        PriceSide price_side_;


      public:
        PriceSide getPriceSide();


      protected:
        Id order_id_;


      public:
        inline const Id getOrderId() const;


      protected:
        Volume remaining_size_;


      public:
        inline const Volume getRemainingSize() const;

    };
    
    //match or trade with or without order_ids
    class Filled : public ExchangeMessage {
      private:
        TradeRole role_;


      public:
        virtual bool accept(MessageHandler* mh);

    };
    
    //opened on Coinbase. Bitstamp, Bitfinex do not send
    class Opened : public ExchangeMessage {
      public:
        virtual bool accept(MessageHandler* mh);

    };
    
    //order change on Bitstamp with increased volume and the same price
    class VolumeIncremented : public ExchangeMessage {
    };
    
    //order_changed on Bitstamp
    class PartiallyCancelled : public ExchangeMessage {
    };
    
    //order_change on Bitstamp or Bitfinex with increased price for bid or decreased price for ask.  Coinbase or MOEX does not generate such messages
    class PriceAdvanced : public ExchangeMessage {
    };
    
    //order_change on Bitstamp or Bitfinex with decremented price for bid or incremented price for ask.  Coinbase or MOEX does not generate such messages
    class PriceReceded : public ExchangeMessage {
    };
    
    //order_deleted on Bitstamp, done on Coinbase, price =0 on Bitfinex
    class FullyCancelled : public ExchangeMessage {
      public:
        virtual bool accept(MessageHandler * mh) override final;

    };
    
    //order_created on Bitstamp, Bitfinex received on Coinbase
    class Received : public ExchangeMessage {
      public:
        virtual bool accept(MessageHandler * mh) override final;

    };
    
    class Elapsed : public Message {
      public:
        bool accept( MessageHandler * mh) override final;

        Elapsed(const Timestamp & exchange_time);

        ~Elapsed();

    };
    

  protected:
    class Era : public Message {
      public:
        virtual bool accept(MessageHandler* mh);

        Era();

        virtual ~Era();

    };
    
    virtual bool elapsed() = 0;

    virtual bool received() = 0;

    virtual bool opened() = 0;

    virtual bool volumeIncremented() = 0;

    virtual bool priceAdvanced() = 0;

    virtual bool priceReceded() = 0;

    virtual bool filled() = 0;

    virtual bool partiallyCancelled() = 0;

    virtual bool fullyCancelled() = 0;

    std::unique_ptr<Message> received_;


  public:
vector<std::unique_ptr<Message>> handle(vector<std::unique_ptr<Message>> && messages);


  protected:
    vector<std::unique_ptr<Message>> output_;


  public:
    vector<std::unique_ptr<Message>> handle(std::unique_ptr<Message> && message);

};
inline const Timestamp MessageHandler::Message::getTimestamp() const {
  return timestamp_;
}

inline const Id MessageHandler::ExchangeMessage::getOrderId() const {
  return order_id_;
}

inline const Volume MessageHandler::ExchangeMessage::getRemainingSize() const {
  return remaining_size_;
}

// implement the state machine Dedup
class Dedup : public MessageHandler {
  protected:
    // Mother class of all the classes representing states
    class AnyState {
      public:
        virtual ~AnyState();

        // return the upper state
        virtual AnyState * _upper(Dedup &) = 0;

        // perform the 'do activity'
        virtual void _do(Dedup &);

        // the current state doesn't manage the event filled, give it to the upper state
        virtual void filled(Dedup & stm);

        // the current state doesn't manage the event cancelled, give it to the upper state
        virtual void cancelled(Dedup & stm);

        // the current state doesn't manage the event stop, give it to the upper state
        virtual void stop(Dedup & stm);

        // the current state doesn't manage the event message, give it to the upper state
        virtual void message(Dedup & stm);

        virtual void create(Dedup &);

        // the current state doesn't manage the event elapsed, give it to the upper state
        virtual void elapsed(Dedup & stm);

    };
    
    // implement the state Dedup
    class Dedup_State : public AnyState {
      public:
        // implement the state Pristine
        class Pristine_State : public AnyState {
          public:
            virtual ~Pristine_State();

            // to manage the event filled
            virtual void filled(Dedup & stm);

            // to manage the event cancelled
            virtual void cancelled(Dedup & stm);

            // to manage the event stop
            virtual void stop(Dedup & stm);

            // to manage the event message
            virtual void message(Dedup & stm);

            // to manage the event create
            virtual void create(Dedup & stm);

            // perform the 'entry behavior'
            void _doentry(Dedup & stm);

            // returns the state containing the current
            virtual AnyState * _upper(Dedup & stm);

        };
        
        // implement the state Contaminated
        class Contaminated_State : public AnyState {
          public:
            // implement the state Fills and Cancellations
            class Fills_and_Cancellations_State : public AnyState {
              public:
                virtual ~Fills_and_Cancellations_State();

                // to manage the event elapsed
                virtual void elapsed(Dedup & stm);

                // to manage the event filled
                virtual void filled(Dedup & stm);

                // to manage the event cancelled
                virtual void cancelled(Dedup & stm);

                // returns the state containing the current
                virtual AnyState * _upper(Dedup & stm);

            };
            
            // implement the state Fills
            class Fills_State : public AnyState {
              public:
                virtual ~Fills_State();

                // to manage the event cancelled
                virtual void cancelled(Dedup & stm);

                // to manage the event filled
                virtual void filled(Dedup & stm);

                // returns the state containing the current
                virtual AnyState * _upper(Dedup & stm);

            };
            
            // implement the state Cancellations
            class Cancellations_State : public AnyState {
              public:
                virtual ~Cancellations_State();

                // to manage the event filled
                virtual void filled(Dedup & stm);

                // to manage the event cancelled
                virtual void cancelled(Dedup & stm);

                // returns the state containing the current
                virtual AnyState * _upper(Dedup & stm);

            };
            
            virtual ~Contaminated_State();

            // memorize the instance of the state Fills and Cancellations, internal
            Fills_and_Cancellations_State _fills_and_cancellations_state;

            // memorize the instance of the state Cancellations, internal
            Cancellations_State _cancellations_state;

            // memorize the instance of the state Fills, internal
            Fills_State _fills_state;

            // to manage the event elapsed
            virtual void elapsed(Dedup & stm);

            // perform the 'entry behavior'
            void _doentry(Dedup & stm);

            // perform the 'exit behavior'
            void _doexit(Dedup & stm);

            // returns the state containing the current
            virtual AnyState * _upper(Dedup & stm);

            // to manage the event message
            virtual void message(Dedup & stm);

        };
        
        virtual ~Dedup_State();

        // memorize the instance of the state Pristine, internal
        Pristine_State _pristine_state;

        // memorize the instance of the state Contaminated, internal
        Contaminated_State _contaminated_state;

        // to manage the event create
        virtual void create(Dedup & stm);

        // returns the state containing the current
        virtual AnyState * _upper(Dedup &);

    };
    
    Dedup();


  public:
    Dedup(DedupOrchestrator * orchestrator);

    virtual ~Dedup();


  protected:
    DedupOrchestrator * orchestrator_;

    // the operation you call to signal the event elapsed
    bool elapsed();

    virtual bool received();

    virtual bool opened();

    virtual bool volumeIncremented();

    virtual bool priceAdvanced();

    virtual bool priceReceded();

    // the operation you call to signal the event filled
    virtual bool filled();

    virtual bool partiallyCancelled();

    virtual bool fullyCancelled();


  private:
    void deferExchangeMessage(bool update_contaminated_period);


  protected:
    void cleanseMessages();

    void passDeferred();

    // memorize the instance of the state Dedup, internal
    Dedup_State _dedup_state;

    // the operation you call to signal the event cancelled
    bool cancelled();


  public:
    // the operation you call to signal the event stop
    bool stop();


  protected:
    // the operation you call to signal the event message
    bool message();


  public:
    // the operation you call to signal the event create
    bool create();

  friend class Dedup_State::Pristine_State;
  friend class Dedup_State::Contaminated_State::Fills_and_Cancellations_State;
  friend class Dedup_State::Contaminated_State::Cancellations_State;
  friend class Dedup_State::Contaminated_State::Fills_State;
  friend class Dedup_State::Contaminated_State;
  friend class Dedup_State;
    // to execute the current state 'do activity'
    void doActivity();


  protected:
    // change the current state, internal
    inline void _set_currentState(AnyState & st);

    // execution done, internal
    void _final();

    // contains the current state, internal
    AnyState * _current_state;


  private:
    PriceSide key_;

    Duration wait_duration_;

    Timestamp contamination_period_[2];

    vector<std::unique_ptr<MessageHandler::Message>> deferred_;

};
// change the current state, internal
inline void Dedup::_set_currentState(Dedup::AnyState & st) {
    _current_state = &st;
}

// implement the state machine DedupOrchestrator
class DedupOrchestrator : public MessageHandler {
  protected:
    // Mother class of all the classes representing states
    class AnyState {
      public:
        virtual ~AnyState();

        // return the upper state
        virtual AnyState * _upper(DedupOrchestrator &) = 0;

        // perform the 'do activity'
        virtual void _do(DedupOrchestrator &);

        // the current state doesn't manage the event stop, give it to the upper state
        virtual void stop(DedupOrchestrator & stm);

        // the current state doesn't manage the event message, give it to the upper state
        virtual void message(DedupOrchestrator & stm);

        // the current state doesn't manage the event elapsed, give it to the upper state
        virtual void elapsed(DedupOrchestrator & stm);

        virtual void create(DedupOrchestrator &);

    };
    
    // implement the state DedupOrchestrator
    class DedupOrchestrator_State : public AnyState {
      public:
        // implement the state Wait
        class Wait_State : public AnyState {
          public:
            virtual ~Wait_State();

            // to manage the event stop
            virtual void stop(DedupOrchestrator & stm);

            // to manage the event message
            virtual void message(DedupOrchestrator & stm);

            // to manage the event elapsed
            virtual void elapsed(DedupOrchestrator & stm);

            // returns the state containing the current
            virtual AnyState * _upper(DedupOrchestrator & stm);

        };
        
        // implement the state Deduplicate
        class Deduplicate_State : public AnyState {
          public:
            virtual ~Deduplicate_State();

            virtual bool _completion(DedupOrchestrator & stm);

            // to manage the event create
            virtual void create(DedupOrchestrator & stm);

            // perform the 'entry behavior'
            void _doentry(DedupOrchestrator & stm);

            // perform the 'exit behavior'
            void _doexit(DedupOrchestrator & stm);

            // returns the state containing the current
            virtual AnyState * _upper(DedupOrchestrator & stm);

        };
        
        virtual ~DedupOrchestrator_State();

        // memorize the instance of the state Wait, internal
        Wait_State _wait_state;

        // to manage the event create
        virtual void create(DedupOrchestrator & stm);

        // returns the state containing the current
        virtual AnyState * _upper(DedupOrchestrator &);

        // memorize the instance of the state Deduplicate, internal
        Deduplicate_State _deduplicate_state;

    };
    

  public:
    inline DedupOrchestrator();

    inline virtual ~DedupOrchestrator();


  protected:
    // the operation you call to signal the event message
    virtual bool message();

    // the operation you call to signal the event elapsed
    bool elapsed() override;


  public:
    void setOutputBoundary(const Timestamp & timestamp);

    void releaseOutputBoundary(const Timestamp & timestamp);


  protected:
    // memorize the instance of the state DedupOrchestrator, internal
    DedupOrchestrator_State _deduporchestrator_state;


  public:
    // the operation you call to signal the event stop
    bool stop();

  friend class DedupOrchestrator_State::Wait_State;
    // the operation you call to signal the event create
    bool create();

  friend class DedupOrchestrator_State;
    // to execute the current state 'do activity'
    void doActivity();


  protected:
    // change the current state, internal
    inline void _set_currentState(AnyState & st);

    // execution done, internal
    void _final();

    // contains the current state, internal
    AnyState * _current_state;

    virtual bool received() override;

    virtual bool opened();

    virtual bool volumeIncremented() override;

    virtual bool priceAdvanced() override;

    virtual bool priceReceded() override;

    virtual bool filled() override;

    virtual bool partiallyCancelled() override;

    virtual bool fullyCancelled() override;

  friend class DedupOrchestrator_State::Deduplicate_State;

  private:
    map<PriceSide, std::unique_ptr<Dedup>> dedups_;

    vector<PriceSide> contaminated_;

    map<Timestamp,int> gates_;

    vector<std::unique_ptr<MessageHandler::Message>> episode_;

};
inline DedupOrchestrator::DedupOrchestrator() {
    _current_state = 0;
}

inline DedupOrchestrator::~DedupOrchestrator() {
}

// change the current state, internal
inline void DedupOrchestrator::_set_currentState(DedupOrchestrator::AnyState & st) {
    _current_state = &st;
}

class ReconstructorImplementation : public Reconstructor {
  protected:
    typedef MessageHandler::Message Message;

    typedef MessageHandler::Elapsed Elapsed;


  public:
    virtual void process(const boost::property_tree::ptree & message) final;


  protected:
    virtual vector<std::unique_ptr<MessageHandler::Message>> extract(const boost::property_tree::ptree & tree) = 0;


  private:
    inline virtual vector<std::unique_ptr<MessageHandler::Message>> cleanse( vector<std::unique_ptr<MessageHandler::Message>> && messages);

    inline virtual void save( vector<std::unique_ptr<MessageHandler::Message>> &&  messages);


  protected:
    std::unique_ptr<DedupOrchestrator> deduplicator_;

    Store * store_;

};
inline vector<std::unique_ptr<MessageHandler::Message>> ReconstructorImplementation::cleanse( vector<std::unique_ptr<MessageHandler::Message>> && messages) {
  return deduplicator_->handle(std::move(messages));
}

inline void ReconstructorImplementation::save( vector<std::unique_ptr<MessageHandler::Message>> &&  messages) {
  for(auto &msg : messages) store_->transmit(msg->toEvent());
  
}

class CoinbaseReconstructor : public ReconstructorImplementation {
  private:
    class CoinbaseReceived : public MessageHandler::Received {
    };
    
    class CoinbaseMatch : public MessageHandler::Filled {
    };
    
    class CoinbaseDone : public MessageHandler::FullyCancelled {
    };
    
    class CoinbaseOpen : public MessageHandler::Opened {
      public:
        CoinbaseOpen(const Id & order_id, const Timestamp & timestamp, const Price & price, const Volume & remaining_size, BookSide side);

        //By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
        virtual std::unique_ptr<Event> toEvent();

        ~CoinbaseOpen();

    };
    

  protected:
    virtual vector<std::unique_ptr<MessageHandler::Message>> extract(const boost::property_tree::ptree & tree) override final;


  public:
    explicit CoinbaseReconstructor( Store * store);


  private:
    map<Id, Volume> remaining_size_;

};
class EventImp : public Event {
  private:
    Id order_id_;

    Timestamp timestamp_;

    EventNo event_no_;

    Price price_;

    Volume volume_;

    Id trade_id_;

    EventType event_type_;


  public:
    inline const Id getOrderId() const;

    inline const Timestamp getTimestamp() const;

    inline const EventNo getEventNo() const;

    inline const Price getPrice() const;

    inline const Volume getVolume() const;

    inline const Id getTradeId() const;

    inline const EventType getEventType() const;

    EventImp(const Id & order_id, const Timestamp & timestamp, const EventNo & event_no, const Price & price, const Volume & volume, EventType event_type);

};
inline const Id EventImp::getOrderId() const {
  return order_id_;
}

inline const Timestamp EventImp::getTimestamp() const {
  return timestamp_;
}

inline const EventNo EventImp::getEventNo() const {
  return event_no_;
}

inline const Price EventImp::getPrice() const {
  return price_;
}

inline const Volume EventImp::getVolume() const {
  return volume_;
}

inline const Id EventImp::getTradeId() const {
  return trade_id_;
}

inline const EventType EventImp::getEventType() const {
  return event_type_;
}


} // namespace oberon::core

} // namespace oberon
#endif
