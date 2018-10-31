#include <gennylib/context.hpp>
#include <memory>

// Helper method to convert Actors:[...] to ActorContexts
std::vector<std::unique_ptr<genny::ActorContext>> genny::WorkloadContext::constructActorContexts(
    const yaml::Node& node, WorkloadContext* workloadContext) {
    auto out = std::vector<std::unique_ptr<genny::ActorContext>>{};
    for (const auto& actor : yaml::get(node, "Actors")) {
        out.emplace_back(std::make_unique<genny::ActorContext>(actor, *workloadContext));
    }
    return out;
}

genny::ActorVector genny::WorkloadContext::constructActors(
    const std::vector<ActorProducer>& producers,
    const std::vector<std::unique_ptr<ActorContext>>& actorContexts) {
    auto actors = genny::ActorVector{};
    for (const auto& producer : producers)
        for (auto& actorContext : actorContexts)
            for (auto&& actor : producer(*actorContext))
                actors.push_back(std::move(actor));
    return actors;
}

// Helper method to convert Phases:[...] to PhaseContexts
std::unordered_map<genny::PhaseNumber, std::unique_ptr<genny::PhaseContext>>
genny::ActorContext::constructPhaseContexts(const yaml::Node&, genny::ActorContext* actorContext) {
    std::unordered_map<genny::PhaseNumber, std::unique_ptr<genny::PhaseContext>> out;
    auto phases = yaml::get<yaml::Node, false>(actorContext->config(), "Phases");
    if (!phases) {
        return out;
    }

    int index = 0;
    for (const yaml::Node& phase : *phases) {
        auto configuredIndex = phase["Phase"].as<genny::PhaseNumber>(index);
        auto[it, success] =
            out.try_emplace(configuredIndex,
                            std::make_unique<genny::PhaseContext>(
                                yaml::Pair{phase, actorContext->config()}, *actorContext));
        if (!success) {
            std::stringstream msg;
            msg << "Duplicate phase " << configuredIndex;
            throw InvalidConfigurationException(msg.str());
        }
        ++index;
    }
    return out;
}
