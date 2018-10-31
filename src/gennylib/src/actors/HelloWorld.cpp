#include <string>

#include "log.hh"

#include <gennylib/actors/HelloWorld.hpp>

namespace genny {

struct actor::HelloWorld::PhaseConfig {
    PhaseConfig(const yaml::Pair& node) {
        message = yaml::get<std::string, false>(node, "Message").value_or("Hello, World!");
    }

    std::string message;
};


struct actor::HelloWorld::PhaseState {
    explicit PhaseState(PhaseContext& context) : config(context.config()) {}

    PhaseConfig config;
};

void actor::HelloWorld::run() {
    for (auto&& [phase, state] : _loop) {
        for (auto _ : state) {
            auto op = this->_outputTimer.raii();
            BOOST_LOG_TRIVIAL(info) << state->config.message;
        }
    }
}

actor::HelloWorld::HelloWorld(ActorContext& context)
    : _id{nextActorId()},
      _outputTimer{context.timer("output", _id)},
      _operations{context.counter("operations", _id)},
      _loop{context} {}

ActorVector actor::HelloWorld::producer(ActorContext& context) {
    if (yaml::get<std::string>(context.config(), "Type") != "HelloWorld") {
        return {};
    }

    ActorVector out;
    out.emplace_back(std::make_unique<actor::HelloWorld>(context));
    return out;
}

} // genny
