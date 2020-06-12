#ifndef OBERON_CORE_COINBASE_H
#define OBERON_CORE_COINBASE_H


#include "reconstructor_impl.h"
#include "reconstructor.h"
#include <boost/property_tree/ptree.hpp>

#include <map>

#include <string>
using namespace std;

namespace oberon { namespace core { class Store; }  } 

namespace oberon {

namespace core {

class CoinbaseReconstructor : public ReconstructorImplementation {
  public:
    class CoinbaseMessage : virtual public MessageHandler::ExchangeMessage {
      protected:
        const CoinbaseReconstructor &reconstructor_;


      public:
        CoinbaseMessage(const CoinbaseReconstructor & reconstructor);


      protected:
        virtual const Volume getBaseMinSize() const;

        virtual const Volume getBaseIncrement() const;

    };
    

  private:
    class CoinbaseReceived : public MessageHandler::Received, public CoinbaseMessage {
      public:
        CoinbaseReceived(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor);

    };
    
    class CoinbaseMatch : public MessageHandler::Filled, public CoinbaseMessage {
      public:
        CoinbaseMatch(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor);

    };
    
    class CoinbaseDoneCanceled : public MessageHandler::FullyCanceled, public CoinbaseMessage {
      public:
        CoinbaseDoneCanceled(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor);

    };
    
    class CoinbaseOpen : public MessageHandler::Opened, public CoinbaseMessage {
      public:
        CoinbaseOpen(const boost::property_tree::ptree & tree, const CoinbaseReconstructor & reconstructor);

    };
    
    //The base_min_size and base_max_size fields define the min and max order size. The quote_increment field specifies the min order price as well as the price increment.
    //The order price must be a multiple of this increment (i.e. if the increment is 0.01, order prices of 0.001 or 0.021 would be rejected).
    Volume base_min_size_;

    Volume base_increment_;


  protected:
    virtual vector<std::unique_ptr<MessageHandler::Message>> extract(const boost::property_tree::ptree & tree) override final;


  public:
    explicit CoinbaseReconstructor( Store * store, const Volume & base_min_size, const Volume & base_increment, bool extract_only);


  private:
    map<OrderId, Volume> remaining_size_;

};
class Deduce_Size_Coinbase_Extensions : public MessageHandler {
  protected:
    Volume previous_size_;

   map<OrderId, Volume> previous_sizes_;

    std::unique_ptr<MessageHandler::ExchangeMessage> em_;


  public:
    virtual bool message() = 0;


  protected:
    virtual bool volumeIncremented();

    virtual bool priceAdvanced();

    virtual bool priceReceded();

    virtual bool partiallyCanceled();

    virtual bool fullyCanceled();

    virtual bool opened();

    virtual string getHandlerName();

};
// implement the state machine Deduce Size Coinbase
class Deduce_Size_Coinbase : public Deduce_Size_Coinbase_Extensions {
  protected:
    // Mother class of all the classes representing states
    class AnyState {
      public:
        virtual ~AnyState();

        // return the upper state
        virtual AnyState * _upper(Deduce_Size_Coinbase &) = 0;

        // perform the 'do activity'
        virtual void _do(Deduce_Size_Coinbase &);

        // the current state doesn't manage the event received, give it to the upper state
        virtual void received(Deduce_Size_Coinbase & stm);

        // the current state doesn't manage the event elapsed, give it to the upper state
        virtual void elapsed(Deduce_Size_Coinbase & stm);

        // the current state doesn't manage the event era, give it to the upper state
        virtual void era(Deduce_Size_Coinbase & stm);

        // the current state doesn't manage the event filled, give it to the upper state
        virtual void filled(Deduce_Size_Coinbase & stm);

        // the current state doesn't manage the event message, give it to the upper state
        virtual void message(Deduce_Size_Coinbase & stm);

        virtual void create(Deduce_Size_Coinbase &);

    };
    
    // implement the state Deduce Size Coinbase
    class Deduce_Size_Coinbase_State : public AnyState {
      public:
        // implement the state Wait
        class Wait_State : public AnyState {
          public:
            virtual ~Wait_State();

            // to manage the event received
            virtual void received(Deduce_Size_Coinbase & stm);

            // to manage the event elapsed
            virtual void elapsed(Deduce_Size_Coinbase & stm);

            // to manage the event era
            virtual void era(Deduce_Size_Coinbase & stm);

