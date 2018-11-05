#pragma once

#include <map>
#include <memory>
#include <sstream>

#include <gennylib/ActorFactory.hpp>

namespace genny{

class Cast {
public:
    using Factory = ActorFactory;
    using Factories = std::map<std::string, std::shared_ptr<Factory>>;

    struct Registration;

    void add(const std::string & castName, std::shared_ptr<Factory> entry) {
        auto res = _factories.emplace(castName, std::move(entry));

        if (!res.second) {
            std::ostringstream stream;
            auto previousFactory = res.first->second;
            stream << "Failed to add '" << entry->name() << "' as '" << castName << "', '"
                   << previousFactory->name() << "' already added instead.";
            throw std::domain_error(stream.str());
        }
    }

    std::shared_ptr<Factory> getFactory(const std::string& name) const {
        try {
            return _factories.at(name);
        } catch (const std::out_of_range) {
            return {};
        }
    }

    const Factories& getFactories() const {
        return _factories;
    }

private:
    Factories _factories;
};

inline Cast & getCast(){
    static Cast _cast;
    return _cast;
}

struct Cast::Registration {
    Registration(const std::string & name, std::shared_ptr<Factory> entry) {
        getCast().add(name, std::move(entry));
    }
};

} // genny
