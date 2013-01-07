#ifndef KTWSWORKSPACE_HPP_
#define KTWSWORKSPACE_HPP_

#include <KtwsGlobal.hpp>

#include <QObject>
#include <QList>
struct QUuid;
class QIcon;
class QAction;
class QSettings;

namespace Ktws {
class Session;
class WorksheetHandler;
class Worksheet;

struct WorkspaceImpl;
class KTWORKSPACEAPP_EXPORT Workspace : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Workspace)

    WorkspaceImpl *d;
    friend class Worksheet;
    friend class Session;

    Q_ENUMS(SessionStatus DefaultAction)

public:
    explicit Workspace(const QString &app_id, QObject *parent = nullptr);
    virtual ~Workspace();
    QString appId() const;

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

    int sessionCount() const;
    QList<Session *> sessions() const;
    Session *currentSession() const;
    Session *lastUsedSession() const;
    Session *sessionById(const QUuid &id) const;
    Session *sessionByName(const QString &name) const;
    Session *createSession(const QString &new_name);

    // Settings and data
    QSettings *globalSettings();
    QString globalDataDir() const;

    // Global actions
    QList<QAction *> globalActions() const;
    void addGlobalAction(QAction *action);
    void addGlobalActions(const QList<QAction *> &actions);
    void insertGlobalAction(QAction *before, QAction *action);
    void insertGlobalActions(QAction *before, const QList<QAction *> &actions);
    void removeGlobalAction(QAction *action);
    void clearGlobalActions();
    enum DefaultAction {
        AboutDefaultAction,
        QuitDefaultAction,
        SessionsDefaultAction
    };
    QAction *defaultAction(DefaultAction da);

    // Worksheet management
    int worksheetCount() const;
    QList<Worksheet *> worksheets() const;
    Worksheet *worksheetById(const QUuid &id) const;
    Worksheet *createWorksheet(const QString &class_name);

    int worksheetHandlerCount() const;
    QList<QString> worksheetHandlerClassNames() const;
    bool registerWorksheetHandler(const QString &class_name, WorksheetHandler *handler);
    WorksheetHandler *worksheetHandler(const QString &class_name) const;
    void unregisterWorksheetHandler(const QString &class_name);
    QString defaultWorksheetClass() const;
    void setDefaultWorksheetClass(const QString &class_name);

    // Convenience function for loading the last used session or, if no session exists
    //     either creating a default session and loading that or showing the sessions dialog
    // Returns true if a session was started, false if no session was started
    //     (e.g. the user canceled the sessions dialog);
    //     immediately returns true without doing anything if a session is already active
    // Parameter default specifies the name of the default session to create if needed;
    //     if empty, the session dialog will be shown instead of creating a default session
    bool startWorkspace(const QString &default_name = QString("Default"));

public slots:
    void requestQuit();
    void sessionsDialog();
    void aboutDialog();

signals:
    // Session management and settings
    void sessionAboutToStart(Session *session);
    void sessionStarted(Session *session);
    void sessionAboutToEnd(Session *session);
    void sessionEnded(Session *session);

    // Global actions
    void globalActionsModified(const QList<QAction *> actions);

private:
	// Sessions
    bool selectSession(const QUuid &id);
    Session *cpSession(const QString &new_name, const QUuid &clone_src);
    void rmSession(const QUuid &id);

    // Worksheets/helpers
    Worksheet *attachWorksheetHelper(const QString &class_name, const QUuid &id);
    void handleWorksheetClose(const QUuid &id);
};
} // namespace Ktws

#endif // KTWSWORKSPACE_HPP_
