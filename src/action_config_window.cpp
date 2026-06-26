#include "action_config_window.h"
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDir>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <utility>
#include "dbus_listener.h"

ActionConfigWindow::ActionConfigWindow(std::shared_ptr<ActionManager> actionManager, DBusListener* dbusListener, QWidget* parent)
    : QMainWindow(parent),
      actionManager(std::move(actionManager)),
      dbusListener(dbusListener),
      activeWindowRefreshTimer(new QTimer(this)),
      trayIcon(nullptr),
      trayMenu(nullptr),
      showAction(nullptr),
      hideAction(nullptr),
      quitAction(nullptr),
      quitting(false) {
    setWindowTitle("FocusFlow");
    setGeometry(100, 100, 600, 500);
    setWindowIcon(QIcon(":/icons/focusflow.svg"));

    setupUI();
    setupTrayIcon();

    if (this->actionManager->getConfigPath().empty()) {
        if (!this->actionManager->loadActions(defaultConfigPath())) {
            QMessageBox::warning(this, "Warning", "Could not load existing configuration.\nStarting with empty configuration.");
        }
    }
    loadActionList();

    if (dbusListener != nullptr) {
        connect(dbusListener, &DBusListener::ActiveWindowChanged,
                this, &ActionConfigWindow::onDaemonActiveWindowChanged);
    }

    connect(activeWindowRefreshTimer, &QTimer::timeout,
            this, &ActionConfigWindow::refreshActiveWindowInfo);
    activeWindowRefreshTimer->start(1000);

    refreshActiveWindowInfo();
}

void ActionConfigWindow::setupUI() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    QLabel* titleLabel = new QLabel("Configure Actions", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    QLabel* listLabel = new QLabel("Actions:", this);
    mainLayout->addWidget(listLabel);

    actionListWidget = new QListWidget(this);
    mainLayout->addWidget(actionListWidget);

    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator);

    QLabel* activeWindowLabel = new QLabel("Active Window (Live):", this);
    QFont activeWindowFont = activeWindowLabel->font();
    activeWindowFont.setBold(true);
    activeWindowLabel->setFont(activeWindowFont);
    mainLayout->addWidget(activeWindowLabel);

    QGridLayout* activeWindowLayout = new QGridLayout();
    activeWindowLayout->addWidget(new QLabel("Class:", this), 0, 0);
    activeWindowClassValueLabel = new QLabel("-", this);
    activeWindowLayout->addWidget(activeWindowClassValueLabel, 0, 1);

    activeWindowLayout->addWidget(new QLabel("Title:", this), 1, 0);
    activeWindowTitleValueLabel = new QLabel("-", this);
    activeWindowTitleValueLabel->setWordWrap(true);
    activeWindowLayout->addWidget(activeWindowTitleValueLabel, 1, 1);

    activeWindowLayout->addWidget(new QLabel("Window ID:", this), 2, 0);
    activeWindowIdValueLabel = new QLabel("-", this);
    activeWindowLayout->addWidget(activeWindowIdValueLabel, 2, 1);
    mainLayout->addLayout(activeWindowLayout);

    QHBoxLayout* activeWindowButtonLayout = new QHBoxLayout();
    refreshActiveWindowButton = new QPushButton("Refresh Active Window", this);
    connect(refreshActiveWindowButton, &QPushButton::clicked,
            this, &ActionConfigWindow::refreshActiveWindowInfo);
    activeWindowButtonLayout->addWidget(refreshActiveWindowButton);
    activeWindowButtonLayout->addStretch();
    mainLayout->addLayout(activeWindowButtonLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("Add Action", this);
    editButton = new QPushButton("Edit Action", this);
    deleteButton = new QPushButton("Delete Action", this);

    connect(addButton, &QPushButton::clicked, this, &ActionConfigWindow::onAddAction);
    connect(editButton, &QPushButton::clicked, this, &ActionConfigWindow::onEditAction);
    connect(deleteButton, &QPushButton::clicked, this, &ActionConfigWindow::onDeleteAction);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    QHBoxLayout* fileLayout = new QHBoxLayout();
    saveButton = new QPushButton("Save", this);
    loadButton = new QPushButton("Load", this);

    connect(saveButton, &QPushButton::clicked, this, &ActionConfigWindow::onSave);
    connect(loadButton, &QPushButton::clicked, this, &ActionConfigWindow::onLoad);

    fileLayout->addWidget(saveButton);
    fileLayout->addWidget(loadButton);
    fileLayout->addStretch();
    mainLayout->addLayout(fileLayout);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void ActionConfigWindow::setupTrayIcon() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    trayIcon = new QSystemTrayIcon(QIcon(":/icons/focusflow.svg"), this);
    trayMenu = new QMenu(this);
    showAction = trayMenu->addAction("Show FocusFlow", this, &ActionConfigWindow::showFromTray);
    hideAction = trayMenu->addAction("Hide FocusFlow", this, &ActionConfigWindow::hideToTray);
    trayMenu->addSeparator();
    quitAction = trayMenu->addAction("Quit", this, &ActionConfigWindow::quitApplication);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->setToolTip("FocusFlow");
    connect(trayIcon, &QSystemTrayIcon::activated, this, &ActionConfigWindow::onTrayIconActivated);
    trayIcon->show();
}

