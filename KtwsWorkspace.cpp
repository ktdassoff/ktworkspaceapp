#include "KtwsWorkspace.hpp"
#include "KtwsWorkspace_p.hpp"
#include "KtwsSession.hpp"
#include "KtwsSessionDialog.hpp"
#include "KtwsSerialization_p.hpp"
#include "KtwsWorksheetHandler.hpp"
#include "KtwsWorksheet.hpp"

#include <QDesktopServices>
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>

namespace Ktws {
const char *S_GLOBAL_DIR_CMPNT = "global";

Workspace::Workspace(const QString &app_id, QObject *parent)
    : QObject(parent), d(new WorkspaceImpl(app_id))
{
    qDebug() << "Creating Ktws::Workspace instance for app_id:" << app_id;
    // Default actions
    d->m_da_about = new QAction(tr("About %1").arg(QCoreApplication::applicationName()), this);
    d->m_da_about->setMenuRole(QAction::AboutRole);
    connect(d->m_da_about, SIGNAL(triggered(bool)), this, SLOT(aboutDialog()));
    d->m_da_quit = new QAction(tr("&Quit"), this);
    d->m_da_quit->setMenuRole(QAction::QuitRole);
    d->m_da_quit->setShortcutContext(Qt::ApplicationShortcut);
    d->m_da_quit->setShortcut(QKeySequence::Quit);
    connect(d->m_da_quit, SIGNAL(triggered(bool)), this, SLOT(requestQuit()));
    d->m_da_sessions = new QAction(tr("&Sessions..."), this);
    d->m_da_sessions->setMenuRole(QAction::ApplicationSpecificRole);
    d->m_da_sessions->setShortcutContext(Qt::ApplicationShortcut);
    d->m_da_sessions->setShortcut(QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_Semicolon));
    connect(d->m_da_sessions, SIGNAL(triggered(bool)), this, SLOT(sessionsDialog()));

    // Add default actions to global actions
    d->m_global_actions.append(d->m_da_sessions);
    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    d->m_global_actions.append(sep);
    d->m_global_actions.append(d->m_da_about);
    d->m_global_actions.append(d->m_da_quit);

    // Initialize session table
    QList<SessionMd> scanned = scanSessions(d->m_app_id);
    foreach(const SessionMd &smd, scanned) {
        qDebug() << ">>> Found a stored session:" << smd.id.toString() << " name:" << smd.name << " timestamp:" << smd.timestamp.toString();
        Session *ns = new Session(smd.id, this, this);
        ns->setName(smd.name);
        ns->setLastUsedTimestamp(smd.timestamp);
        d->m_session_table.insert(smd.id, ns);
    }
}
Workspace::~Workspace() { delete d; }
QString Workspace::appId() const { return d->m_app_id; }

//// Public methods ////
// Session management
Workspace::SessionStatus Workspace::sessionStatus() const { return d->m_session_status; }

int Workspace::sessionCount() const { return d->m_session_table.count(); }
QList<Session *> Workspace::sessions() const { return d->m_session_table.values(); }
Session *Workspace::currentSession() const { return d->m_session_table.value(d->m_cur_session_id, nullptr); }
Session *Workspace::lastUsedSession() const {
    qDebug() << "Ktws::Workspace searching for last used session for app_id: " << d->m_app_id;
    QDateTime mdt;
    Session *ret = nullptr;

    foreach(Session *chks, d->m_session_table) {
        if(chks->lastUsedTimestamp().isValid() && (chks->lastUsedTimestamp() > mdt || mdt.isNull())) {
            qDebug() << ">>> Most recent timestamp so far is " << chks->lastUsedTimestamp().toString() << " id:" << chks->id().toString();
            mdt = chks->lastUsedTimestamp();
            ret = chks;
        }
    }
    return ret;
}

Session *Workspace::sessionById(const QUuid &id) const {
    return d->m_session_table.value(id, nullptr);
}
Session *Workspace::sessionByName(const QString &name) const {
    foreach(Session *s, d->m_session_table) {
        if(s->name() == name) return s;
    }
    return nullptr;
}

Session *Workspace::createSession(const QString &new_name) {
    QUuid nid = QUuid::createUuid();
    Session *ns = new Session(nid, this, this);
    ns->setName(new_name);
    d->m_session_table.insert(nid, ns);
    SessionMd nmd = { nid, new_name, QDateTime() };
    writeSessionMetadata(d->m_app_id, nmd);
    return ns;
}

// Settings and data
QSettings *Workspace::globalSettings() {
    if(!d->m_settings) {
        d->m_settings = openGlobalSettings(d->m_app_id);
        d->m_settings->setParent(this);
    }
    return d->m_settings;
}
QString Workspace::globalDataDir() const { return getGlobalDataDir(d->m_app_id); }

