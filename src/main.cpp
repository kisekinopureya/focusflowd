#include "dbus_listener.h"
#include "action_manager.h"
#include <iostream>
#include <memory>
#include <QCoreApplication>
#include <QDir>

const std::string version = "v0.1.0";

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "Focus Flow daemon " << version << " starting..." << std::endl;

    // Determine config file path
    const QString configPathQt = QDir::homePath() + "/.config/focusflow/actions.json";
    const std::string configPath = configPathQt.toStdString();

    // Create action manager
    auto actionManager = std::make_shared<ActionManager>();

    // Create and initialize DBus listener
    DBusListener listener(actionManager);
    if (!listener.initialize(configPath)) {
        std::cerr << "Failed to initialize DBus listener" << std::endl;
        return 1;
    }

    if (!listener.start()) {
        std::cerr << "Failed to start DBus listener" << std::endl;
        return 1;
    }

    std::cout << "Focus Flow daemon running..." << std::endl;
    return app.exec();
}
