#ifndef HEADER_A5170346_CB57_4438_854F_20C3D99FF187_INCLUDED
#define HEADER_A5170346_CB57_4438_854F_20C3D99FF187_INCLUDED

#include <iostream>
#include <random>

#include <mongocxx/pool.hpp>

#include <gennylib/Actor.hpp>
#include <gennylib/PhaseLoop.hpp>
#include <gennylib/metrics.hpp>

namespace genny::actor {

/**
 * InsertRemove is a simple actor that inserts and then removes the same document from a
 * collection. It uses {@code PhaseLoop} for looping.  Each instance of the actor uses a
 * different document, indexed by an integer _id field. The actor records the latency of each
 * insert and each remove.
 */
class InsertRemove : public Actor {
public:
    struct PhaseConfig;
    struct ActorConfig;

public:
    explicit InsertRemove(ActorContext& context);
    ~InsertRemove() = default;

    void run() override;

private:
    std::mt19937_64 _rng;

    const ActorId _id;

    metrics::Timer _insertTimer;
    metrics::Timer _removeTimer;
    mongocxx::pool::entry _client;

    struct PhaseState;
    PhaseLoop<PhaseState> _loop;
};

}  // namespace genny::actor

#endif  // HEADER_A5170346_CB57_4438_854F_20C3D99FF187_INCLUDED
