#ifndef CONFIGLOADER_H
#define CONFIGLOADER_H

#include <string>
#include <yaml-cpp/yaml.h>

class ConfigLoader {
public:
    static YAML::Node loadConfig(const std::string& filename);

    static YAML::Node getModelOwnerConfig(const YAML::Node& config, int id);
    static YAML::Node getClientConfig(const YAML::Node& config, int id);
    static YAML::Node getSuperDealerConfig(const YAML::Node& config, int id);
    static YAML::Node getDealerConfig(const YAML::Node& config, int id);
};

#endif // CONFIGLOADER_H
