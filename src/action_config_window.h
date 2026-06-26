#pragma once

#include <QAction>
#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QTimer>
#include <memory>
#include "action_manager.h"

class DBusListener;
class QCloseEvent;
class QEvent;
class QMenu;

class ActionConfigWindow : public QMainWindow {
    Q_OBJECT

public:
    ActionConfigWindow(std::shared_ptr<ActionManager> actionManager, DBusListener* dbusListener, QWidget* parent = nullptr);
    ~ActionConfigWindow() override = default;

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void onAddAction();
    void onEditAction();
    void onDeleteAction();
    void onSave();
    void onLoad();
    void refreshActiveWindowInfo();
    void onDaemonActiveWindowChanged(const QString& windowClass, const QString& windowTitle, const QString& windowId);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void showFromTray();
    void hideToTray();
    void quitApplication();

    void setupUI();
    void setupTrayIcon();
    void loadActionList();
    std::string defaultConfigPath() const;

    std::shared_ptr<ActionManager> actionManager;
    DBusListener* dbusListener;
    QListWidget* actionListWidget;
    QLabel* activeWindowClassValueLabel;
    QLabel* activeWindowTitleValueLabel;
    QLabel* activeWindowIdValueLabel;
    QPushButton* refreshActiveWindowButton;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* deleteButton;
    QPushButton* saveButton;
    QPushButton* loadButton;
    QTimer* activeWindowRefreshTimer;
    QSystemTrayIcon* trayIcon;
    QMenu* trayMenu;
    QAction* showAction;
    QAction* hideAction;
    QAction* quitAction;
    bool trayHintShown;
    bool quitting;
};