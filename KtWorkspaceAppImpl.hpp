#ifndef KTWORKSPACEAPPIMPL_HPP
#define KTWORKSPACEAPPIMPL_HPP

#include <QSet>
#include <QHash>
#include <QString>
#include <QAction>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QMenu>

#ifdef Q_OS_WIN32
#define UNICODE
#define WIN32_LEAN_AND_MEAN
#define STRICT
#define WINVER 0x0502
#include <Windows.h>
#endif

class KtWorksheet;
struct KtWorkspaceAppImpl {
    QString computed_datadir_path;
    QIcon app_icon;
    bool already_running;
    KtWorkspaceApp::SessionStatus session_status;
    QString cur_session, user_status;

#ifdef Q_OS_WIN32
    HANDLE instance_mutex;
    bool openMutexWin32(const QString &computed_app_id);
    void closeMutexWin32();
#endif
    
    QSystemTrayIcon *tray_icon;
    QSet<QAction *> installed_actions;
    QMenu *global_menu;
    KtWorkspaceAboutHandler *about_handler;
    KtWorkspacePrefsHandler *prefs_handler;
    QAction *ws_action, *se_action, *p_action, *top_sep;

    QSet<KtWorksheet *> worksheets;
    QHash<QString, KtWorkspaceWorksheetHandler *> worksheet_handlers;
    QStringList default_worksheet_class;

    KtWorkspaceAppImpl()
        : already_running(false), session_status(KtWorkspaceApp::SessionNone),
#ifdef Q_OS_WIN32
        instance_mutex(NULL),
#endif
            tray_icon(0), global_menu(0), about_handler(0), prefs_handler(0), ws_action(0), se_action(0), top_sep(0) {}
    ~KtWorkspaceAppImpl() {
        if(tray_icon) tray_icon->deleteLater();
        if(global_menu) global_menu->deleteLater();
    }
};

#endif // !defined(KTWORKSPACEAPPIMPL_HPP)
