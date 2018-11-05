#ifndef HEADER_C7F4E568_590C_4D4D_B46F_766447E6AE31_INCLUDED
#define HEADER_C7F4E568_590C_4D4D_B46F_766447E6AE31_INCLUDED

#include <iostream>
#include <memory>
#include <random>

#include <mongocxx/pool.hpp>

#include <gennylib/Actor.hpp>
#include <gennylib/PhaseLoop.hpp>
#include <gennylib/metrics.hpp>

namespace genny::actor {

class Insert : public genny::Actor {

public:
    struct PhaseConfig;
    struct ActorConfig;

public:
    explicit Insert(ActorContext& context);
    ~Insert() = default;

    void run() override;

private:
    std::mt19937_64 _rng;

    const ActorId _id;

    metrics::Timer _insertTimer;
    metrics::Counter _operations;
    mongocxx::pool::entry _client;

    struct PhaseState;
    PhaseLoop<PhaseState> _loop;
};

}  // namespace genny::actor

#endif  // HEADER_C7F4E568_590C_4D4D_B46F_766447E6AE31_INCLUDED
