#pragma once

#include <memory>
#include <vector>

#include <gennylib/Actor.hpp>
#include <gennylib/Parallelizable.hpp>
#include <gennylib/config.hpp>
#include <gennylib/context.hpp>
#include <gennylib/yaml-forward.hpp>

namespace genny {
class ActorContext;
}

namespace genny {

class ActorFactory {
public:
    ActorFactory(const std::string & name) : _name{name} {}
    ActorFactory(const ActorFactory&) = delete;
    ActorFactory& operator=(const ActorFactory&) = delete;
    ActorFactory(ActorFactory&&) = default;
    ActorFactory& operator=(ActorFactory&&) = default;

    const std::string& name() const {
        return _name;
    }

    virtual std::shared_ptr<config::PhaseConfig> loadPhaseConfig(const yaml::Pair&) = 0;
    virtual std::shared_ptr<config::ActorConfig> loadActorConfig(const yaml::Node&) = 0;
    virtual std::vector<std::unique_ptr<Actor>> produce(ActorContext &) = 0;

private:
    const std::string _name;
};

template <class ActorT>
class DefaultActorFactory : public ActorFactory {
public:
    DefaultActorFactory(const std::string & name) : ActorFactory(name){}

    std::shared_ptr<config::PhaseConfig> loadPhaseConfig(const yaml::Pair& pair) override {
        using PhaseConfig = typename ActorT::PhaseConfig;
        return std::make_shared<PhaseConfig>(pair);
    }
    std::shared_ptr<config::ActorConfig> loadActorConfig(const yaml::Node& node) override {
        using ActorConfig = typename ActorT::ActorConfig;
        return std::make_shared<ActorConfig>(node);
    }
    std::vector<std::unique_ptr<Actor>> produce(ActorContext& context) override {
        std::vector<std::unique_ptr<Actor>> actors;

        auto parallelThreads = 1;
        auto parallelConfig =
            std::dynamic_pointer_cast<Parallelizable::ActorConfig>(context.config());
        if (parallelConfig) {
            parallelThreads = parallelConfig->parallelThreads;
        }

        for (int i = 0; i < parallelThreads; ++i) {
            actors.emplace_back(std::make_unique<ActorT>(context));
        }
        return actors;
    }
};

}  // namespace genny