// Global actions
QList<QAction *> Workspace::globalActions() const { return d->m_global_actions; }
void Workspace::addGlobalAction(QAction *action) {
    d->m_global_actions.append(action);
    emit globalActionsModified(d->m_global_actions);
}
void Workspace::addGlobalActions(const QList<QAction *> &actions) {
    d->m_global_actions.append(actions);
    emit globalActionsModified(d->m_global_actions);
}
void Workspace::insertGlobalAction(QAction *before, QAction *action) {
    if(before == nullptr) d->m_global_actions.insert(0, action);
    else {
        int idx = d->m_global_actions.indexOf(before);
        if(idx < 0) d->m_global_actions.insert(0, action);
        else d->m_global_actions.insert(idx, action);
    }
    emit globalActionsModified(d->m_global_actions);
}
void Workspace::insertGlobalActions(QAction *before, const QList<QAction *> &actions) {
    if(before == nullptr) d->m_global_actions = actions + d->m_global_actions;
    else {
        int idx = d->m_global_actions.indexOf(before);
        if(idx < 0) d->m_global_actions = actions + d->m_global_actions;
        else d->m_global_actions = d->m_global_actions.mid(0, idx) + actions + d->m_global_actions.mid(idx);
    }
    emit globalActionsModified(d->m_global_actions);
}
void Workspace::removeGlobalAction(QAction *action) {
    if(d->m_global_actions.removeOne(action)) emit globalActionsModified(d->m_global_actions);
}
void Workspace::clearGlobalActions() { d->m_global_actions.clear(); }
QAction *Workspace::defaultAction(DefaultAction da) {
    switch(da) {
    case AboutDefaultAction: return d->m_da_about;
    case QuitDefaultAction: return d->m_da_quit;
    case SessionsDefaultAction: return d->m_da_sessions;
    default: return nullptr;
    }
}

// Worksheet management
int Workspace::worksheetCount() const { return d->m_worksheet_table.count(); }
QList<Worksheet *> Workspace::worksheets() const { return d->m_worksheet_table.values(); }
Worksheet *Workspace::worksheetById(const QUuid &id) const { return d->m_worksheet_table.value(id, nullptr); }
Worksheet *Workspace::createWorksheet(const QString &class_name) {
    Worksheet *ws = attachWorksheetHelper(class_name, QUuid::createUuid());
    if(ws) {
        qDebug() << "Ktws::Workspace::createWorksheet: new worksheet of class" << class_name << " with id" << ws->id();
        WorksheetMd wmd = { ws->id(), ws->className() };
        writeWorksheetMetadata(d->m_app_id, d->m_cur_session_id, wmd);
    } else qWarning() << QString("Unable to create new worksheet for class %1!").arg(class_name);
    return ws;
}

int Workspace::worksheetHandlerCount() const { return d->m_worksheet_handler_table.count(); }
QList<QString> Workspace::worksheetHandlerClassNames() const { return d->m_worksheet_handler_table.keys(); }
bool Workspace::registerWorksheetHandler(const QString &class_name, WorksheetHandler *handler) {
    if(!d->m_worksheet_handler_table.contains(class_name)) {
        d->m_worksheet_handler_table.insert(class_name, handler);
        return true;
    } else return false;
}
WorksheetHandler *Workspace::worksheetHandler(const QString &class_name) const { return d->m_worksheet_handler_table.value(class_name, nullptr); }
void Workspace::unregisterWorksheetHandler(const QString &class_name) { d->m_worksheet_handler_table.remove(class_name); }
QString Workspace::defaultWorksheetClass() const { return d->m_default_worksheet_class; }
void Workspace::setDefaultWorksheetClass(const QString &class_name) {
    if(!d->m_worksheet_handler_table.contains(class_name)) {
        qWarning() << QString("Ktws::Workspace::setDefaultWorksheetClass: "
            "Set \"%1\" as default, but this handler class isn't registered yet!").arg(class_name);
    }
    d->m_default_worksheet_class = class_name;
}

// Convenience function
bool Workspace::startWorkspace(const QString &default_name) {
    qDebug() << "Starting a session for app_id:" << d->m_app_id;
    // Is a session already running?
    if(d->m_session_status != SessionNone) return true;

    // Find the last used session, if any
    // lastUsedSession will not return a session if sessions exists but none has a timestamp
    Session *lu = lastUsedSession();
    if(lu) {
        qDebug() << ">>> Found the last used session @" << lu->lastUsedTimestamp().toString() << " id:" << lu->id().toString() << " name:" << lu->name();
        return lu->switchTo();
    } else if(sessionCount() == 1) {
        // If there's only one session, switch to that
        qDebug() << ">>> No last used session; switching to the sole saved session";
        return sessions().first()->switchTo();
    } else if(sessionCount() > 1 || default_name.isEmpty()) {
        // If there's more than one session, or no default, show the dialog
        qDebug() << ">>> No last used session; showing dialog";
        sessionsDialog();
        // After returning, check if a session was selected
        return isSessionRunning();
    } else {
        // No sessions exist, use the default_name
        qDebug() << ">>> No last used session; creating default session with name:" << default_name;
        Session *ns = createSession(default_name);
        if(ns) return ns->switchTo();
        else return false; /* bad name? */
    }
}

//// Public slots ////
void Workspace::endSession() {
    selectSession(QUuid());
}
void Workspace::sessionsDialog() {
    SessionDialog *dlg = new SessionDialog(this, QApplication::activeWindow());
    dlg->exec();
    dlg->deleteLater();
}
void Workspace::aboutDialog() {
    QMessageBox::about(QApplication::activeWindow(), QApplication::applicationName(), 
        QString("<b>%1 %2</b><br/>%3").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion(), QCoreApplication::organizationName()));
}

