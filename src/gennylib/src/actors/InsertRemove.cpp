#include <gennylib/actors/InsertRemove.hpp>

#include <memory>

#include <mongocxx/client.hpp>
#include <mongocxx/pool.hpp>

#include "log.hh"

#include <gennylib/Cast.hpp>
#include <gennylib/Parallelizable.hpp>
#include <gennylib/config.hpp>
#include <gennylib/context.hpp>
#include <gennylib/value_generators.hpp>
#include <gennylib/yaml-private.hh>

namespace genny {

struct actor::InsertRemove::PhaseConfig : public config::PhaseConfig, public Iterable::PhaseConfig{
    PhaseConfig(const yaml::Pair & node) : config::PhaseConfig(node), Iterable::PhaseConfig(node) {
        collection = yaml::get<std::string>(node, "Collection");
        db = yaml::get<std::string>(node, "Database");
    }

    std::string collection;
    std::string db;
};

struct actor::InsertRemove::ActorConfig : public config::ActorConfig,
                                          public Parallelizable::ActorConfig {
    ActorConfig(const yaml::Node& node)
        : config::ActorConfig(node), Parallelizable::ActorConfig(node) {}
};

struct actor::InsertRemove::PhaseState {
    PhaseState(PhaseContext& context, std::mt19937_64& rng, mongocxx::pool::entry& client, int id)
        : config{std::dynamic_pointer_cast<PhaseConfig>(context.config())},
          database{client->database(config->db)},
          collection{database.collection(config->collection)},
          myDoc{bsoncxx::builder::stream::document{} << "_id" << id
                                                     << bsoncxx::builder::stream::finalize} {}

    std::shared_ptr<PhaseConfig> config;
    mongocxx::database database;
    mongocxx::collection collection;
    bsoncxx::document::value myDoc;
};

void actor::InsertRemove::run() {
    for (auto && [ phase, state ] : _loop) {
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

namespace {
// Register the InsertRemove actor
auto factoryInsertRemove =
    std::make_shared<DefaultActorFactory<actor::InsertRemove>>("InsertRemove");
auto registerInsertRemove = Cast::Registration("InsertRemove", factoryInsertRemove);
}
} // genny
