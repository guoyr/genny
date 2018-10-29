#include <string>

#include "log.hh"

#include <gennylib/actors/HelloWorld.hpp>

namespace genny {

struct actor::HelloWorld::PhaseConfig {
    std::string message;
    explicit PhaseConfig(PhaseContext& context)
        : message{context.get<std::string, false>("Message").value_or("Hello, World!")} {}
};

void actor::HelloWorld::run() {
    for (auto&& [phase, config] : _loop) {
        for (auto _ : config) {
            auto op = this->_outputTimer.raii();
            BOOST_LOG_TRIVIAL(info) << config->message;
        }
    }
}

actor::HelloWorld::HelloWorld(ActorContext& context)
    : _id{nextActorId()},
      _outputTimer{context.timer("output", _id)},
      _operations{context.counter("operations", _id)},
      _loop{context} {}

ActorVector actor::HelloWorld::producer(ActorContext& context) {
    if (context.get<std::string>("Type") != "HelloWorld") {
        return {};
    }

    ActorVector out;
    out.emplace_back(std::make_unique<actor::HelloWorld>(context));
    return out;
}

} // genny
