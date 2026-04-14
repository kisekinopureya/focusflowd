#pragma once

#include "action_manager.h"
#include <memory>
#include <string>
#include <QObject>
#include <QString>

class DBusListener : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kisekinopureya.FocusFlow")

public:
    DBusListener(std::shared_ptr<ActionManager> actionManager);
    ~DBusListener();

    bool initialize(const std::string& configPath);
    bool start();
    void stop();

public slots:
    void ExecuteAction(const QString& actionName, bool isStart);
    void ExecuteActionInt(const QString& actionName, int isStart);
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
