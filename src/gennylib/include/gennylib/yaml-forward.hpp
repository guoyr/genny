#pragma once

namespace YAML {
class Node;
}

namespace genny::yaml {
using Node = ::YAML::Node;
struct Pair{
    const Node primary;
    const Node fallback;
};
}
