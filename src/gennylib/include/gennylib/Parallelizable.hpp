#pragma once

#include <gennylib/yaml-forward.hpp>

namespace genny{
class Parallelizable {
public:
    class ActorConfig {
    public:
        ActorConfig(const yaml::Node& node);

    public:
        int parallelThreads;
    };

public:
    virtual ~Parallelizable() = 0;
};

inline Parallelizable::~Parallelizable() {}
} // namespace genny
