#ifndef KTWSWORKSPACE_HPP_
#define KTWSWORKSPACE_HPP_

#include <KtwsGlobal.hpp>

#include <QList>
#include <QWeakPointer>
#include <QVariant>
struct QUuid;
class QIcon;
class QAction;

namespace Ktws {
class Session;
struct SessionInfo;
class WorksheetHandler;
class Worksheet;

class WorkspaceImpl;
class KTWORKSPACEAPP_EXPORT Workspace : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Workspace)

    WorkspaceImpl *d;
    friend class Worksheet;
    friend class Session;

    Q_ENUMS(SessionStatus DefaultAction)

public:
    explicit Workspace(QObject *parent = nullptr);
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

    int sessionCount() const;
    QList<Session *> sessions() const;
    Session *currentSession() const;
    Session *lastUsedSession() const;
    Session *sessionById(const QUuid &id);
    Session *sessionByName(const QString &name);
    Session *createSession(const QString &new_name);

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
        SessionsDefaultAction
    };
    QAction *defaultAction(DefaultAction da);

    // Worksheet management
    int worksheetCount() const;
    QList<Worksheet *> worksheets() const;
    Worksheet *worksheetById(const QUuid &id);
    Worksheet *createWorksheet(const QString &class_name);

    int worksheetHandlerCount() const;
    QList<QString> worksheetHandlerClassNames() const;
    bool registerWorksheetHandler(const QString &class_name, WorksheetHandler *handler);
    WorksheetHandler *getWorksheetHandler(const QString &class_name) const;
    void unregisterWorksheetHandler(const QString &class_name);

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
	QUuid currentSessionId() const;
    QUuid lastUsedSessionId() const;
    bool existsSession(const QUuid &id);
    bool selectSession(const QUuid &id);
    QUuid mkSession(const QString &new_name, const QUuid &clone_src);
    bool rmSession(const QUuid &id);
    bool renameSession(const QString &new_name, const QUuid &id);

    // Data
    QString baseDataDir() const;
    QString curSessionDataDir() const;
    QVariantHash &curSessionSettings();
    void replaceCurSessionSettings(const QVariantHash &settings);

    bool deserializeGlobalSettings();
    bool serializeGlobalSettings();
    bool deserializeSessionSettings(const QUuid &session_id, QVariantHash &target);
    bool serializeSessionSettings(const QUuid &session_id, const QVariantHash &source);
    void clearSessionSettings(const QUuid &session_id);
    bool deserializeWorksheetSettings(const QUuid &session_id, const QUuid &worksheet_id, QVariantHash &target);
    bool serializeWorksheetSettings(const QUuid &session_id, const QUuid &worksheet_id, const QVariantHash &source);
    void clearWorksheetSettings(const QUuid &session_id, const QUuid &worksheet_id);

    // Worksheets/helpers
    Worksheet *attachWorksheetHelper(const QString &class_name, const QUuid &id);
    void detachWorksheetHelper(const QUuid &id, bool rm_saved);
    void handleWorksheetClose(const QUuid &id);
};
} // namespace Ktws

#endif // KTWSWORKSPACE_HPP_
