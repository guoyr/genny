#include "log.hh"

#include <gennylib/actors/HelloWorld.hpp>

#include <string>

#include <gennylib/ActorFactory.hpp>
#include <gennylib/Cast.hpp>
#include <gennylib/Iterable.hpp>
#include <gennylib/config.hpp>
#include <gennylib/context.hpp>
#include <gennylib/yaml-private.hh>

namespace genny {

struct actor::HelloWorld::PhaseConfig : public config::PhaseConfig, public Iterable::PhaseConfig{
    PhaseConfig(const yaml::Pair& node) : config::PhaseConfig(node), Iterable::PhaseConfig(node) {
        message = yaml::get<std::string, false>(node, "Message").value_or("Hello, World!");
    }
    std::string message;
};

struct actor::HelloWorld::ActorConfig : public config::ActorConfig,
                                        public Parallelizable::ActorConfig {
    ActorConfig(const yaml::Node& node)
        : config::ActorConfig(node), Parallelizable::ActorConfig(node) {}
};

struct actor::HelloWorld::PhaseState {
    PhaseState(PhaseContext& context)
        : config{std::dynamic_pointer_cast<PhaseConfig>(context.config())} {}

    std::shared_ptr<PhaseConfig> config;
};

void actor::HelloWorld::run() {
    for (auto&& [phase, state] : _loop) {
        for (auto _ : state) {
            auto op = this->_outputTimer.raii();
            BOOST_LOG_TRIVIAL(info) << state->config->message;
        }
    }
}

actor::HelloWorld::HelloWorld(ActorContext& context)
    : _id{nextActorId()},
      _outputTimer{context.timer("output", _id)},
      _operations{context.counter("operations", _id)},
      _loop{context} {}

namespace {
// Register the HelloWorld actor
using HelloWorldFactory = DefaultActorFactory<actor::HelloWorld>;
auto factoryHelloWorld = std::make_shared<HelloWorldFactory>("HelloWorld");
auto registerHelloWorld = Cast::Registration("HelloWorld", factoryHelloWorld);
}
} // genny
