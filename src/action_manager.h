#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

struct Action {
    std::string name;
    std::vector<std::string> startActions;
    std::vector<std::string> endActions;

    [[nodiscard]] json to_json() const {
        return json{
            {"name", name},
            {"startActions", startActions},
            {"endActions", endActions}
        };
    }

    static Action from_json(const json& actionJson) {
        Action action;
        action.name = actionJson.at("name").get<std::string>();
        action.startActions = actionJson.at("startActions").get<std::vector<std::string>>();
        action.endActions = actionJson.at("endActions").get<std::vector<std::string>>();
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
    [[nodiscard]] bool saveActions(const std::string& configPath) const;

    // Create a default empty configuration file
    static bool createDefaultConfig(const std::string& configPath);

    // Ensure config directory exists
    static bool ensureConfigDirectory(const std::string& configPath);

    // Get action by name
    [[nodiscard]] const Action* getAction(const std::string& actionName) const;

    // Get all actions
    [[nodiscard]] const std::vector<Action>& getActions() const { return actions; }
    std::vector<Action>& getActions() { return actions; }

    // Add a new action
    void addAction(const Action& action);

    // Remove action by index
    void removeAction(size_t index);

    // Update action at index
    void updateAction(size_t index, const Action& action);

    // Get config file path
    [[nodiscard]] const std::string& getConfigPath() const { return configPath; }

    // Execute actions (start or end)
    void executeStartActions(const std::string& actionName) const;
    void executeEndActions(const std::string& actionName) const;

private:
    std::vector<Action> actions;
    std::string configPath;

    static void executeAction(const std::string& command);
};
