#ifndef OBERON_CORE_RECONSTRUCTOR_IMPL_H
#define OBERON_CORE_RECONSTRUCTOR_IMPL_H


#include "reconstructor.h"
#include <string>
using namespace std;
#include <vector>

#include <boost/property_tree/ptree.hpp>


namespace oberon { namespace core { class Store; }  } 

namespace oberon {

namespace core {

class MessageHandler;
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

    PriceSide(const Price & price, BookSide side);

    PriceSide();

};
enum TradeRole {
  kMaker,
  kTaker
};
class EventImpl : public Event {
  private:
    OrderId order_id_;

    Timestamp timestamp_;

    EventNo event_no_;

    Price price_;

    Volume remaining_size_;

    Volume change_size_;

    TradeId trade_id_;

    EventType event_type_;


  public:
    inline const OrderId getOrderId() const;

    inline const Timestamp getTimestamp() const;

    inline const EventNo getEventNo() const;

    inline const Price getPrice() const;

    inline const Volume getRemainingSize() const;

    inline const Volume getChangeSize() const;

    inline const TradeId getTradeId() const;

    inline const EventType getEventType() const;

    EventImpl(const OrderId & order_id, const Timestamp & timestamp, const EventNo & event_no, const Price & price, const Volume & remaining_size, const Volume & change_size, EventType event_type);

    EventImpl(const OrderId & order_id, const Timestamp & timestamp, const EventNo & event_no, const Price & price, const Volume & remaining_size, const Volume & change_size, EventType event_type, const TradeId & trade_id);

};
inline const OrderId EventImpl::getOrderId() const {
  return order_id_;
}

inline const Timestamp EventImpl::getTimestamp() const {
  return timestamp_;
}

inline const EventNo EventImpl::getEventNo() const {
  return event_no_;
}

inline const Price EventImpl::getPrice() const {
  return price_;
}

inline const Volume EventImpl::getRemainingSize() const {
  return remaining_size_;
}

inline const Volume EventImpl::getChangeSize() const {
  return change_size_;
}

inline const TradeId EventImpl::getTradeId() const {
  return trade_id_;
}

inline const EventType EventImpl::getEventType() const {
  return event_type_;
}

class MessageHandler {
  public:
    class Message {
      protected:
        Timestamp timestamp_;


      public:
        inline const Timestamp getTimestamp() const;

        //By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
        virtual std::unique_ptr<Event> toEvent();

        virtual string toString();

        virtual bool accept(MessageHandler* mh) = 0;

        virtual ~Message();

    };
    
    class ExchangeMessage : public Message {
      public:
        PriceSide getPriceSide();


      protected:
        OrderId order_id_;


      public:
        inline const OrderId getOrderId() const;


      protected:
        Volume remaining_size_;


      public:
        inline const Volume getRemainingSize() const;

        void setRemainingSize(const Volume & value);


      protected:
        Volume change_size_;


      public:
        inline const Volume getChangeSize() const;

        void setChangeSize(Volume value);


      protected:
        EventNo event_no_;


      public:
        inline const EventNo getEventNo() const;


      protected:
        Price price_;


      public:
        inline const Price getPrice() const;


      protected:
        BookSide side_;


      public:
        inline const BookSide getSide() const;


      protected:
        TradeId trade_id_;


      public:
        inline const TradeId getTradeId() const;

        virtual string toString();


      protected:
        virtual const Volume getBaseMinSize() const = 0;

    };
    
    //match or trade with or without order_ids
    class Filled : public ExchangeMessage {
      protected:
        TradeRole role_;


      public:
        virtual bool accept(MessageHandler* mh);

        virtual string toString();

        //By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
        virtual std::unique_ptr<Event> toEvent();

    };
    
    //opened on Coinbase. Bitstamp, Bitfinex do not send
    class Opened : public ExchangeMessage {
      public:
        virtual bool accept(MessageHandler* mh);

        virtual string toString();

