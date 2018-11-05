#include <gennylib/actors/Insert.hpp>

#include <memory>

#include <yaml-cpp/yaml.h>

#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/database.hpp>

#include <bsoncxx/json.hpp>

#include "log.hh"

#include <gennylib/Cast.hpp>
#include <gennylib/Parallelizable.hpp>
#include <gennylib/config.hpp>
#include <gennylib/context.hpp>
#include <gennylib/value_generators.hpp>
#include <gennylib/yaml-private.hh>

namespace genny{

struct actor::Insert::PhaseConfig : public config::PhaseConfig, public Iterable::PhaseConfig {
    PhaseConfig(const yaml::Pair & node) : config::PhaseConfig(node), Iterable::PhaseConfig(node) {
        collection = yaml::get<std::string>(node, "Collection");
        db = yaml::get<std::string>(node, "Database");
        document = yaml::get<yaml::Node>(node,"Document");
    }

    std::string collection;
    std::string db;
    yaml::Node document;
};

struct actor::Insert::ActorConfig : public config::ActorConfig, public Parallelizable::ActorConfig {
    ActorConfig(const yaml::Node& node)
        : config::ActorConfig(node), Parallelizable::ActorConfig(node) {}
};

struct actor::Insert::PhaseState {
    PhaseState(PhaseContext& context, std::mt19937_64& rng, mongocxx::pool::entry& client, int id)
        : config{std::dynamic_pointer_cast<PhaseConfig>(context.config())},
          database{client->database(config->db)},
          collection{database.collection(config->collection)},
          json_document{value_generators::makeDoc(config->document, rng)} {}

    std::shared_ptr<PhaseConfig> config;
    mongocxx::database database;
    mongocxx::collection collection;
    std::unique_ptr<value_generators::DocumentGenerator> json_document;
};

void actor::Insert::run() {
    for (auto&& [phase, state] : _loop) {
        for (const auto&& _ : state) {
            auto op = _insertTimer.raii();
            bsoncxx::builder::stream::document mydoc{};
            auto view = state->json_document->view(mydoc);
            BOOST_LOG_TRIVIAL(info) << " Inserting " << bsoncxx::to_json(view);
            state->collection.insert_one(view);
            _operations.incr();
        }
    }
}

actor::Insert::Insert(ActorContext& context)
    : _rng{context.workload().createRNG()},
      _id{nextActorId()},
      _insertTimer{context.timer("insert", _id)},
      _operations{context.counter("operations", _id)},
      _client{std::move(context.client())},
      _loop{context, _rng, _client, _id} {}

namespace {
// Register the Insert actor
auto factoryInsert = std::make_shared<DefaultActorFactory<actor::Insert>>("Insert");
auto registerInsert = Cast::Registration("Insert", factoryInsert);
}
} // genny
