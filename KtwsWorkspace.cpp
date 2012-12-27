#include "KtWorkspaceApp.hpp"
#include "KtWorkspaceAppImpl.hpp"
#include "KtWorksheet.hpp"
#include "KtWorkspaceSessionDialog.hpp"

#include <QDesktopServices>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <stdexcept>

KtWorkspaceApp * KtWorkspaceApp::s_instance = 0;

QIcon KtWorkspaceWorksheetHandler::classIcon(const QString & class_name) { return QIcon(); }

KtWorkspaceApp::KtWorkspaceApp(int &argc, char **argv, bool single_instance, const QString &appName, const QString &appVer, const QString &orgName, const QString &orgDomain, const QIcon &appIcon,
                               const QHash<QString, KtWorkspaceWorksheetHandler *> &worksheet_handlers, const QStringList &default_worksheet_class)
    : QApplication(argc, argv), self(new KtWorkspaceAppImpl) {
        if(s_instance) {
            delete self;
            throw std::logic_error("Constructing a KtWorkspaceApp when an instance already exists!");
        } else s_instance = this;
        setApplicationName(appName);
        setApplicationVersion(appVer);
        setOrganizationName(orgName);
        if(!orgDomain.isEmpty()) setOrganizationDomain(orgDomain);
        setApplicationIcon(appIcon);

        // Compute application id and set up single-instance mechanism
        if(single_instance) {
            QString computed_app_id = appName;
            for(auto itr = computed_app_id.begin(); itr != computed_app_id.end(); itr++) {
                if(!itr->isLetterOrNumber()) *itr = QChar('_');
            }
    #ifdef Q_OS_WIN32
            if(!self->openMutexWin32(computed_app_id)) {
                self->already_running = true;
                return;
            }
    #endif
        }

        // Determine session root
        self->computed_datadir_path = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
        QDir dr(self->computed_datadir_path);
        if(!dr.exists()) dr.mkpath(".");

        // Initial handler registrations
        for(auto itr = worksheet_handlers.constBegin(); itr != worksheet_handlers.constEnd(); itr++) registerWorksheetHandler(itr.key(), itr.value());
        self->default_worksheet_class = default_worksheet_class;
}

bool KtWorkspaceApp::initApp() {
    if(self->already_running) return false;

    // Restore last session or ask for a session
    // If no sessions exist, create a new one called "Default" and load that
    if(allSessions().isEmpty()) {
        if(!createSession("Default")) { 
            QMessageBox::critical(0, applicationName(), "Unable to create an initial session!");
            return false;
        } else selectSession("Default");
    } else {
        auto gcfg = getGlobalSettings();
        QString lus = gcfg->value("kt_last_used_session").toString();
        if(!existsSession(lus)) {
            KtWorkspaceSessionDialog sdlg;
            int rc = sdlg.exec();
            if(rc == QDialog::Rejected || self->cur_session.isEmpty()) return false;
        } else selectSession(lus);
    }

    // Set up global menus/actions/shell elements (e.g. tray icon)
    self->tray_icon = new QSystemTrayIcon;
    if(self->user_status.isEmpty()) self->tray_icon->setToolTip(applicationName());
    else self->tray_icon->setToolTip(applicationName() + ": " + self->user_status);
    self->tray_icon->setIcon(self->app_icon);
    self->global_menu = new QMenu(applicationName());
    self->tray_icon->setContextMenu(self->global_menu);


    self->top_sep = self->global_menu->addSeparator();
    self->se_action = self->global_menu->addAction(tr("Manage sessions..."), this, SLOT(sessionsDialog()));
    self->global_menu->addSeparator();
    QAction * a = self->global_menu->addAction(tr("About %1...").arg(applicationName()), this, SLOT(onAbout()));
    a->setMenuRole(QAction::AboutRole);
    self->p_action = self->global_menu->addAction(tr("Preferences..."), this, SLOT(onPrefs()));
    self->p_action->setMenuRole(QAction::PreferencesRole);
    self->p_action->setVisible(false);
    a = self->global_menu->addAction(tr("&Quit"), this, SLOT(requestQuit()));
    a->setMenuRole(QAction::QuitRole);

    self->tray_icon->show();

    return true;
}

