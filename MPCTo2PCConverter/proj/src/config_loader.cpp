#include "config_loader.h"

YAML::Node ConfigLoader::loadConfig(const std::string& filename) {
    return YAML::LoadFile(filename);
}

YAML::Node ConfigLoader::getModelOwnerConfig(const YAML::Node& config, int id = 0) {
    for (const auto& modelOwner : config["model_owners"]) {
        if (modelOwner["id"].as<int>() == id) {
            return modelOwner;
        }
    }
    static const YAML::Node nullNode;
    return nullNode;
}

YAML::Node ConfigLoader::getClientConfig(const YAML::Node& config, int id = 0) {
    for (const auto& client : config["clients"]) {
        if (client["id"].as<int>() == id) {
            return client;
        }
    }
    static const YAML::Node nullNode;
    return nullNode;
}

YAML::Node ConfigLoader::getSuperDealerConfig(const YAML::Node& config, int id = 0) {
    for (const auto& superDealer : config["super_dealers"]) {
        if (superDealer["id"].as<int>() == id) {
            return superDealer;
        }
    }
    static const YAML::Node nullNode;
    return nullNode;
}

YAML::Node ConfigLoader::getDealerConfig(const YAML::Node& config, int id = 0) {
    for (const auto& dealer : config["dealers"]) {
        if (dealer["id"].as<int>() == id) {
            return dealer;
        }
    }
    static const YAML::Node nullNode;
    return nullNode;
}
