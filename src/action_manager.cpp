#include "action_manager.h"
#include <QProcess>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

namespace fs = std::filesystem;

ActionManager::ActionManager() = default;

bool ActionManager::ensureConfigDirectory(const std::string& configPath) {
    try {
        fs::path path(configPath);
        fs::path dir = path.parent_path();
        if (!dir.empty() && !fs::exists(dir)) {
            fs::create_directories(dir);
            std::cout << "Created config directory: " << dir << '\n';
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to create config directory: " << e.what() << '\n';
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
            std::cerr << "Failed to create default config: " << configPath << '\n';
            return false;
        }

        file << data.dump(2) << '\n';
        std::cout << "Created default config file: " << configPath << '\n';
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error creating default config: " << e.what() << '\n';
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
        std::cout << "Config file not found: " << configPath << '\n';
        if (createDefaultConfig(configPath)) {
            std::cout << "Created default config, service will run with no actions" << '\n';
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
            actions.push_back(Action::from_json(actionJson));
        }
        std::cout << "Loaded " << actions.size() << " action(s) from config" << '\n';
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing config file: " << e.what() << '\n';
        return false;
    }
}

bool ActionManager::reloadActions() {
    if (configPath.empty()) {
        std::cerr << "Config path not set, cannot reload" << '\n';
        return false;
    }
    std::cout << "Reloading configuration..." << '\n';
    return loadActions(configPath);
}

bool ActionManager::saveActions(const std::string& configPath) const {
    // Ensure directory exists
    ensureConfigDirectory(configPath);

    std::ofstream file(configPath);

    if (!file.is_open()) {
        std::cerr << "Failed to open config file for writing: " << configPath << '\n';
        return false;
    }

    try {
        json data;
        std::vector<json> actionList;
        actionList.reserve(actions.size());
        for (const auto& action : actions) {
            actionList.push_back(action.to_json());
        }
        data["actions"] = actionList;

        file << data.dump(2) << '\n';
        std::cout << "Saved " << actions.size() << " action(s) to config" << '\n';
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error writing config file: " << e.what() << '\n';
        return false;
    }
}

const Action* ActionManager::getAction(const std::string& actionName) const {
    for (const auto& action : actions) {
        if (action.name == actionName) {
            return &action;
        }
    }
    return nullptr;
}

void ActionManager::addAction(const Action& action) {
    actions.push_back(action);
}

void ActionManager::removeAction(size_t index) {
    if (index < actions.size()) {
        actions.erase(actions.begin() + static_cast<std::vector<Action>::difference_type>(index));
    }
}

void ActionManager::updateAction(size_t index, const Action& action) {
    if (index < actions.size()) {
        actions[index] = action;
    }
}

void ActionManager::executeStartActions(const std::string& actionName) const {
    const Action* action = getAction(actionName);
    if (action == nullptr) {
        std::cerr << "Action not found: " << actionName << '\n';
        return;
    }

    for (const auto& cmd : action->startActions) {
        executeAction(cmd);
    }
}

void ActionManager::executeEndActions(const std::string& actionName) const {
    const Action* action = getAction(actionName);
    if (action == nullptr) {
        std::cerr << "Action not found: " << actionName << '\n';
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

    std::cout << "Executing: " << command << '\n';
    const int result = QProcess::execute("/bin/sh", {"-lc", QString::fromStdString(command)});
    if (result != 0) {
        std::cerr << "Command failed with exit code: " << result << '\n';
    }
}