KtWorkspaceApp::~KtWorkspaceApp() {
    if(!self->cur_session.isEmpty() && !self->already_running) selectSession();
    delete self;
    s_instance = 0;
}

QIcon KtWorkspaceApp::applicationIcon() const { return self->app_icon; }
void KtWorkspaceApp::setApplicationIcon(const QIcon & val) {
    self->app_icon = val;
}

bool KtWorkspaceApp::notify(QObject *receiver, QEvent *event) {
    try {
        return QApplication::notify(receiver, event);
    } catch (const std::exception & e) {
        QMessageBox::critical(nullptr, applicationName(), QString("Fatal exception has arisen: %1;\nIn event: %2 to %3 \"%4\"")
                                .arg(e.what()).arg(event->type()).arg(receiver->metaObject()->className()).arg(receiver->objectName()));
        exit(-2);
        return false;
    }
}

bool KtWorkspaceApp::instanceAlreadyRunning() const { return self->already_running; }

// Session management and settings
KtWorkspaceApp::SessionStatus KtWorkspaceApp::sessionStatus() const { return self->session_status; }
QString KtWorkspaceApp::currentSession() const { return self->cur_session; }

bool KtWorkspaceApp::selectSession(const QString &name) {
    if(!name.isEmpty() && !existsSession(name)) return false;
    else {
        if(!self->cur_session.isEmpty()) {
            self->session_status = SessionEnding;
            emit sessionAboutToEnd(self->cur_session);
            while(!self->worksheets.isEmpty()) detachWorksheetHelper(*self->worksheets.begin(), false, false);
            QString olds = self->cur_session;
            self->cur_session.clear();
            self->session_status = SessionNone;
            emit sessionEnded(olds);
        }
        if(!name.isEmpty()) {
            if(!isValidSessionName(name)) return false;
            self->session_status = SessionStarting;
            emit sessionAboutToStart(name);
            self->cur_session = name;
            auto gcfg = getGlobalSettings();
            gcfg->setValue("kt_last_used_session", name);
            auto savws = allSavedWorksheets();
            if(savws.isEmpty()) {
                foreach(auto d, self->default_worksheet_class) attachWorksheetHelper(d, QUuid::createUuid(), true);
            } else foreach(const SavedWs & ws, savws) attachWorksheetHelper(ws.second, ws.first, false);
            self->session_status = SessionRunning;
            emit sessionStarted(name);
        }
        return true;
    }
}