            // to manage the event filled
            virtual void filled(Deduce_Size_Coinbase & stm);

            // to manage the event message
            virtual void message(Deduce_Size_Coinbase & stm);

            // perform the 'exit behavior'
            void _doexit(Deduce_Size_Coinbase & stm);

            // returns the state containing the current
            virtual AnyState * _upper(Deduce_Size_Coinbase & stm);

        };
        
        // implement the state Save
        class Save_State : public AnyState {
          public:
            virtual ~Save_State();

            virtual bool _completion(Deduce_Size_Coinbase & stm);

            // to manage the event create
            virtual void create(Deduce_Size_Coinbase & stm);

            // perform the 'entry behavior'
            void _doentry(Deduce_Size_Coinbase & stm);

            // returns the state containing the current
            virtual AnyState * _upper(Deduce_Size_Coinbase & stm);

        };
        
        // implement the state Deduce
        class Deduce_State : public AnyState {
          public:
            // implement the state Size change
            class Size_change_State : public AnyState {
              public:
                virtual ~Size_change_State();

                virtual bool _completion(Deduce_Size_Coinbase & stm);

                // to manage the event create
                virtual void create(Deduce_Size_Coinbase & stm);

                // returns the state containing the current
                virtual AnyState * _upper(Deduce_Size_Coinbase & stm);

            };
            
            // implement the state Remaining size
            class Remaining_size_State : public AnyState {
              public:
                virtual ~Remaining_size_State();

                virtual bool _completion(Deduce_Size_Coinbase & stm);

                // to manage the event create
                virtual void create(Deduce_Size_Coinbase & stm);

                // perform the 'entry behavior'
                void _doentry(Deduce_Size_Coinbase & stm);

                // returns the state containing the current
                virtual AnyState * _upper(Deduce_Size_Coinbase & stm);

            };
            
            virtual ~Deduce_State();

            // memorize the instance of the state Size change, internal
            Size_change_State _size_change_state;

            // memorize the instance of the state Remaining size, internal
            Remaining_size_State _remaining_size_state;

            // to manage the exit point 'exit', defined because probably shared
            void _exit1(Deduce_Size_Coinbase & stm);

            // perform the 'entry behavior'
            void _doentry(Deduce_Size_Coinbase & stm);

            // perform the 'exit behavior'
            void _doexit(Deduce_Size_Coinbase & stm);

            // returns the state containing the current
            virtual AnyState * _upper(Deduce_Size_Coinbase & stm);

        };
        
        virtual ~Deduce_Size_Coinbase_State();

        // memorize the instance of the state Wait, internal
        Wait_State _wait_state;

        // memorize the instance of the state Deduce, internal
        Deduce_State _deduce_state;

        // memorize the instance of the state Save, internal
        Save_State _save_state;

        // to manage the event create
        virtual void create(Deduce_Size_Coinbase & stm);

        // returns the state containing the current
        virtual AnyState * _upper(Deduce_Size_Coinbase &);

    };
    
    // memorize the instance of the state Deduce Size Coinbase, internal
    Deduce_Size_Coinbase_State _deduce_size_coinbase_state;


  public:
    Deduce_Size_Coinbase();

    virtual ~Deduce_Size_Coinbase();

    // the operation you call to signal the event received
    bool received();

    // the operation you call to signal the event elapsed
    bool elapsed();

    // the operation you call to signal the event era
    bool era();

    // the operation you call to signal the event filled
    bool filled();

    // the operation you call to signal the event message
    bool message();

  friend class Deduce_Size_Coinbase_State::Wait_State;
    // the operation you call to signal the event create
    bool create();

  friend class Deduce_Size_Coinbase_State::Deduce_State::Size_change_State;
  friend class Deduce_Size_Coinbase_State::Deduce_State::Remaining_size_State;
  friend class Deduce_Size_Coinbase_State::Deduce_State;
  friend class Deduce_Size_Coinbase_State::Save_State;
  friend class Deduce_Size_Coinbase_State;
    // to execute the current state 'do activity'
    void doActivity();


  protected:
    // change the current state, internal
    inline void _set_currentState(AnyState & st);

    // execution done, internal
    void _final();

    // contains the current state, internal
    AnyState * _current_state;

};
// change the current state, internal
inline void Deduce_Size_Coinbase::_set_currentState(Deduce_Size_Coinbase::AnyState & st) {
    _current_state = &st;
}


} // namespace oberon::core

} // namespace oberon
#endif