void ActionConfigWindow::loadActionList() {
    actionListWidget->clear();

    const auto& actions = actionManager->getActions();
    for (const auto& action : actions) {
        QString text = QString::fromStdString(action.name);
        text += " [" + QString::number(action.startActions.size()) + " start, ";
        text += QString::number(action.endActions.size()) + " end]";
        actionListWidget->addItem(text);
    }
}

std::string ActionConfigWindow::defaultConfigPath() const {
    return (QDir::homePath() + "/.config/focusflow/actions.json").toStdString();
}

void ActionConfigWindow::refreshActiveWindowInfo() {
    if (dbusListener == nullptr) {
        activeWindowClassValueLabel->setText("(daemon unavailable)");
        activeWindowTitleValueLabel->setText("FocusFlow daemon is not running");
        activeWindowIdValueLabel->setText("-");
        return;
    }

    const QString windowClass = dbusListener->GetActiveWindowClass();
    const QString windowTitle = dbusListener->GetActiveWindowTitle();
    const QString windowId = dbusListener->GetActiveWindowId();

    activeWindowClassValueLabel->setText(windowClass.isEmpty() ? "(none)" : windowClass);
    activeWindowTitleValueLabel->setText(windowTitle.isEmpty() ? "(none)" : windowTitle);
    activeWindowIdValueLabel->setText(windowId.isEmpty() ? "(none)" : windowId);
}

void ActionConfigWindow::onDaemonActiveWindowChanged(const QString& windowClass, const QString& windowTitle, const QString& windowId) {
    activeWindowClassValueLabel->setText(windowClass.isEmpty() ? "(none)" : windowClass);
    activeWindowTitleValueLabel->setText(windowTitle.isEmpty() ? "(none)" : windowTitle);
    activeWindowIdValueLabel->setText(windowId.isEmpty() ? "(none)" : windowId);
}

void ActionConfigWindow::onAddAction() {
    QDialog dialog(this);
    dialog.setWindowTitle("Add New Action");
    dialog.setGeometry(200, 200, 400, 250);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* nameLabel = new QLabel("Action Name:", &dialog);
    QLineEdit* nameEdit = new QLineEdit(&dialog);

    layout->addWidget(nameLabel);
    layout->addWidget(nameEdit);
    layout->addWidget(new QLabel("Start and end commands can be added after creation.", &dialog));

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK", &dialog);
    QPushButton* cancelButton = new QPushButton("Cancel", &dialog);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    if (dialog.exec() == QDialog::Accepted && !nameEdit->text().trimmed().isEmpty()) {
        Action newAction;
        newAction.name = nameEdit->text().trimmed().toStdString();
        actionManager->addAction(newAction);
        loadActionList();
    }
}