QStringList KtWorkspaceApp::allSessions() const {
    QDir dr(self->computed_datadir_path);
    QFileInfoList entries = dr.entryInfoList(QStringList() << "session_*", QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QStringList sl;
    foreach(const QFileInfo & e, entries) {
        if(isValidSessionName(e.fileName())) sl << e.fileName().remove("session_");
    }
    return sl;
}

bool KtWorkspaceApp::existsSession(const QString &name) {
    if(!isValidSessionName(name)) return false;
    else {
        QDir dr(self->computed_datadir_path);
        return dr.exists(QString("session_") += name);
    }
}

bool KtWorkspaceApp::renameSession(const QString &new_name, const QString &old_name) {
    if(existsSession(new_name) || !existsSession(old_name) || old_name == currentSession()) return false;
    else {
        bool rc = createSession(new_name, old_name);
        if(rc) {
            deleteSession(old_name);
            return true;
        } else return false;
    }
}

bool KtWorkspaceApp::createSession(const QString &new_name, const QString &src_name) {
    if(existsSession(new_name) || !isValidSessionName(new_name) || (!src_name.isEmpty() && !existsSession(src_name))) return false;
    else {
        QDir dr(self->computed_datadir_path);
        if(!dr.mkdir(QString("session_") += new_name)) return false;
        else {
            if(!src_name.isEmpty()) cp_r(self->computed_datadir_path, QString("session_") += src_name, self->computed_datadir_path, QString("session_") += new_name);
            return true;
        }
    }
}

bool KtWorkspaceApp::deleteSession(const QString &name) {
    if(existsSession(name) && self->cur_session != name) {
        rm_r(self->computed_datadir_path, QString("session_") += name);
        return true;
    } else return false;
}

std::auto_ptr<QSettings> KtWorkspaceApp::getGlobalSettings() {
    auto qs = std::auto_ptr<QSettings>(new QSettings(QDir(getGlobalDataDir()).absoluteFilePath("ktws_settings.ini"), QSettings::IniFormat));
    return qs;
}
QString KtWorkspaceApp::getGlobalDataDir() const { 
    QDir dr(self->computed_datadir_path);
    if(!dr.exists("global")) dr.mkpath("global");
    return dr.absoluteFilePath("global");
}
std::auto_ptr<QSettings> KtWorkspaceApp::getSessionSettings(const QString & name) {
    if((!name.isEmpty() && !existsSession(name)) || self->cur_session.isEmpty()) return std::auto_ptr<QSettings>();
    else {
        auto qs = std::auto_ptr<QSettings>(new QSettings(QDir(getSessionDataDir()).absoluteFilePath("ktws_settings.ini"), QSettings::IniFormat));
        return qs;
    }
}
QString KtWorkspaceApp::getSessionDataDir(const QString &name) {
    if((!name.isEmpty() && !existsSession(name)) || self->cur_session.isEmpty()) return QString();
    else {
        QDir dr(self->computed_datadir_path);
        QString sn = QString("session_") += (name.isEmpty() ? self->cur_session : name);
        if(!dr.exists(sn)) dr.mkpath(sn);
        return dr.absoluteFilePath(sn);
    }
}

// Global notifications
void KtWorkspaceApp::showNotification(const QString &synopsis, NotificationKinds kind, const QString &detail) const {
    switch(kind) {
    case N_Info:
        self->tray_icon->showMessage(applicationName(),
            (detail.isEmpty() ? synopsis : QString("%1\n\n%2").arg(synopsis, detail)));
        break;
    case N_Warning:
        self->tray_icon->showMessage(applicationName(),
            (detail.isEmpty() ? synopsis : QString("%1\n\n%2").arg(synopsis, detail)),
            QSystemTrayIcon::Warning);
        break;
    case N_Critical:
        {
            QMessageBox mb;
            mb.setStandardButtons(QMessageBox::Ok);
            mb.setWindowTitle(applicationName());
            mb.setIcon(QMessageBox::Critical);
            mb.setText(synopsis);
            mb.setInformativeText(detail);
            mb.exec();
        } break;
    }
}

QString KtWorkspaceApp::userStatus() const { return self->user_status; }
void KtWorkspaceApp::setUserStatus(const QString & val) {
    self->user_status = val;
    if(self->tray_icon) {
        if(val.isEmpty()) self->tray_icon->setToolTip(applicationName());
        else self->tray_icon->setToolTip(applicationName() + ": " + self->user_status);
    }
}

// Global actions
bool KtWorkspaceApp::addGlobalAction(QAction *action) {
    if(self->installed_actions.contains(action)) return false;
    else {
        self->installed_actions.insert(action);
        self->global_menu->insertAction(self->top_sep, action);
        return true;
    }
}
void KtWorkspaceApp::removeGlobalAction(QAction *action) { self->installed_actions.remove(action); }
QSet<QAction *> KtWorkspaceApp::globalActions() const { return self->installed_actions; }

bool KtWorkspaceApp::registerAboutHandler(KtWorkspaceAboutHandler * handler) {
    if(self->about_handler) return false;
    else {
        self->about_handler = handler;
        return true;
    }
}

// Additional global elements
KtWorkspaceAboutHandler * KtWorkspaceApp::getAboutHandler() const { return self->about_handler; }
void KtWorkspaceApp::unregisterAboutHandler() { self->about_handler = 0; }
bool KtWorkspaceApp::registerPrefsHandler(KtWorkspacePrefsHandler * handler) {
    if(self->prefs_handler) return false;
    else {
        self->prefs_handler = handler;
        self->p_action->setVisible(true);
        return true;
    }
}
KtWorkspacePrefsHandler * KtWorkspaceApp::getPrefsHandler() const { return self->prefs_handler; }
void KtWorkspaceApp::unregisterPrefsHandler() {
    self->p_action->setVisible(false);
    self->prefs_handler = 0;
}

// Worksheet management
quint16 KtWorkspaceApp::worksheetCount() const {
    return self->worksheets.count();
}
QList<KtWorksheet *> KtWorkspaceApp::allWorksheets() const {
    QList<KtWorksheet *> wl;
    foreach(KtWorksheet * w, self->worksheets) wl.append(w);
    return wl;
}

bool KtWorkspaceApp::registerWorksheetHandler(const QString & class_name, KtWorkspaceWorksheetHandler * handler) {
    if(self->worksheet_handlers.contains(class_name)) return false;
    else {
        self->worksheet_handlers.insert(class_name, handler);
        return true;
    }
}
KtWorkspaceWorksheetHandler *KtWorkspaceApp::getWorksheetHandler(const QString &class_name) const { return self->worksheet_handlers.value(class_name, 0); }
void KtWorkspaceApp::unregisterWorksheetHandler(const QString & class_name) { self->worksheet_handlers.remove(class_name); }
void KtWorkspaceApp::resetDefaultWorksheetClasses(const QStringList & defaults) { self->default_worksheet_class = defaults; }

KtWorksheet * KtWorkspaceApp::attachWorksheet(const QString & class_name) {
    return attachWorksheetHelper(class_name, QUuid::createUuid(), true);
}
bool KtWorkspaceApp::detachWorksheet(KtWorksheet * worksheet) {
    return detachWorksheetHelper(worksheet, false, true);
}

// Slots
void KtWorkspaceApp::sessionsDialog() {
    KtWorkspaceSessionDialog dlg;
    dlg.exec();
}
void KtWorkspaceApp::onAbout() {
    if(self->about_handler) self->about_handler->onAbout();
    else QMessageBox::about(0, applicationName(), QString("%1 %2\n%3").arg(applicationName(), applicationVersion(), organizationName()));
}
void KtWorkspaceApp::onPrefs() {
    if(self->prefs_handler) self->prefs_handler->onPrefs();
}
void KtWorkspaceApp::requestQuit() {
    emit sessionAboutToEnd(self->cur_session);
    QString olds = self->cur_session;
    selectSession();
    emit sessionEnded(olds);
    exit(0);
}

// Misc.
bool KtWorkspaceApp::isValidSessionName(const QString &n) {
    if(n.isEmpty()) return false;
    foreach(QChar c, n) {
        if(!(c.isLetterOrNumber() || c == QChar('_') || c == QChar(' '))) return false;
    }
    return true;
}

void KtWorkspaceApp::rm_r(const QString &parent, const QString &dirname) {
    QDir p(parent);
    QDir root = p;
    if(p.exists(dirname)) {
        root.cd(dirname);
        QFileInfoList entries = root.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        foreach(QFileInfo e, entries) {
            if(e.isDir()) {
                rm_r(root.absolutePath(), e.fileName());
                root.rmdir(e.fileName());
            } else root.remove(e.fileName());
        }
        p.rmdir(dirname);
    }
}

void KtWorkspaceApp::cp_r(const QString &parent, const QString &dirname, const QString &newparent, const QString &newdirname) {
    QDir p(parent);
    QDir np(newparent);
    QDir root = p;
    np.mkdir(newdirname);
    QDir newroot = np;
    newroot.cd(newdirname);
    if(p.exists(dirname)) {
        root.cd(dirname);
        QFileInfoList entries = root.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        foreach(QFileInfo e, entries) {
            if(e.isDir()) {
                root.mkdir(e.fileName());
                cp_r(root.absolutePath(), e.fileName(), newroot.absolutePath(), e.fileName());
            } else QFile::copy(root.filePath(e.fileName()), newroot.filePath(e.fileName()));
        }
    }
}

QList<KtWorkspaceApp::SavedWs> KtWorkspaceApp::allSavedWorksheets() {
    auto scfg = getSessionSettings();
    QList<SavedWs> wsl;
    if(scfg->contains("ktws_saved")) {
        QStringList strls = scfg->value("ktws_saved").toStringList();
        foreach(const QString & s, strls) {
            QStringList spl = s.split(QChar('/'));
            if(spl.count() == 2) {
                QUuid u(spl[0]);
                if(!u.isNull()) wsl.append(SavedWs(u, spl[1]));
            }
        }
    }
    return wsl;
}

void KtWorkspaceApp::removeSavedWorksheet(const QUuid &id) {
    auto scfg = getSessionSettings();
    if(scfg->contains("ktws_saved")) {
        QStringList strls = scfg->value("ktws_saved").toStringList();
        for(auto itr = strls.begin(); itr != strls.end(); itr++) {
            if(itr->startsWith(id.toString())) {
                strls.erase(itr);
                scfg->setValue("ktws_saved", strls);
                break;
            }
        }
    }
}

void KtWorkspaceApp::addSavedWorksheet(const QUuid &id, const QString &class_name) {
    auto scfg = getSessionSettings();
    QStringList strls = scfg->value("ktws_saved").toStringList();
    strls.append(id.toString() + QChar('/') + class_name);
    scfg->setValue("ktws_saved", strls);
}

void KtWorkspaceApp::worksheetClose(KtWorksheet * worksheet) {
    detachWorksheetHelper(worksheet, true, true);
}

KtWorksheet * KtWorkspaceApp::attachWorksheetHelper(const QString &class_name, const QUuid &id, bool add_saved_worksheet) {
    if(!self->worksheet_handlers.contains(class_name)) {
        qWarning() << QString("Can't attach new worksheet: no handler registered for KtWorksheet class name of %1!").arg(class_name);
        return 0;
    } else {
        KtWorksheet * ws = self->worksheet_handlers.value(class_name)->attach(id, class_name);
        if(ws) {
            self->worksheets.insert(ws);
            if(add_saved_worksheet) addSavedWorksheet(ws->id(), class_name);
            ws->show();
        } else qWarning() << QString("Creation of %1 worksheet failed").arg(class_name);
        return ws;
    }
}

bool KtWorkspaceApp::detachWorksheetHelper(KtWorksheet *worksheet, bool already_closed, bool remove_saved_worksheet) {
    if(worksheet) {
        QUuid old_id = worksheet->id();
        if(!already_closed) worksheet->close();
        self->worksheets.remove(worksheet);

        // Clear worksheet settings
        if(remove_saved_worksheet) {
            auto scfg = getSessionSettings();
            scfg->remove(QString("ktws_") + old_id.toString());
            removeSavedWorksheet(old_id);
        }

        return true;
    } else {
        qWarning() << "Unable to detach worksheet that is null";
        return false;
    }
}

// OS-specific procedures
#ifdef Q_OS_WIN32
bool KtWorkspaceAppImpl::openMutexWin32(const QString &computed_app_id) {
    QString mutname("Local\\");
    mutname.append(computed_app_id);
    HANDLE hm = CreateMutex(NULL, TRUE, (const wchar_t *)mutname.utf16());
    if(hm == NULL) return false;
    else if(GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hm);
        return false;
    } else {
        instance_mutex = hm;
        return true;
    }
}
void KtWorkspaceAppImpl::closeMutexWin32() {
    if(instance_mutex != NULL) {
        CloseHandle(instance_mutex);
        instance_mutex = NULL;
    }
}
#endif
