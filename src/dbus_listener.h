#pragma once

#include "action_manager.h"
#include <memory>
#include <string>
#include <QObject>
#include <QString>

class DBusListener final : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kisekinopureya.FocusFlow")

public:
    explicit DBusListener(std::shared_ptr<ActionManager> actionManager);
    ~DBusListener() override;

    [[nodiscard]] bool initialize(const std::string& configPath) const;
    bool start();
    void stop();

    void ExecuteAction(const QString& actionName, bool isStart) const;
    void ExecuteActionInt(const QString& actionName, int isStart) const;
    [[nodiscard]] bool ReloadConfig() const;
    void UpdateActiveWindow(const QString& windowClass, const QString& windowTitle, const QString& windowId);
    [[nodiscard]] QString GetActiveWindowClass() const;
    [[nodiscard]] QString GetActiveWindowTitle() const;
    [[nodiscard]] QString GetActiveWindowId() const;

signals:
    void ActiveWindowChanged(const QString& windowClass, const QString& windowTitle, const QString& windowId);

private:
    void onFocusChanged(const QString& actionName) const;

    std::shared_ptr<ActionManager> actionManager;
    bool running;
    QString activeWindowClass;
    QString activeWindowTitle;
    QString activeWindowId;
};
