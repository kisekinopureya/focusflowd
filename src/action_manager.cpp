#include "action_manager.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

ActionManager::ActionManager() = default;

bool ActionManager::ensureConfigDirectory(const std::string& configPath) {
    try {
        fs::path path(configPath);
        fs::path dir = path.parent_path();
        if (!dir.empty() && !fs::exists(dir)) {
            fs::create_directories(dir);
            std::cout << "Created config directory: " << dir << std::endl;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create config directory: " << e.what() << std::endl;
        return false;
    }
}

bool ActionManager::createDefaultConfig(const std::string& configPath) {
    try {
        ensureConfigDirectory(configPath);
        json data;
        data["actions"] = json::array();

        std::ofstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Failed to create default config: " << configPath << std::endl;
            return false;
        }

        file << data.dump(2) << std::endl;
        std::cout << "Created default config file: " << configPath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating default config: " << e.what() << std::endl;
        return false;
    }
}

bool ActionManager::loadActions(const std::string& configPath) {
    this->configPath = configPath;

    // Ensure directory exists
    ensureConfigDirectory(configPath);

    std::ifstream file(configPath);

    // If file doesn't exist, create a default one
    if (!file.is_open()) {
        std::cout << "Config file not found: " << configPath << std::endl;
        if (createDefaultConfig(configPath)) {
            std::cout << "Created default config, service will run with no actions" << std::endl;
            actions.clear();
            return true;
        }
        return false;
    }

    try {
        json data;
        file >> data;

        actions.clear();
        for (const auto& actionJson : data.at("actions")) {
            Action action = Action::from_json(actionJson);
            actions[action.name] = action;
        }
        std::cout << "Loaded " << actions.size() << " action(s) from config" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config file: " << e.what() << std::endl;
        return false;
    }
}

bool ActionManager::reloadActions() {
    if (configPath.empty()) {
        std::cerr << "Config path not set, cannot reload" << std::endl;
        return false;
    }
    std::cout << "Reloading configuration..." << std::endl;
    return loadActions(configPath);
}

bool ActionManager::saveActions(const std::string& configPath) const {
    // Ensure directory exists
    ensureConfigDirectory(configPath);

    std::ofstream file(configPath);

    if (!file.is_open()) {
        std::cerr << "Failed to open config file for writing: " << configPath << std::endl;
        return false;
    }

    try {
        json data;
        std::vector<json> actionList;
        for (const auto& [name, action] : actions) {
            actionList.push_back(action.to_json());
        }
        data["actions"] = actionList;

        file << data.dump(2) << std::endl;
        std::cout << "Saved " << actions.size() << " action(s) to config" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing config file: " << e.what() << std::endl;
        return false;
    }
}

const Action* ActionManager::getAction(const std::string& actionName) const {
    auto it = actions.find(actionName);
    if (it != actions.end()) {
        return &it->second;
    }
    return nullptr;
}

void ActionManager::executeStartActions(const std::string& actionName) {
    const Action* action = getAction(actionName);
    if (!action) {
        std::cerr << "Action not found: " << actionName << std::endl;
        return;
    }

    for (const auto& cmd : action->startActions) {
        executeAction(cmd);
    }
}

void ActionManager::executeEndActions(const std::string& actionName) {
    const Action* action = getAction(actionName);
    if (!action) {
        std::cerr << "Action not found: " << actionName << std::endl;
        return;
    }

    for (const auto& cmd : action->endActions) {
        executeAction(cmd);
    }
}

void ActionManager::executeAction(const std::string& command) {
    if (command.empty()) {
        return;
    }

    std::cout << "Executing: " << command << std::endl;
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Command failed with exit code: " << result << std::endl;
    }
}
