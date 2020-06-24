#ifndef OBERON_CORE_BITSTAMP_H
#define OBERON_CORE_BITSTAMP_H


#include "reconstructor_impl.h"
#include <map>

#include "reconstructor.h"
#include <list>
using namespace std;
#include <boost/property_tree/ptree.hpp>


namespace oberon { namespace core { class Store; }  } 

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
    
    class DeduplicateBitstamp : public MessageHandler {
      protected:
        virtual bool elapsed();

        virtual bool filled();

        virtual bool exchangeMessage();


      private:
        list<std::unique_ptr<MessageHandler::ExchangeMessage>> other_messages_;

        list<std::unique_ptr<MessageHandler::Filled>> fills_;

        const BitstampReconstructor &reconstructor_;


      public:
        DeduplicateBitstamp(const BitstampReconstructor & reconstructor);

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
    
    class BitstampCreated : public MessageHandler::Created, public BitstampMessage {
      public:
        BitstampCreated(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor);

    };
    
    class BitstampReceived : public MessageHandler::Received {
    };
    
    class BitstampOpened : public MessageHandler::Opened {
    };
    
    class BitstampCanceled : public MessageHandler::Canceled, public BitstampMessage {
      public:
        BitstampCanceled(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor);

    };
    
    class BitstampFilled : public MessageHandler::Filled, public BitstampMessage {
      public:
        BitstampFilled(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor);

    };
    
    class BitstampChanged : public MessageHandler::Changed, public BitstampMessage {
      public:
        BitstampChanged(const boost::property_tree::ptree & tree, const BitstampReconstructor & reconstructor);

    };
    
    BitstampReconstructor(Store * store, const Volume & base_min_size, const Volume & base_increment, bool extract_only);


  protected:
    virtual vector<std::unique_ptr<MessageHandler::Message>> extract(const boost::property_tree::ptree & tree);

};

} // namespace oberon::core

} // namespace oberon
#endif
