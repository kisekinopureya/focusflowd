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

// NOLINTBEGIN(readability-redundant-access-specifiers)
public slots:
    void ExecuteAction(const QString& actionName, bool isStart) const;
    void ExecuteActionInt(const QString& actionName, int startFlag) const;
    bool ReloadConfig();
    void UpdateActiveWindow(const QString& windowClass, const QString& windowTitle, const QString& windowId);
    QString GetActiveWindowClass() const;
    QString GetActiveWindowTitle() const;
    QString GetActiveWindowId() const;

signals:
        void ActiveWindowChanged(const QString& windowClass, const QString& windowTitle, const QString& windowId);

private slots:
    void onFocusChanged(const QString& actionName);

private:
    std::shared_ptr<ActionManager> actionManager;
    bool running;
    QString activeWindowClass;
    QString activeWindowTitle;
    QString activeWindowId;
};
// NOLINTEND(readability-redundant-access-specifiers)