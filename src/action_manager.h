#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Action {
    std::string name;
    std::vector<std::string> startActions;
    std::vector<std::string> endActions;

    json to_json() const {
        return json{
            {"name", name},
            {"startActions", startActions},
            {"endActions", endActions}
        };
    }

    static Action from_json(const json& j) {
        Action action;
        action.name = j.at("name").get<std::string>();
        action.startActions = j.at("startActions").get<std::vector<std::string>>();
        action.endActions = j.at("endActions").get<std::vector<std::string>>();
        return action;
    }
};

class ActionManager {
public:
    ActionManager();
    ~ActionManager() = default;

    // Load actions from configuration file (creates default if missing)
    bool loadActions(const std::string& configPath);

    // Reload actions from the current config file
    bool reloadActions();

    // Save actions to configuration file
    bool saveActions(const std::string& configPath) const;

    // Create a default empty configuration file
    static bool createDefaultConfig(const std::string& configPath);

    // Ensure config directory exists
    static bool ensureConfigDirectory(const std::string& configPath);

    // Get action by name
    const Action* getAction(const std::string& actionName) const;

    // Get all actions
    const std::map<std::string, Action>& getAllActions() const { return actions; }

    // Execute actions (start or end)
    void executeStartActions(const std::string& actionName);
    void executeEndActions(const std::string& actionName);

private:
    std::map<std::string, Action> actions;
    std::string configPath;

    void executeAction(const std::string& command);
};
