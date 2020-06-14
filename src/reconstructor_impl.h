#ifndef OBERON_CORE_RECONSTRUCTOR_IMPL_H
#define OBERON_CORE_RECONSTRUCTOR_IMPL_H


#include "reconstructor.h"
#include <string>
using namespace std;
#include <vector>

#include <map>

#include <boost/property_tree/ptree.hpp>


namespace oberon { namespace core { class Store; }  } 

namespace oberon {

namespace core {

class MessageHandler;
enum BookSide {
  kBid,
  kAsk,
  kBookSideNA
};
struct PriceSide {
    BookSide side_;

    Price price_;

    bool operator <(const PriceSide & rhs) const;

    PriceSide(const Price & price, BookSide side);

    PriceSide();

};
class EventImpl : public Event {
  private:
    OrderId order_id_;

    Timestamp timestamp_;

    EventNo event_no_ = Event::kNaEventNo;

    OrderState state_;

    Price price_;

    Volume volume_;

    Volume delta_volume_;

    TradeId trade_id_;

    OrderId taker_order_id_ = Event::kNaOrderId;

    Timestamp local_timestamp_;


  public:
    EventImpl(const OrderId & order_id, const Timestamp & timestamp, const Timestamp & local_timestamp, const EventNo & event_no, const Price & price, const Volume & volume, const Volume & delta_volume, OrderState state);

    EventImpl(const OrderId & order_id, const Timestamp & timestamp, const Timestamp & local_timestamp, const EventNo & event_no, const Price & price, const Volume & volume, const Volume & delta_volume, OrderState state, const TradeId & trade_id, const OrderId & taker_order_id);

    virtual const Timestamp timestamp() const;

    virtual const OrderId orderId() const;

    virtual const EventNo eventNo() const;

    virtual const OrderState state() const;

    virtual const Price price() const;

    virtual const Volume volume() const;

    virtual const TradeId tradeId() const;

    virtual const Volume deltaVolume() const;

    virtual OrderId takerOrderId();

    virtual const Timestamp localTimestamp() const;

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

        virtual string toString();

        virtual bool accept(MessageHandler* mh) = 0;

        virtual ~Message();

    };
    
    class ExchangeMessage : public Message {
      public:
        PriceSide getPriceSide();


      protected:
        OrderId order_id_ = Event::kNaOrderId;


      public:
        inline const OrderId getOrderId() const;


      protected:
        Volume remaining_size_ = Event::kNaVolume;


      public:
        inline const Volume getRemainingSize() const;

        void setRemainingSize(const Volume & value);


      protected:
        Volume change_size_ = Event::kNaVolume;


      public:
        inline const Volume getChangeSize() const;

        void setChangeSize(Volume value);


      protected:
        EventNo event_no_ = Event::kNaEventNo;


      public:
        inline const EventNo getEventNo() const;

        void setEventNo(EventNo value);


      protected:
        Price price_ = Event::kNaPrice;


      public:
        inline const Price getPrice() const;


      protected:
        BookSide side_ = kBookSideNA;


      public:
        inline const BookSide getSide() const;


      protected:
        Timestamp local_timestamp_;


      public:
        inline const Timestamp getLocalTimestamp() const;

        virtual string toString();


      protected:
        virtual const Volume getBaseMinSize() const = 0;

        virtual const Volume getBaseIncrement() const = 0;

        const Volume roundToBaseIncrement(const Volume & volume) const;

    };
    
    class Created : virtual public ExchangeMessage {
      public:
        //By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
        virtual std::unique_ptr<Event> toEvent();

        virtual bool accept(MessageHandler* mh);

    };
    
    class Changed : virtual public ExchangeMessage {
      public:
        //By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
        virtual std::unique_ptr<Event> toEvent();

        virtual bool accept(MessageHandler* mh);

    };
    
    //match or trade with or without order_ids
    class Filled : virtual public ExchangeMessage {
      protected:
        OrderId taker_order_id_ = Event::kNaOrderId;

        TradeId trade_id_ = Event::kNaTradeId;


      public:
        virtual bool accept(MessageHandler* mh);

        inline const TradeId getTradeId() const;

        virtual string toString();

