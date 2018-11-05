#include <gennylib/config.hpp>

#include <gennylib/Cast.hpp>
#include <gennylib/InvalidConfigurationException.hpp>
#include <gennylib/yaml-private.hh>

namespace genny::config {

PhaseConfig::PhaseConfig(const yaml::Pair& pair) {
    // Load in the index
    number = yaml::get<PhaseNumber, /* Required = */ false>(pair, "Phase");
}
ActorConfig::ActorConfig(const yaml::Node & node){
    // Load in the extra bits
    typeName = yaml::get<std::string>(node, "Type");
    logName = yaml::get<std::string, /* Required = */ false>(node, "Name").value_or(typeName);
}
WorkloadConfig::WorkloadConfig(const yaml::Node & node){
    randomSeed =
        yaml::get<int, /* Required = */ false>(node, "RandomSeed").value_or(269849313357703264);

    /**
     * This doesn't do any real verification of what a "Schema" even means and that's not okay.
     * I personally believe that 2018-07-01 implies at least:
     *   SchemaVersion: 2018-07-01
     *   Actors:
     *   - Type: Foo
     *     ...
     *
     * And potentially:
     *   SchemaVersion: 2018-07-01
     *   RandomSeed: 42
     *   Actors:
     *   - Type: Foo
     *     Name: Fooregard
     *     ...
     *     Phases:
     *     - Phase: 0
     *       ...
     */
    schemaVersion = yaml::get<std::string>(node, "SchemaVersion");
}

std::shared_ptr<ActorFactory> getActorFactory(const yaml::Node& yamlActor) {
    // Grab the factory entry for this Actor Type
    auto typeName = yaml::get<std::string>(yamlActor, "Type");
    auto factory = getCast().getFactory(typeName);
    if (!factory) {
        std::ostringstream stream;
        stream << "No ActorFactory for '" << typeName << "'.";
        throw InvalidConfigurationException(stream.str());
    }
    return factory;
}

std::shared_ptr<WorkloadConfig> loadConfig(const yaml::Node& yamlConfig) {
    // Load the base workload
    auto workloadConfig = std::make_shared<WorkloadConfig>(yamlConfig);
    if (workloadConfig->schemaVersion != "2018-07-01") {
        throw InvalidConfigurationException("Invalid schema version");
    }

    auto yamlActors = yaml::get<yaml::Node>(yamlConfig, "Actors");
    if (!yamlActors.IsSequence()) {
        std::ostringstream stream;
        stream << "'Actors' is not a sequence. This can't be right.";
        throw InvalidConfigurationException(stream.str());
    }

    // Loop through the actors
    for (const auto& yamlActor : yamlActors) {
        auto factory = getActorFactory(yamlActor);

        // Load in the ActorConfig type for this Actor
        auto actorConfig = factory->loadActorConfig(yamlActor);

        // If we don't have phases, we're done for this actor
        auto yamlPhases = yaml::get<yaml::Node, /* Required = */ false>(yamlActor, "Phases");
        if (!yamlPhases) 
            continue;

        if (!yamlPhases->IsSequence()) {
            std::ostringstream stream;
            stream << "'Phases' is not a sequence. This can't be right.";
            throw InvalidConfigurationException(stream.str());
        }

        // Load in the PhaseConfig type for this Actor
        for (const auto& yamlPhase : *yamlPhases) {
            auto phaseConfig = factory->loadPhaseConfig({yamlPhase, yamlActor});
            actorConfig->addPhase(phaseConfig);
        }

        workloadConfig->addActor(actorConfig);
    }
    return workloadConfig;
}

std::shared_ptr<WorkloadConfig> loadFile(const std::string& file) {
    return loadConfig(yaml::loadFile(file));
}

}  // namespace genny::config
