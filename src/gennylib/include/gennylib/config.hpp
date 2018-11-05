#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gennylib/Orchestrator.hpp>
#include <gennylib/yaml-forward.hpp>

namespace genny{
class ActorFactory;
}

namespace genny::config {
class PhaseConfig {
public:
    PhaseConfig(const yaml::Pair &);
    PhaseConfig(const PhaseConfig&) = delete;
    PhaseConfig& operator=(const PhaseConfig&) = delete;

    PhaseConfig(PhaseConfig&&) = delete;
    PhaseConfig& operator=(PhaseConfig&&) = delete;

    virtual ~PhaseConfig(){};

public:
    std::optional<PhaseNumber> number;
};

class ActorConfig {
public:
    ActorConfig(const yaml::Node & node);
    ActorConfig(const ActorConfig&) = delete;
    ActorConfig& operator=(const ActorConfig&) = delete;

    ActorConfig(ActorConfig&&) = delete;
    ActorConfig& operator=(ActorConfig&&) = delete;

    virtual ~ActorConfig(){};

    void addPhase(std::shared_ptr<PhaseConfig> phase) {
        phases.emplace_back(std::move(phase));
    }

public:
    std::string typeName;
    std::string logName;

    std::vector<std::shared_ptr<PhaseConfig>> phases;
};

class WorkloadConfig {
public:
    WorkloadConfig(const yaml::Node & node);
    WorkloadConfig(const WorkloadConfig&) = delete;
    WorkloadConfig& operator=(const WorkloadConfig&) = delete;

    WorkloadConfig(WorkloadConfig&&) = delete;
    WorkloadConfig& operator=(WorkloadConfig&&) = delete;

    virtual ~WorkloadConfig(){};

    void addActor(std::shared_ptr<ActorConfig> actor) {
        actors.emplace_back(std::move(actor));
    }

public:
    int randomSeed;
    std::string schemaVersion;

    std::vector<std::shared_ptr<ActorConfig>> actors;
};

std::shared_ptr<ActorFactory> getActorFactory(const yaml::Node& node);
std::shared_ptr<WorkloadConfig> loadNode(const yaml::Node& node);
std::shared_ptr<WorkloadConfig> loadFile(const std::string & file);
}  // namespace genny::config
