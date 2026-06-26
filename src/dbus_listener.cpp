#include "dbus_listener.h"
#include <QDBusConnection>
#include <QDBusError>
#include <iostream>
#include <utility>
#include "action_manager.h"
#include "qobjectdefs.h"
#include "qstring.h"


DBusListener::DBusListener(std::shared_ptr<ActionManager> actionManager)
    : actionManager(std::move(actionManager)), running(false) {}

DBusListener::~DBusListener() {
    stop();
}

bool DBusListener::initialize(const std::string& configPath) {
    if (QDBusConnection bus = QDBusConnection::sessionBus(); !bus.isConnected()) {
        std::cerr << "Failed to connect to session D-Bus" << '\n';
        return false;
    }

    // Load actions from config
    if (!actionManager->loadActions(configPath)) {
        std::cerr << "Failed to load action configuration" << '\n';
        return false;
    }

    std::cout << "D-Bus listener initialized successfully" << '\n';
    return true;
}

bool DBusListener::start() {
    QDBusConnection bus = QDBusConnection::sessionBus();

    if (!bus.registerService("com.kisekinopureya.FocusFlow")) {
        std::cerr << "Failed to own D-Bus name: "
                  << bus.lastError().message().toStdString() << '\n';
        return false;
    }

    if (!bus.registerObject(
            "/com/kisekinopureya/FocusFlow",
            this,
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        std::cerr << "Failed to register D-Bus object: "
                  << bus.lastError().message().toStdString() << '\n';
        bus.unregisterService("com.kisekinopureya.FocusFlow");
        return false;
    }

    bus.connect(
        "",
        "/org/kde/KWin/Effects/WindowFocus",
        "org.kde.KWin.Effects.WindowFocus",
        "WindowFocusChanged",
        this,
        SLOT(onFocusChanged(QString)));

    running = true;

    std::cout << "D-Bus service started" << '\n';
    return true;
}

void DBusListener::stop() {
    if (!running) {
        return;
    }

    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject("/com/kisekinopureya/FocusFlow");
    bus.unregisterService("com.kisekinopureya.FocusFlow");
    running = false;
}

void DBusListener::onFocusChanged(const QString& actionName) {
    if (actionName.isEmpty()) {
        return;
    }

    std::cout << "Focus changed to action: " << actionName.toStdString() << '\n';
    actionManager->executeStartActions(actionName.toStdString());
}

void DBusListener::ExecuteAction(const QString& actionName, bool isStart) const {
    if (actionName.isEmpty()) {
        return;
    }

    std::cout << "ExecuteAction method called: " << actionName.toStdString()
              << " (isStart=" << (isStart ? "true" : "false") << ")" << '\n';

    if (isStart) {
        actionManager->executeStartActions(actionName.toStdString());
    } else {
        actionManager->executeEndActions(actionName.toStdString());
    }
}

bool DBusListener::ReloadConfig() {
    std::cout << "ReloadConfig method called" << '\n';
    return actionManager->reloadActions();
}

void DBusListener::UpdateActiveWindow(const QString& windowClass, const QString& windowTitle, const QString& windowId) {
    if (windowClass == activeWindowClass &&
        windowTitle == activeWindowTitle &&
        windowId == activeWindowId) {
        return;
    }

    activeWindowClass = windowClass;
    activeWindowTitle = windowTitle;
    activeWindowId = windowId;

    emit ActiveWindowChanged(activeWindowClass, activeWindowTitle, activeWindowId);
}

QString DBusListener::GetActiveWindowClass() const {
    return activeWindowClass;
}

QString DBusListener::GetActiveWindowTitle() const {
    return activeWindowTitle;
}

QString DBusListener::GetActiveWindowId() const {
    return activeWindowId;
}
