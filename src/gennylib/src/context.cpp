#include <gennylib/context.hpp>

#include <memory>
#include <sstream>

#include <gennylib/Cast.hpp>

// Helper method to convert Actors:[...] to ActorContexts
std::vector<std::unique_ptr<genny::ActorContext>> genny::WorkloadContext::constructActorContexts(
    const std::shared_ptr<config::WorkloadConfig> & config, WorkloadContext* workloadContext) {
    auto out = std::vector<std::unique_ptr<genny::ActorContext>>{};
    for (const auto& actorConfig : config->actors) {
        out.emplace_back(std::make_unique<genny::ActorContext>(actorConfig, *workloadContext));
    }
    return out;
}

genny::ActorVector genny::WorkloadContext::constructActors(
    const std::vector<std::unique_ptr<ActorContext>>& actorContexts) {
    auto actors = genny::ActorVector{};
    for (auto& actorContext : actorContexts) {
        auto name = actorContext->config()->typeName;
        auto factory = getCast().getFactory(name);
        if(!factory){
            std::ostringstream stream;
            stream << "Unable to construct actors: No factory for '" << name << "'.";
            throw std::out_of_range(stream.str());
        }
        for (auto&& actor : factory->produce(*actorContext)) {
            actors.push_back(std::move(actor));
        }
    }
    return actors;
}

// Helper method to convert Phases:[...] to PhaseContexts
std::unordered_map<genny::PhaseNumber, std::unique_ptr<genny::PhaseContext>>
genny::ActorContext::constructPhaseContexts(const std::shared_ptr<config::ActorConfig> & config,
                                            genny::ActorContext* actorContext) {
    std::unordered_map<genny::PhaseNumber, std::unique_ptr<genny::PhaseContext>> out;
    if (config->phases.empty()) {
        return out;
    }

    PhaseNumber nextIndex = 0;
    for (const auto& phase : config->phases) {
        auto index = phase->number ? *(phase->number) : nextIndex;
        auto[it, success] =
            out.try_emplace(index, std::make_unique<genny::PhaseContext>(phase, *actorContext));
        if (!success) {
            std::stringstream msg;
            msg << "Duplicate phase " << index;
            throw InvalidConfigurationException(msg.str());
        }
        nextIndex = std::max(index, nextIndex) + 1;
    }
    return out;
}
