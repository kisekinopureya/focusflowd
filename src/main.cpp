#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <iostream>
#include <memory>
#include <string_view>
#include "action_config_window.h"
#include "action_manager.h"
#include "dbus_listener.h"

constexpr std::string_view version{"v0.1.0"};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setApplicationName("FocusFlow");
    QApplication::setApplicationDisplayName("FocusFlow");
    QApplication::setWindowIcon(QIcon(":/icons/focusflow.svg"));

    std::cout << "Focus Flow daemon " << version << " starting..." << '\n';

    // Determine config file path
    const QString configPathQt = QDir::homePath() + "/.config/focusflow/actions.json";
    const std::string configPath = configPathQt.toStdString();

    // Create action manager
    const auto actionManager = std::make_shared<ActionManager>();

    // Create and initialize DBus listener
    DBusListener listener(actionManager);
    if (!listener.initialize(configPath)) {
        QMessageBox::critical(nullptr, "FocusFlow", "Failed to initialize the FocusFlow daemon.");
        std::cerr << "Failed to initialize DBus listener" << '\n';
        return 1;
    }

    if (!listener.start()) {
        QMessageBox::critical(nullptr, "FocusFlow", "Failed to start the FocusFlow D-Bus service.");
        std::cerr << "Failed to start DBus listener" << '\n';
        return 1;
    }

    ActionConfigWindow window(actionManager, &listener);
    window.show();

    std::cout << "Focus Flow daemon running..." << '\n';
    return QApplication::exec();
}