        //By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
        virtual std::unique_ptr<Event> toEvent();

    };
    
    //order change on Bitstamp with increased volume and the same price
    class VolumeIncremented : public ExchangeMessage {
      public:
        virtual string toString();

    };
    
    //order_changed on Bitstamp
    class PartiallyCanceled : public ExchangeMessage {
      public:
        virtual string toString();

    };
    
    //order_change on Bitstamp or Bitfinex with increased price for bid or decreased price for ask.  Coinbase or MOEX does not generate such messages
    class PriceAdvanced : public ExchangeMessage {
      public:
        virtual string toString();

    };
    
    //order_change on Bitstamp or Bitfinex with decremented price for bid or incremented price for ask.  Coinbase or MOEX does not generate such messages
    class PriceReceded : public ExchangeMessage {
      public:
        virtual string toString();

    };
    
    //order_deleted on Bitstamp, done on Coinbase, price =0 on Bitfinex
    class FullyCanceled : public ExchangeMessage {
      public:
        virtual bool accept(MessageHandler * mh) override final;

        virtual string toString();

        //By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
        virtual std::unique_ptr<Event> toEvent();

    };
    
    //order_created on Bitstamp, Bitfinex received on Coinbase
    class Received : public ExchangeMessage {
      public:
        virtual bool accept(MessageHandler * mh) override final;

        virtual string toString();

    };
    
    class Elapsed : public Message {
      public:
        bool accept( MessageHandler * mh) override final;

        Elapsed(const Timestamp & exchange_time);

        virtual string toString();

    };
    
    class Era : public Message {
      public:
        virtual bool accept(MessageHandler* mh);

        Era();

        virtual ~Era();

        virtual string toString();

    };
    

  protected:
    virtual bool elapsed() = 0;

    virtual bool received() = 0;

    virtual bool opened() = 0;

    virtual bool volumeIncremented() = 0;

    virtual bool priceAdvanced() = 0;

    virtual bool priceReceded() = 0;

    virtual bool filled() = 0;

    virtual bool partiallyCanceled() = 0;

    virtual bool fullyCanceled() = 0;

    std::unique_ptr<Message> received_;


  public:
vector<std::unique_ptr<Message>> handle(vector<std::unique_ptr<Message>> && messages);


  protected:
    vector<std::unique_ptr<Message>> output_;


  public:
    virtual bool create();


  protected:
    virtual string getHandlerName() = 0;

};
inline const Timestamp MessageHandler::Message::getTimestamp() const {
  return timestamp_;
}

inline const OrderId MessageHandler::ExchangeMessage::getOrderId() const {
  return order_id_;
}

inline const Volume MessageHandler::ExchangeMessage::getRemainingSize() const {
  return remaining_size_;
}

inline const Volume MessageHandler::ExchangeMessage::getChangeSize() const {
  return change_size_;
}

inline const EventNo MessageHandler::ExchangeMessage::getEventNo() const {
  return event_no_;
}

inline const Price MessageHandler::ExchangeMessage::getPrice() const {
  return price_;
}

inline const BookSide MessageHandler::ExchangeMessage::getSide() const {
  return side_;
}

inline const TradeId MessageHandler::ExchangeMessage::getTradeId() const {
  return trade_id_;
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
    virtual vector<std::unique_ptr<MessageHandler::Message>> cleanse( vector<std::unique_ptr<MessageHandler::Message>> && messages);

    inline virtual void save( vector<std::unique_ptr<MessageHandler::Message>> &&  messages);


  protected:
    Store * store_;

    std::unique_ptr<MessageHandler> deduplicator_;

    std::unique_ptr<MessageHandler> size_deducer_;

};
inline void ReconstructorImplementation::save( vector<std::unique_ptr<MessageHandler::Message>> &&  messages) {
  for(auto &msg : messages) store_->transmit(msg->toEvent());
  
}


} // namespace oberon::core

} // namespace oberon
#endif
