#include <boost/log/trivial.hpp>

#include <gennylib/yaml-private.hh>

namespace genny::yaml {
Node loadFile(const std::string& fileName) {
    try {
        return YAML::LoadFile(fileName);
    } catch (const std::exception& ex) {
        BOOST_LOG_TRIVIAL(error) << "Error loading yaml from " << fileName << ": " << ex.what();
        throw;
    }
}
} // namespace genny
