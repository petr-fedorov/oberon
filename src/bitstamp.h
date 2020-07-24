#ifndef OBERON_CORE_BITSTAMP_H
#define OBERON_CORE_BITSTAMP_H


#include "reconstructor_impl.h"
#include <map>

#include "reconstructor.h"
#include <list>
using namespace std;
#include <boost/property_tree/ptree.hpp>

#include <map>
using namespace std;

#include <iostream>
namespace oberon { namespace core { class Store; }  } 
namespace oberon { namespace core { class Price; }  } 

namespace oberon {

namespace core {

class DedupOrchestrator;
class BitstampReconstructor : public ReconstructorImplementation {
  protected:
    class DeduceSizeBitstamp : public MessageHandler {
      protected:
        virtual bool created();

        virtual bool changed();

        virtual bool canceled();


      private:
        map<OrderId, Volume> sizes_;

    };
    
    class Deduplicator : public MessageHandler {
      protected:
        virtual bool elapsed();

        virtual bool filled();

        virtual bool exchangeMessage();


      private:
        list<std::unique_ptr<MessageHandler::ExchangeMessage>> other_messages_;

        list<std::unique_ptr<MessageHandler::Filled>> fills_;

        const BitstampReconstructor &reconstructor_;


      public:
        Deduplicator(const BitstampReconstructor & reconstructor);

    };
    

  public:
    class BitstampMessage : virtual public MessageHandler::ExchangeMessage {
      protected:
        virtual const Volume getBaseMinSize() const;

        virtual const Volume getBaseIncrement() const;

        const BitstampReconstructor &reconstructor_;


      public:
        BitstampMessage(const BitstampReconstructor & reconstructor);

    };
    
    class BitstampOpened : public BitstampMessage, public MessageHandler::Opened {
      public:
        BitstampOpened(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor);

        virtual BitstampOpened* clone();

        BitstampOpened(const BitstampOpened & source) = default;

    };
    
    class BitstampCanceled : public MessageHandler::Canceled, public BitstampMessage {
      public:
        BitstampCanceled(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor);

        virtual BitstampCanceled* clone();

    };
    
    class BitstampFilled : public MessageHandler::Filled, public BitstampMessage {
      public:
        BitstampFilled(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor);

        virtual BitstampFilled* clone();

    };
    
    class BitstampChanged : public MessageHandler::Changed, public BitstampMessage {
      public:
        BitstampChanged(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor);

        virtual BitstampChanged* clone();

    };
    
    BitstampReconstructor(Store * store, const Volume & base_min_size, const Volume & base_increment, bool extract_only);


  protected:
    virtual vector<std::unique_ptr<MessageHandler::Message>> extract(const boost::property_tree::ptree & tree);


  private:
    class Classifier : public MessageHandler {
      public:
        Classifier(const BitstampReconstructor & reconstructor);


      private:
        multimap<Price, std::unique_ptr<MessageHandler::ExchangeMessage>, std::greater<Price>> bids_;

        multimap<Price, std::unique_ptr<MessageHandler::ExchangeMessage>, std::less<Price>> asks_;

        map<OrderId, Price> prices_;

        const BitstampReconstructor &reconstructor_;


      protected:
        virtual bool opened();


      private:
        template<class U, class T>
        inline bool classifyOpened(U * us, T * them);


      protected:
        virtual bool changed();


      private:
        template<class U, class T>
        inline bool classifyChanged(U * us, T * them);


      protected:
        virtual bool canceled();


      private:
        template<class U, class T>
        inline bool classifyCanceled(U * us, T * them);


      protected:
        virtual bool filled();


