#ifndef HEADER_FF3E897B_C747_468B_AAAC_EA6421DB0902_INCLUDED
#define HEADER_FF3E897B_C747_468B_AAAC_EA6421DB0902_INCLUDED

#include <iostream>

#include <mongocxx/client_session.hpp>

#include <gennylib/Actor.hpp>
#include <gennylib/PhaseLoop.hpp>
#include <gennylib/metrics.hpp>

namespace genny::actor {

class HelloWorld : public genny::Actor {

public:
    struct PhaseConfig;
    struct ActorConfig;

public:
    explicit HelloWorld(ActorContext& context);
    ~HelloWorld() = default;

    void run() override;

private:
    const ActorId _id;

    metrics::Timer _outputTimer;
    metrics::Counter _operations;

    struct PhaseState;
    PhaseLoop<PhaseState> _loop;
};

}  // namespace genny::actor

#endif  // HEADER_FF3E897B_C747_468B_AAAC_EA6421DB0902_INCLUDED