        //By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
        virtual std::unique_ptr<Event> toEvent();

    };
    
    //opened on Coinbase. Bitstamp, Bitfinex do not send
    class Opened : public Created {
      public:
        virtual bool accept(MessageHandler* mh);

        virtual string toString();

    };
    
    //order change on Bitstamp with increased volume and the same price
    class VolumeIncremented : public Changed {
      public:
        virtual string toString();

    };
    
    //order_changed on Bitstamp
    class VolumeDecremented : public Changed {
      public:
        virtual string toString();

    };
    
    //order_change on Bitstamp or Bitfinex with increased price for bid or decreased price for ask.  Coinbase or MOEX does not generate such messages
    class PriceAdvanced : public Changed {
      public:
        virtual string toString();

    };
    
    //order_change on Bitstamp or Bitfinex with decremented price for bid or incremented price for ask.  Coinbase or MOEX does not generate such messages
    class PriceReceded : public Changed {
      public:
        virtual string toString();

    };
    
    //order_deleted on Bitstamp, done on Coinbase, price =0 on Bitfinex
    class Canceled : virtual public ExchangeMessage {
      public:
        virtual bool accept(MessageHandler * mh) override final;

        virtual string toString();

        //By default, toEvent() returns 0 Events. A derived class that overrides this method is supposed to return 1 Event
        virtual std::unique_ptr<Event> toEvent();

    };
    
    //order_created on Bitstamp, Bitfinex received on Coinbase
    class Received : public Created {
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
    
    virtual bool create();

vector<std::unique_ptr<Message>> handle(vector<std::unique_ptr<Message>> && messages);


  protected:
    virtual string getHandlerName();

    virtual bool message();

    virtual bool elapsed();

    virtual bool exchangeMessage();

    virtual bool created();

    virtual bool received();

    virtual bool opened();

    virtual bool changed();

    virtual bool volumeIncremented();

    virtual bool volumeDecremented();

    virtual bool priceAdvanced();

    virtual bool priceReceded();

    virtual bool filled();

    virtual bool canceled();

    std::unique_ptr<Message> received_;

    vector<std::unique_ptr<Message>> output_;


  public:
    MessageHandler();

    virtual ~MessageHandler();

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

inline const Timestamp MessageHandler::ExchangeMessage::getLocalTimestamp() const {
  return local_timestamp_;
}

inline const TradeId MessageHandler::Filled::getTradeId() const {
  return trade_id_;
}

class EventNumberGenerator : public MessageHandler {
  private:
    map<OrderId, EventNo> eventNumbers_;


  protected:
    virtual bool exchangeMessage();

    virtual bool received();

    virtual bool fullyCanceled();

    virtual bool filled();

    virtual string getHandlerName();

};
class ReconstructorImplementation : public Reconstructor {
  protected:
    //The base_min_size and base_max_size fields define the min and max order size. The quote_increment field specifies the min order price as well as the price increment.
    //The order price must be a multiple of this increment (i.e. if the increment is 0.01, order prices of 0.001 or 0.021 would be rejected).
    Volume base_min_size_;

    Volume base_increment_;

    bool extract_only_;

    typedef MessageHandler::Message Message;

    typedef MessageHandler::Elapsed Elapsed;


  public:
    virtual void process(const boost::property_tree::ptree & message) final;


  protected:
    virtual vector<std::unique_ptr<MessageHandler::Message>> extract(const boost::property_tree::ptree & tree) = 0;


  private:
    virtual vector<std::unique_ptr<MessageHandler::Message>> cleanse( vector<std::unique_ptr<MessageHandler::Message>> && messages);

    inline virtual void transmit( vector<std::unique_ptr<MessageHandler::Message>> &&  messages);


  protected:
    Store * store_;

    std::unique_ptr<MessageHandler> deduplicator_;

    std::unique_ptr<MessageHandler> size_deducer_;

    std::unique_ptr<MessageHandler> event_number_generator_;

};
inline void ReconstructorImplementation::transmit( vector<std::unique_ptr<MessageHandler::Message>> &&  messages) {
  for(auto &msg : messages) store_->transmit(msg->toEvent());
  
}


} // namespace oberon::core

} // namespace oberon
#endif
