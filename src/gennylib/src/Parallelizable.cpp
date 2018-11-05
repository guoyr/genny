#include <gennylib/Parallelizable.hpp>

#include <gennylib/yaml-private.hh>

namespace genny{
Parallelizable::ActorConfig::ActorConfig(const yaml::Node& node) {
    parallelThreads = yaml::get<int, /* Required = */ false>(node, "Threads").value_or(1);
}
}  // namespace genny