//// Private methods ////
// Sessions
bool Workspace::selectSession(const QUuid &id) {
    qDebug() << "Ktws::Workspace: Selecting session" << id.toString();
    if(!id.isNull() && !d->m_session_table.contains(id)) {
        qWarning() << "Ktws::Workspace::selectSession: given id" << id.toString() << "is not in session table!";
        return false;
    }

    if(!d->m_cur_session_id.isNull()) {
        // End existing session
        Session *os = d->m_session_table.value(d->m_cur_session_id);
        Q_ASSERT_X(os != nullptr, "Ktws::Workspace::selectSession", "Session from exiting id != nullptr");

        d->m_session_status = SessionEnding;
        qDebug() << ">>> Ending session" << os->id();
        emit sessionAboutToEnd(os);

        // Close worksheets
        QList<QUuid> wsids = d->m_worksheet_table.keys();
        foreach(const QUuid &id, wsids) {
            qDebug() << ">>> Closing worksheet" << id << " class:" << d->m_worksheet_table[id]->className();
            d->m_worksheet_table[id]->close();
        }

        d->m_session_status = SessionNone;
        qDebug() << ">>> Session ended";
        emit sessionEnded(os);
    }
    
    d->m_cur_session_id = id;
    
    if(!id.isNull()) {
        // Begin selected session
        Session *ss = d->m_session_table.value(id);
        Q_ASSERT_X(ss != nullptr, "Ktws::Workspace::selectSession", "Session from entering id != nullptr");

        d->m_session_status = SessionStarting;
        qDebug() << ">>> Starting session" << ss->id();
        emit sessionAboutToStart(ss);

        // Update timestamp
        ss->setLastUsedTimestamp(QDateTime::currentDateTimeUtc());
        SessionMd smd = { ss->id(), ss->name(), ss->lastUsedTimestamp() };
        writeSessionMetadata(d->m_app_id, smd);

        // Restore worksheets
        QList<WorksheetMd> wsl = scanWorksheets(d->m_app_id, id);
        foreach(const WorksheetMd &md, wsl) {
            qDebug() << ">>> Found saved worksheet" << md.id << " class:" << md.class_name;
            attachWorksheetHelper(md.class_name, md.id);
        }
        if(d->m_worksheet_table.isEmpty()) {
            // Open a new worksheet with the default handler if one is registered
            if(!d->m_default_worksheet_class.isEmpty()) {
                if(!d->m_worksheet_handler_table.contains(d->m_default_worksheet_class)) {
                    qWarning() << QString("Ktws::Workspace::selectSession: the default worksheet class %1 "
                        "doesn't have a registered handler!").arg(d->m_default_worksheet_class);
                } else {
                    qDebug() << "Creating worksheet with default handler" << d->m_default_worksheet_class;
                    attachWorksheetHelper(d->m_default_worksheet_class, QUuid::createUuid());
                }
            }
        }
        d->m_session_status = SessionRunning;
        qDebug() << ">>> Session started";
        emit sessionStarted(ss);
    }
    return true;
}
Session *Workspace::cpSession(const QString &new_name, const QUuid &clone_src) {
    Session *ns = createSession(new_name);
    qDebug() << "Ktws::Workspace:cpSession: cloning session" << clone_src << " with new name" << new_name;
    if(ns) cp_r(getSessionDir(d->m_app_id, clone_src), getSessionDir(d->m_app_id, ns->id()));
    else qWarning() << QString("Unable to create new session to clone %1 into.").arg(clone_src.toString());
    return ns;
}
void Workspace::rmSession(const QUuid &id) {
    qDebug() << "Ktws::Workspace:rmSession: deleting session" << id;
    if(d->m_session_table.remove(id)) deleteSession(d->m_app_id, id);
    else qWarning() << QString("Attempt to delete non-existant session %1").arg(id.toString());
}

Worksheet *Workspace::attachWorksheetHelper(const QString &class_name, const QUuid &id) {
    if(!d->m_worksheet_handler_table.contains(class_name)) {
        qWarning() << QString("Ktws::Workspace::attachWorksheet: No handler registered for the class name \"%1\"").arg(class_name);
        return nullptr;
    } else if(d->m_worksheet_table.contains(id)) {
        qCritical() << QString("Ktws::Workspace::attachWorksheetHelper: Duplicate id %1 attach attempted!!").arg(id.toString());
        return nullptr;
    } else {
        Worksheet *ws = d->m_worksheet_handler_table[class_name]->attach(class_name, this, id);
        if(ws) {
            d->m_worksheet_table.insert(id, ws);
            ws->show();
        }
        return ws;
    }
};
void Workspace::handleWorksheetClose(const QUuid &id) {
    qDebug() << "Ktws::Workspace::handleWorksheetClose: removing worksheet" << id << "from table";
    Worksheet *ws = d->m_worksheet_table.take(id);
    ws->deleteLater();
}
} // namespace Ktws