// Copyright 2019-present MongoDB Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef HEADER_0BE8D22D_E93B_48FE_BC5A_CFFF2E05D861
#define HEADER_0BE8D22D_E93B_48FE_BC5A_CFFF2E05D861

#include <boost/exception/diagnostic_information.hpp>
#include <boost/log/trivial.hpp>

#include <loki/ScopeGuard.h>

#include <metrics/metrics.hpp>


namespace genny {

/**
 * Configuration for a `genny::RetryStrategy`.
 */
struct RetryOptions {
    /** Default values for each key */
    struct Defaults {
        static constexpr auto kMaxRetries = size_t{0};
        static constexpr auto kThrowOnFailure = false;
    };

    /** YAML keys to use */
    struct Keys {
        static constexpr auto kMaxRetries = "Retries";
        static constexpr auto kThrowOnFailure = "ThrowOnFailure";
    };

    size_t maxRetries = Defaults::kMaxRetries;
    bool throwOnFailure = Defaults::kThrowOnFailure;
};

}  // namespace genny


namespace YAML {

/**
 * Convert to/from `genny::RetryOptions` and YAML
 */
template <>
struct convert<genny::RetryOptions> {
    using Config = genny::RetryOptions;
    using Defaults = typename Config::Defaults;
    using Keys = typename Config::Keys;

    static Node encode(const Config& rhs) {
        Node node;

        node[Keys::kMaxRetries] = rhs.maxRetries;
        node[Keys::kThrowOnFailure] = rhs.throwOnFailure;

        return node;
    }

    static bool decode(const Node& node, Config& rhs) {
        if (!node.IsMap()) {
            return false;
        }

        genny::decodeNodeInto(rhs.maxRetries, node[Keys::kMaxRetries], Defaults::kMaxRetries);
        genny::decodeNodeInto(
            rhs.throwOnFailure, node[Keys::kThrowOnFailure], Defaults::kThrowOnFailure);

        return true;
    }
};


}  // namespace YAML


namespace genny {

class ActorContext;

/**
 * A small wrapper for running Mongo commands and recording metrics.
 *
 * The RetryStrategy allows the user to specify a maximum number of retries for failed
 * operations. Note that failed operations do not throw -- It is the user's responsibility to check
 * `lastResult()` when different behavior is desired for failed operations.
 */
class RetryStrategy {
public:
    struct Result {
        bool wasSuccessful = false;
        size_t numAttempts = 0;
    };

    using Options = RetryOptions;

public:
    explicit RetryStrategy(metrics::Operation op) : _op{std::move(op)} {}
    ~RetryStrategy() = default;

    template <typename F>
    void run(F&& fun, const Options& options = Options{}) {
        Result result;

        // Always report our results, even if we threw
        auto guard = Loki::MakeGuard([&]() { _finishRun(options, std::move(result)); });

        bool shouldContinue = true;
        while (shouldContinue) {
            auto ctx = _op.start();
            try {
                ++result.numAttempts;

                fun(ctx);

                ctx.success();
                result.wasSuccessful = true;
                shouldContinue = false;
            } catch (const boost::exception& e) {
                BOOST_LOG_TRIVIAL(debug) << "Caught error: " << boost::diagnostic_information(e);
                ctx.discard();

                // We should continue if we've attempted less than the amount of retries plus one
                // for the original attempt
                shouldContinue = result.numAttempts <= options.maxRetries;
                if (!shouldContinue) {
                    result.wasSuccessful = false;

                    if (options.throwOnFailure) {
                        throw;
                    }
                }
            }
        }
    }

    const Result& lastResult() const {
        return _lastResult;
    }

private:
    void _finishRun(const Options& options, Result result);

    metrics::Operation _op;
    Result _lastResult;
};
}  // namespace genny

#endif  // HEADER_0BE8D22D_E93B_48FE_BC5A_CFFF2E05D861
