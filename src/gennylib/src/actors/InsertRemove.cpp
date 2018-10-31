#include <gennylib/actors/InsertRemove.hpp>

#include <memory>

#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>
#include <yaml-cpp/yaml.h>

#include "log.hh"
#include <gennylib/context.hpp>
#include <gennylib/value_generators.hpp>

namespace genny{

struct actor::InsertRemove::PhaseConfig{
    PhaseConfig(const yaml::Pair & node){
        collection = yaml::get<std::string>(node, "Collection");
        db = yaml::get<std::string>(node, "Database");
    }

    std::string collection;
    std::string db;
};

struct actor::InsertRemove::PhaseState {
    PhaseState(PhaseContext& context, std::mt19937_64& rng, mongocxx::pool::entry& client, int id)
        : config{context.config()},
          database{client->database(config.db)},
          collection{database.collection(config.collection)},
          myDoc{bsoncxx::builder::stream::document{} << "_id" << id
                                                     << bsoncxx::builder::stream::finalize} {}

    PhaseConfig config;

    mongocxx::database database;
    mongocxx::collection collection;
    bsoncxx::document::value myDoc;
};

void actor::InsertRemove::run() {
    for (auto&& [phase, state] : _loop) {
        for (auto&& _ : state) {
            BOOST_LOG_TRIVIAL(info) << " Inserting and then removing";
            {
                auto op = _insertTimer.raii();
                state->collection.insert_one(state->myDoc.view());
            }
            {
                auto op = _removeTimer.raii();
                state->collection.delete_many(state->myDoc.view());
            }
        }
    }
}

actor::InsertRemove::InsertRemove(ActorContext& context)
    : _rng{context.workload().createRNG()},
      _id{nextActorId()},
      _insertTimer{context.timer("insert", _id)},
      _removeTimer{context.timer("remove", _id)},
      _client{std::move(context.client())},
      _loop{context, _rng, _client, _id} {}

ActorVector actor::InsertRemove::producer(ActorContext& context) {
    if (yaml::get<std::string>(context.config(), "Type") != "InsertRemove") {
        return {};
    }

    ActorVector out;
    out.emplace_back(std::make_unique<actor::InsertRemove>(context));
    return out;
}

} // genny
