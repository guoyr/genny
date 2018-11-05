#include <gennylib/Iterable.hpp>

#include <gennylib/yaml-private.hh>

namespace genny{
Iterable::PhaseConfig::PhaseConfig(const yaml::Pair& node) {
    iterationDuration =
        yaml::get<std::chrono::milliseconds, /* Required = */ false>(node, "Duration");
    iterationRepeat = yaml::get<int, /* Required = */ false>(node, "Repeat");
}
}  // namespace genny