      private:
        template<class U, class T>
        inline bool classifyFilled(const U & us, const T & them);

    };
    
};
template<class U, class T>
inline bool BitstampReconstructor::Classifier::classifyOpened(U * us, T * them) {
  BitstampOpened *opened = dynamic_cast<BitstampOpened *>(received_.get());
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
  std::cout << opened->toString() << "\n";
#endif
  ExchangeMessage *msg = nullptr;
  auto comp = them -> key_comp();
  prices_.insert(decltype(prices_)::value_type{opened->getOrderId(),
                                               opened->getPrice()});
  if (!them->empty() && !comp(opened->getPrice(), them->begin()->first)) {
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
    std::cout << them ->begin()->second->toString() << " them best \n";
#endif
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
    std::cout << opened->toString() << "\nDropped\n";
#endif
    received_->set_is_deleted(true);
    us->insert(typename std::remove_reference<decltype(*us)>::type::value_type{
        opened->getPrice(), std::unique_ptr<ExchangeMessage>(opened->clone())});
    //received_.reset(nullptr);
    output_.push_back(std::move(received_));
  } else {
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
    std::cout << opened->toString() << "\nOutput\n";
#endif
    us->insert(typename std::remove_reference<decltype(*us)>::type::value_type{
        opened->getPrice(), std::unique_ptr<ExchangeMessage>(opened->clone())});
    output_.push_back(std::move(received_));
  }
  return true;
}

template<class U, class T>
inline bool BitstampReconstructor::Classifier::classifyChanged(U * us, T * them) {
  BitstampChanged *msg = dynamic_cast<BitstampChanged *>(received_.get());
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
  std::cout << msg->toString() << "\n";
#endif
  auto comp = them -> key_comp();
  Price previous_price = 0.0;
  Volume previous_volume = 0.0;
  ExchangeMessage *previous_msg = nullptr;
  try {
    previous_price = prices_.at(msg->getOrderId());
    for (auto iter = us->lower_bound(previous_price);
         iter != us->end() && iter->first == previous_price; ++iter)
      if (iter->second->getOrderId() == msg->getOrderId()) {
        previous_volume = iter->second->getRemainingSize();
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
        std::cout  << iter->second->toString()<< " removed from LOB \n";
#endif
        us->erase(iter);
        break;
      }
  } catch (const std::out_of_range &) {
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
        std::cout  << "Previous price not found\n";
#endif
  }
  
  prices_[msg->getOrderId()] = msg->getPrice();
  
  if (!them->empty() &&
      !comp(msg->getPrice(), them->begin()->second->getPrice())) {
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
    std::cout << them ->begin()->second->toString() <<" them best \n";
#endif
    received_->set_is_deleted(true);
    us->insert(typename std::remove_reference<decltype(*us)>::type::value_type{
        msg->getPrice(), std::unique_ptr<ExchangeMessage>(msg->clone())});
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
    std::cout << msg->toString() <<" added to LOB \nDropped\n";
#endif
    // received_.release();
    output_.push_back(std::move(received_));
  } else {
    us->insert(typename std::remove_reference<decltype(*us)>::type::value_type{
        msg->getPrice(), std::unique_ptr<ExchangeMessage>(msg->clone())});
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
    std::cout << msg->toString() <<" added to LOB \nOutput\n";
#endif
    output_.push_back(std::move(received_));
  }
  return true;
}

template<class U, class T>
inline bool BitstampReconstructor::Classifier::classifyCanceled(U * us, T * them) {
  BitstampCanceled *msg = dynamic_cast<BitstampCanceled *>(received_.get());
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
  std::cout << msg->toString() << "\n";
#endif
  try {
    auto previous_price = prices_.at(msg->getOrderId());
    for (auto iter = us->lower_bound(previous_price);
         iter != us->end() && iter->first == previous_price; ++iter)
      if (iter->second->getOrderId() == msg->getOrderId()) {
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
        std::cout << iter->second->toString()<< " removed from LOB\n";
#endif
        auto comp = them->key_comp();
        if(!iter->second->isDeleted() && (them->empty() || comp(previous_price, them->begin()->second->getPrice()))) {
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
          std::cout << "Output\n";
#endif
          output_.push_back(std::move(received_));
        }
        else {
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
          std::cout << "Dropped\n";
#endif
          received_->set_is_deleted(true);
          output_.push_back(std::move(received_));
        }
        us->erase(iter);
        break;
      }
  } catch (const std::out_of_range &) {
  
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
    std::cout << "Previous price not found\nDropped\n";
#endif
  }
  return true;
}

template<class U, class T>
inline bool BitstampReconstructor::Classifier::classifyFilled(const U & us, const T & them) {
  Filled *msg = dynamic_cast<Filled *>(received_.get());
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
  std::cout << msg->toString() << "\n";
#endif
  Price previous_price = 0.0;
  Volume previous_volume = 0.0;
  try {
    previous_price = prices_.at(msg->getOrderId());
    for (auto iter = us->lower_bound(previous_price);
         iter != us->end() && iter->second->getPrice() == previous_price;
         ++iter) {
      if (iter->second->getOrderId() == msg->getOrderId()) {
        previous_volume = iter->second->getRemainingSize();
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
        std::cout << iter->second->toString()<< " removed from LOB\n";
#endif
        us->erase(iter);
        break;
      }
    }
  } catch (const std::out_of_range &) {
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
    std::cout << "Previous price not found\n";
#endif
  }
  if(msg->getRemainingSize() >= reconstructor_.base_min_size_) {
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
    std::cout << "Added to LOB\n";
#endif
    us->insert(typename std::remove_reference<decltype(*us)>::type::value_type{
        msg->getPrice(), std::unique_ptr<ExchangeMessage>(msg->clone())});
  }
  output_.push_back(std::move(received_));
#ifdef VERBOSE_BITSTAMP_CLASSIFIER
  std::cout << "Output\n";
#endif
  return true;
}


} // namespace oberon::core

} // namespace oberon
#endif
