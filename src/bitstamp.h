#ifndef OBERON_CORE_BITSTAMP_H
#define OBERON_CORE_BITSTAMP_H


#include "reconstructor_impl.h"
#include "reconstructor.h"
#include <vector>

#include <string>
using namespace std;
#include <map>

#include <boost/property_tree/ptree.hpp>


namespace oberon { namespace core { class Store; }  } 

namespace oberon {

namespace core {

class DedupOrchestrator;
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

    virtual bool partiallyCanceled();

    virtual bool fullyCanceled();


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


  protected:
    virtual string getHandlerName();

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

  friend class DedupOrchestrator_State::Deduplicate_State;

  private:
    map<PriceSide, std::unique_ptr<Dedup>> dedups_;

    vector<PriceSide> contaminated_;

    map<Timestamp,int> gates_;

    vector<std::unique_ptr<MessageHandler::Message>> episode_;


  protected:
    virtual string getHandlerName();

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

class BitstampReconstructor : public ReconstructorImplementation {
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
