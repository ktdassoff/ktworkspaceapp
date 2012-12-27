#ifndef KTWSWORKSPACE_HPP_
#define KTWSWORKSPACE_HPP_

#include <KtwsGlobal.hpp>

#include <QWorkspace>
#include <QList>
#include <QWeakPointer>
#include <QSharedPointer>
#include <QVariant>
class QUuid;
class QIcon;
class QAction;

namespace Ktws {
class Session;
typedef QSharedPointer<Session> SessionRef;
class WorksheetHandler;
class Worksheet;
typedef QWeakPointer<Worksheet> WorksheetRef;

class WorkspaceImpl
class KTWORKSPACEAPP_EXPORT Workspace : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Workspace)

    WorkspaceImpl *d;
    friend class WorkspaceImpl;

    Q_ENUMS(SessionStatus DefaultAction)

public:
    explicit Workspace(const QString &app_name);
    virtual ~Workspace();

    QString appName();

    // Session management
    enum SessionStatus {
        SessionNone,
        SessionStarting,
        SessionEnding,
        SessionRunning
    };
    SessionStatus sessionStatus() const;
    bool isSessionStarting() const { return sessionStatus() == SessionStarting; }
    bool isSessionEnding() const { return sessionStatus() == SessionEnding; }
    bool isSessionTransition() const { return sessionStatus() == SessionStarting || sessionStatus() == SessionEnding; }
    bool isSessionRunning() const { return sessionStatus() == SessionRunning; }
    SessionRef currentSession() const;

    SessionRef sessionById(const QUuid &id);
    SessionRef sessionByName(const QString &name);
    SessionRef createSession(const QString &new_name);

    // Settings and data
    QVariantHash &globalSettings();
    const QVariantHash &globalSettings() const;
    void replaceGlobalSettings(const QVariantHash &settings);
    QString globalDataDir() const;

    // Global notifications and status
    QString userStatus() const;
    void setUserStatus(const QString &status);
    void clearNotifications();

    // Global actions
    QList<QAction *> globalActions() const;
    void addGlobalAction(QAction *action);
    void addGlobalActions(QList<QAction *> actions);
    void insertGlobalAction(QAction *before, QAction *action);
    void insertGlobalActions(QAction *before, QList<QAction *> actions);
    void removeGlobalAction(QAction *action);
    void clearGlobalActions();
    enum DefaultAction {
        AboutDefaultAction,
        QuitDefaultAction,
        SessionsDefaultAction,
        WorksheetsDefaultAction
    };
    QAction *defaultAction(DefaultAction da);

    // Worksheet management
    int worksheetCount() const;
    QList<WorksheetRef> worksheets() const;
    bool registerWorksheetHandler(const QString &class_name, WorksheetHandler *handler);
    WorksheetHandler *getWorksheetHandler(const QString &class_name) const;
    void unregisterWorksheetHandler(const QString &class_name);
    WorksheetRef attachWorksheet(const QString &class_name);
    void detachWorksheet(WorksheetRef worksheet);

public slots:
    void requestQuit();
    void sessionsDialog();
    void worksheetsDialog();

signals:
    // Session management and settings
    void sessionAboutToStart(Session *session);
    void sessionStarted(Session *session);
    void sessionAboutToEnd(Session *session);
    void sessionEnded(Session *session);

    // Global actions
    void globalActionsModified(const QList<QAction *> actions);
};
} // namespace Ktws

#endif // KTWSWORKSPACE_HPP_
