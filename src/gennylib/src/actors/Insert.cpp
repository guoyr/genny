#include <gennylib/actors/Insert.hpp>

#include <memory>

#include <yaml-cpp/yaml.h>

#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/database.hpp>

#include "log.hh"
#include <bsoncxx/json.hpp>
#include <gennylib/context.hpp>
#include <gennylib/value_generators.hpp>

namespace genny{

struct actor::Insert::PhaseConfig {
    PhaseConfig(const yaml::Pair& node) {
        collection = yaml::get<std::string>(node, "Collection");
        db = yaml::get<std::string>(node, "Database");
        document = yaml::get<yaml::Node>(node,"Document");
    }

    std::string collection;
    std::string db;
    yaml::Node document;
};


struct actor::Insert::PhaseState {
    PhaseState(PhaseContext& context, std::mt19937_64& rng, mongocxx::pool::entry& client, int id)
        : config(context.config()),
          database{client->database(config.db)},
          collection{database.collection(config.collection)},
          json_document{value_generators::makeDoc(config.document, rng)} {}

    PhaseConfig config;

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

ActorVector actor::Insert::producer(ActorContext& context) {
    if (yaml::get<std::string>(context.config(), "Type") != "Insert") {
        return {};
    }

    ActorVector out;
    out.emplace_back(std::make_unique<actor::Insert>(context));
    return out;
}

} // genny
