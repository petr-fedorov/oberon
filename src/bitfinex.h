#ifndef OBERON_CORE_BITFINEX_H
#define OBERON_CORE_BITFINEX_H


#include "reconstructor.h"
#include "reconstructor_impl.h"
#include <boost/property_tree/ptree.hpp>


namespace oberon { namespace core { class Store; }  } 

namespace oberon {

namespace core {

class BitfinexReconstructor : public ReconstructorImplementation {
  public:
    BitfinexReconstructor(Store * store, const Volume & base_min_size, const Volume & base_increment, bool extract_only);


  protected:
    virtual vector<std::unique_ptr<MessageHandler::Message>> extract(const boost::property_tree::ptree & tree);


  public:
    class BitfinexMessage : virtual public MessageHandler::ExchangeMessage {
      protected:
        virtual const Volume getBaseMinSize() const;

        virtual const Volume getBaseIncrement() const;


      public:
        BitfinexMessage(const BitfinexReconstructor & reconstructor);


      protected:
        const BitfinexReconstructor &reconstructor_;

    };
    
    class BitfinexChanged : public MessageHandler::Changed, public BitfinexMessage {
      public:
        BitfinexChanged(const boost::property_tree::ptree & tree, const BitfinexReconstructor & reconstructor);

    };
    
    class BitfinexCanceled : public MessageHandler::Canceled, public BitfinexMessage {
      public:
        BitfinexCanceled(const boost::property_tree::ptree & tree, const BitfinexReconstructor & reconstructor);

    };
    
    class BitfinexFilled : public MessageHandler::Filled, public BitfinexMessage {
      public:
        BitfinexFilled(const boost::property_tree::ptree & tree, const BitfinexReconstructor & reconstructor);

    };
    
};

} // namespace oberon::core

} // namespace oberon
#endif
