#pragma once

#include <chrono>
#include <optional>

#include <gennylib/yaml-forward.hpp>

namespace genny{

class Iterable {
public:
    class PhaseConfig {
    public:
        PhaseConfig(const yaml::Pair& node);

    public:
        std::optional<std::chrono::milliseconds> iterationDuration;
        std::optional<int> iterationRepeat;
    };

public:
    virtual ~Iterable() = 0;
};

inline Iterable::~Iterable() {}
} // namespace genny
