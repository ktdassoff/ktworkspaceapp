#ifndef KTWORKSPACEAPP_HPP
#define KTWORKSPACEAPP_HPP

#include <ktworkspaceapp_export.h>

#include <QApplication>
#include <QString>
#include <QSettings>
#include <QIcon>
#include <QAction>
#include <QList>
#include <QStringList>
#include <QUuid>
#include <QPair>
#include <memory>

class KtWorksheet;
struct KtWorkspaceAppImpl;

class KTWORKSPACEAPP_EXPORT KtWorkspaceAboutHandler {
public:
    virtual void onAbout() = 0;
};

class KTWORKSPACEAPP_EXPORT KtWorkspacePrefsHandler {
public:
    virtual void onPrefs() = 0;
};

class KTWORKSPACEAPP_EXPORT KtWorkspaceWorksheetHandler {
public:
    virtual KtWorksheet * attach(const QUuid &new_id, const QString &class_name) = 0;
    virtual QString classDisplayName(const QString &class_name) = 0;
    virtual QIcon classIcon(const QString &class_name);
    // classIcon returns a null QIcon by default
};

class KTWORKSPACEAPP_EXPORT KtWorkspaceApp : public QApplication {
    Q_OBJECT
    Q_DISABLE_COPY(KtWorkspaceApp)
    KtWorkspaceAppImpl *self;
    friend class KtWorksheet;
    static KtWorkspaceApp *s_instance;

public:
    static inline KtWorkspaceApp *instance() { return s_instance; }

    explicit KtWorkspaceApp(int &argc, char **argv, bool single_instance, const QString &appName, const QString &appVer, const QString &orgName, const QString &orgDomain = QString(), const QIcon &appIcon = QIcon(),
        const QHash<QString, KtWorkspaceWorksheetHandler *> &worksheet_handlers = (QHash<QString, KtWorkspaceWorksheetHandler *>()), const QStringList &default_worksheet_class = QStringList());
    virtual ~KtWorkspaceApp();

    bool initApp();

    QIcon applicationIcon() const;
    void setApplicationIcon(const QIcon &val);

    virtual bool notify(QObject *receiver, QEvent *event);
    bool instanceAlreadyRunning() const;

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
    QString currentSession() const;

    bool selectSession(const QString &name = QString());
    QStringList allSessions() const;
    bool existsSession(const QString &name);
    bool renameSession(const QString &new_name, const QString &old_name);
    bool createSession(const QString &new_name, const QString &src_name = QString());
    bool deleteSession(const QString &name);

    // Settings and data
    std::auto_ptr<QSettings> getGlobalSettings();
    QString getGlobalDataDir() const;
    std::auto_ptr<QSettings> getSessionSettings(const QString &name = QString());
    QString getSessionDataDir(const QString &name = QString());

    // Global notifications
    enum NotificationKinds {
        N_Info,
        N_Warning,
        N_Critical
    };
    void showNotification(const QString &synopsis, NotificationKinds kind = N_Info, const QString &detail = QString()) const;
    QString userStatus() const;
    void setUserStatus(const QString &val = QString());

    // Global actions
    bool addGlobalAction(QAction *action);
    void removeGlobalAction(QAction *action);
    QSet<QAction *> globalActions() const;

    // Additional global elements
    bool registerAboutHandler(KtWorkspaceAboutHandler *handler);
    KtWorkspaceAboutHandler *getAboutHandler() const;
    void unregisterAboutHandler();
    bool registerPrefsHandler(KtWorkspacePrefsHandler *handler);
    KtWorkspacePrefsHandler *getPrefsHandler() const;
    void unregisterPrefsHandler();

    // Worksheet management
    quint16 worksheetCount() const;
    QList<KtWorksheet *> allWorksheets() const;
    bool registerWorksheetHandler(const QString & class_name, KtWorkspaceWorksheetHandler *handler);
    KtWorkspaceWorksheetHandler *getWorksheetHandler(const QString &class_name) const;
    void unregisterWorksheetHandler(const QString &class_name);
    void resetDefaultWorksheetClasses(const QStringList &defaults);
    KtWorksheet *attachWorksheet(const QString &class_name);
    bool detachWorksheet(KtWorksheet *worksheet);

    // Misc.
    static bool isValidSessionName(const QString &n);

public slots:
    void requestQuit();
    void sessionsDialog();

signals:
    // Session management and settings
    void sessionAboutToStart(const QString &name);
    void sessionStarted(const QString &name);
    void sessionAboutToEnd(const QString &name);
    void sessionEnded(const QString &name);

private slots:
    void onAbout();
    void onPrefs();

private:
    static void rm_r(const QString &parent, const QString &dirname);
    static void cp_r(const QString &parent, const QString &dirname, const QString &newparent, const QString &newdirname);
    typedef QPair<QUuid, QString> SavedWs;
    QList<SavedWs> allSavedWorksheets();
    void removeSavedWorksheet(const QUuid &id);
    void addSavedWorksheet(const QUuid &id, const QString &class_name);
    void worksheetClose(KtWorksheet *worksheet);
    KtWorksheet *attachWorksheetHelper(const QString &class_name, const QUuid &id, bool add_saved_worksheet);
    bool detachWorksheetHelper(KtWorksheet *worksheet, bool already_closed, bool remove_saved_worksheet);
};

#endif // !defined(KTWORKSPACEAPP_HPP)