void ActionConfigWindow::onEditAction() {
    const int currentRow = actionListWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "No Selection", "Please select an action to edit.");
        return;
    }

    const Action originalAction = actionManager->getActions().at(static_cast<size_t>(currentRow));
    Action updatedAction = originalAction;

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Action: " + QString::fromStdString(originalAction.name));
    dialog.setGeometry(200, 150, 500, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    QLabel* nameLabel = new QLabel("Action Name:", &dialog);
    QLineEdit* nameEdit = new QLineEdit(QString::fromStdString(originalAction.name), &dialog);
    layout->addWidget(nameLabel);
    layout->addWidget(nameEdit);

    QLabel* startLabel = new QLabel("Start Actions:", &dialog);
    QListWidget* startList = new QListWidget(&dialog);
    for (const auto& cmd : originalAction.startActions) {
        startList->addItem(QString::fromStdString(cmd));
    }
    layout->addWidget(startLabel);
    layout->addWidget(startList);

    QHBoxLayout* startButtonLayout = new QHBoxLayout();
    QPushButton* addStartButton = new QPushButton("Add Start Action", &dialog);
    QPushButton* removeStartButton = new QPushButton("Remove", &dialog);
    startButtonLayout->addWidget(addStartButton);
    startButtonLayout->addWidget(removeStartButton);
    layout->addLayout(startButtonLayout);

    QLabel* endLabel = new QLabel("End Actions:", &dialog);
    QListWidget* endList = new QListWidget(&dialog);
    for (const auto& cmd : originalAction.endActions) {
        endList->addItem(QString::fromStdString(cmd));
    }
    layout->addWidget(endLabel);
    layout->addWidget(endList);

    QHBoxLayout* endButtonLayout = new QHBoxLayout();
    QPushButton* addEndButton = new QPushButton("Add End Action", &dialog);
    QPushButton* removeEndButton = new QPushButton("Remove", &dialog);
    endButtonLayout->addWidget(addEndButton);
    endButtonLayout->addWidget(removeEndButton);
    layout->addLayout(endButtonLayout);

    auto syncList = [](QListWidget* listWidget) {
        std::vector<std::string> commands;
        commands.reserve(static_cast<size_t>(listWidget->count()));
        for (int index = 0; index < listWidget->count(); ++index) {
            commands.push_back(listWidget->item(index)->text().toStdString());
        }
        return commands;
    };

    connect(addStartButton, &QPushButton::clicked, &dialog, [startList]() {
        bool accepted = false;
        const QString text = QInputDialog::getText(startList, "Add Start Action", "Command:", QLineEdit::Normal, "", &accepted);
        if (accepted && !text.trimmed().isEmpty()) {
            startList->addItem(text.trimmed());
        }
    });
    connect(removeStartButton, &QPushButton::clicked, &dialog, [startList]() {
        delete startList->takeItem(startList->currentRow());
    });
    connect(addEndButton, &QPushButton::clicked, &dialog, [endList]() {
        bool accepted = false;
        const QString text = QInputDialog::getText(endList, "Add End Action", "Command:", QLineEdit::Normal, "", &accepted);
        if (accepted && !text.trimmed().isEmpty()) {
            endList->addItem(text.trimmed());
        }
    });
    connect(removeEndButton, &QPushButton::clicked, &dialog, [endList]() {
        delete endList->takeItem(endList->currentRow());
    });

    QHBoxLayout* okCancelLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK", &dialog);
    QPushButton* cancelButton = new QPushButton("Cancel", &dialog);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    okCancelLayout->addWidget(okButton);
    okCancelLayout->addWidget(cancelButton);
    layout->addLayout(okCancelLayout);

    if (dialog.exec() == QDialog::Accepted) {
        updatedAction.name = nameEdit->text().trimmed().toStdString();
        updatedAction.startActions = syncList(startList);
        updatedAction.endActions = syncList(endList);
        actionManager->updateAction(static_cast<size_t>(currentRow), updatedAction);
        loadActionList();
    }
}

void ActionConfigWindow::onDeleteAction() {
    const int currentRow = actionListWidget->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "No Selection", "Please select an action to delete.");
        return;
    }

    const QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Delete",
        "Are you sure you want to delete this action?");

    if (reply == QMessageBox::Yes) {
        actionManager->removeAction(static_cast<size_t>(currentRow));
        loadActionList();
    }
}

void ActionConfigWindow::onSave() {
    const std::string configPath = actionManager->getConfigPath().empty() ? defaultConfigPath() : actionManager->getConfigPath();
    if (actionManager->saveActions(configPath)) {
        QMessageBox::information(this, "Success",
            "Actions saved successfully.\n" + QString::fromStdString(configPath));
        return;
    }

    QMessageBox::critical(this, "Error", "Failed to save actions.");
}

void ActionConfigWindow::onLoad() {
    const std::string configPath = actionManager->getConfigPath().empty() ? defaultConfigPath() : actionManager->getConfigPath();
    if (actionManager->loadActions(configPath)) {
        loadActionList();
        QMessageBox::information(this, "Success",
            "Actions loaded successfully from:\n" + QString::fromStdString(configPath));
        return;
    }

    QMessageBox::critical(this, "Error",
        "Failed to load actions from:\n" + QString::fromStdString(configPath));
}

void ActionConfigWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (isVisible() && !isMinimized()) {
            hideToTray();
        } else {
            showFromTray();
        }
    }
}

void ActionConfigWindow::showFromTray() {
    showNormal();
    raise();
    activateWindow();
}

void ActionConfigWindow::hideToTray() {
    hide();
}

void ActionConfigWindow::quitApplication() {
    quitting = true;
    if (trayIcon != nullptr) {
        trayIcon->hide();
    }
    QApplication::quit();
}

void ActionConfigWindow::closeEvent(QCloseEvent* event) {
    if (!quitting && trayIcon != nullptr && trayIcon->isVisible()) {
        hideToTray();
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}

void ActionConfigWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange && isMinimized() && trayIcon != nullptr && trayIcon->isVisible()) {
        QTimer::singleShot(0, this, &ActionConfigWindow::hideToTray);
    }

    QMainWindow::changeEvent(event);
}
