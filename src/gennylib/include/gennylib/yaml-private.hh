#pragma once

#include <functional>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include <yaml-cpp/yaml.h>

#include <gennylib/InvalidConfigurationException.hpp>
#include <gennylib/conventions.hpp>
#include <gennylib/yaml-forward.hpp>

static_assert(std::is_constructible<genny::yaml::Node>(),
              "Definition of YAML::Node was not mapped to genny::yaml::Node.");

namespace genny::V1 {

/**
 * If Required, type is Out, else it's optional<Out>
 */
template <class Out, bool Required = true>
struct MaybeOptional {
    using type = typename std::conditional<Required, Out, std::optional<Out>>::type;
};

/**
 * The "path" to a configured value. E.g. given the structure
 *
 * ```yaml
 * foo:
 *   bar:
 *     baz: [10,20,30]
 * ```
 *
 * The path to the 10 is "foo/bar/baz/0".
 *
 * This is used to report meaningful exceptions in the case of mis-configuration.
 */
class ConfigPath {

public:
    ConfigPath() = default;

    ConfigPath(ConfigPath&) = delete;
    void operator=(ConfigPath&) = delete;

    template <class T>
    void add(const T& elt) {
        _elements.push_back(elt);
    }
    auto begin() const {
        return std::begin(_elements);
    }
    auto end() const {
        return std::end(_elements);
    }

private:
    /**
     * The parts of the path, so for this structure
     *
     * ```yaml
     * foo:
     *   bar: [bat, baz]
     * ```
     *
     * If this `ConfigPath` represents "baz", then `_elements`
     * will be `["foo", "bar", 1]`.
     *
     * To be "efficient" we only store a `function` that produces the
     * path component string; do this to avoid (maybe) expensive
     * string-formatting in the "happy case" where the ConfigPath is
     * never fully serialized to an exception.
     */
    std::vector<std::function<void(std::ostream&)>> _elements;
};

// Support putting ConfigPaths onto ostreams
inline std::ostream& operator<<(std::ostream& out, const ConfigPath& path) {
    for (const auto& f : path) {
        f(out);
        out << "/";
    }
    return out;
}

// Used by get() in WorkloadContext and ActorContext
//
// This is the base-case when we're out of Args... expansions in the other helper below
template <class Out,
          class Current,
          bool Required = true,
          class OutV = typename MaybeOptional<Out, Required>::type>
OutV get_helper(const ConfigPath& parent, const Current& curr) {
    if (!curr) {
        if constexpr (Required) {
            std::stringstream error;
            error << "Invalid key at path [" << parent << "]";
            throw InvalidConfigurationException(error.str());
        } else {
            return std::nullopt;
        }
    }
    try {
        if constexpr (Required) {
            return curr.template as<Out>();
        } else {
            return std::make_optional<Out>(curr.template as<Out>());
        }
    } catch (const YAML::BadConversion& conv) {
        std::stringstream error;
        // typeid(Out).name() is kinda hokey but could be useful when debugging config issues.
        error << "Bad conversion of [" << curr << "] to [" << typeid(Out).name() << "] "
              << "at path [" << parent << "]: " << conv.what();
        throw InvalidConfigurationException(error.str());
    }
}

// Used by get() in WorkloadContext and ActorContext
//
// Recursive case where we pick off first item and recurse:
//      get_helper(foo, a, b, c) // this fn
//   -> get_helper(foo[a], b, c) // this fn
//   -> get_helper(foo[a][b], c) // this fn
//   -> get_helper(foo[a][b][c]) // "base case" fn above
template <class Out,
          class Current,
          bool Required = true,
          class OutV = typename MaybeOptional<Out, Required>::type,
          class PathFirst,
          class... PathRest>
OutV get_helper(ConfigPath& parent,
                const Current& curr,
                PathFirst&& pathFirst,
                PathRest&&... rest) {
    if (curr.IsScalar()) {
        std::stringstream error;
        error << "Wanted [" << parent << pathFirst << "] but [" << parent << "] is scalar: ["
              << curr << "]";
        throw InvalidConfigurationException(error.str());
    }
    const auto& next = curr[std::forward<PathFirst>(pathFirst)];

    parent.add([&](std::ostream& out) { out << pathFirst; });

    if (!next.IsDefined()) {
        if constexpr (Required) {
            std::stringstream error;
            error << "Invalid key [" << pathFirst << "] at path [" << parent << "]. Last accessed ["
                  << curr << "].";
            throw InvalidConfigurationException(error.str());
        } else {
            return std::nullopt;
        }
    }
    return V1::get_helper<Out, Current, Required>(parent, next, std::forward<PathRest>(rest)...);
}

}  // namespace genny::V1

namespace genny::yaml {

/**
 * Retrieve configuration values from the configuration node.
 * Returns `root[arg1][arg2]...[argN]`.
 *
 * This is somewhat expensive and should only be called during actor/workload setup.
 *
 * Typical usage:
 *
 * ```c++
 *     class MyActor ... {
 *       string name;
 *       MyActor(ActorContext& context)
 *       : name{context.get<string>("Name")} {}
 *     }
 * ```
 *
 * Given this YAML:
 *
 * ```yaml
 *     SchemaVersion: 2018-07-01
 *     Actors:
 *     - Name: Foo
 *       Count: 100
 *     - Name: Bar
 * ```
 *
 * Then traverse as with the following:
 *
 * ```c++
 *     auto schema = context.get<std::string>("SchemaVersion");
 *     auto actors = context.get("Actors"); // actors is a YAML::Node
 *     auto name0  = context.get<std::string>("Actors", 0, "Name");
 *     auto count0 = context.get<int>("Actors", 0, "Count");
 *     auto name1  = context.get<std::string>("Actors", 1, "Name");
 *
 *     // if value may not exist:
 *     std::optional<int> = context.get<int,false>("Actors", 0, "Count");
 * ```
 * @tparam T the output type required. Will forward to YAML::Node.as<T>()
 * @tparam Required If true, will error if item not found. If false, will return an optional<T>
 * that will be empty if not found.
 */
template <class T = Node,
          bool Required = true,
          class OutV = typename V1::MaybeOptional<T, Required>::type,
          class... Args>
OutV get(const Node& node, Args&&... args) {
    V1::ConfigPath p;
    return V1::get_helper<T, Node, Required>(p, node, std::forward<Args>(args)...);
}

template <class T = Node,
          bool Required = true,
          class OutV = typename V1::MaybeOptional<T, Required>::type,
          class... Args>
OutV get(const Pair& pair, Args&&... args) {
    V1::ConfigPath p;
    auto fromSelf = V1::get_helper<T, Node, /* Required = */ false>(
        p, pair.primary, std::forward<Args>(args)...);
    if (fromSelf) {
        if
            constexpr(Required) {
                // unwrap from optional<T>
                return *fromSelf;
            }
        else {
            // don't unwrap, return the optional<T> itself
            return fromSelf;
        }
    }

    // try the fallback node
    return V1::get_helper<T, Node, Required>(p, pair.fallback, std::forward<Args>(args)...);
}

Node loadFile(const std::string& fileName);

}  // namespace genny::yaml
