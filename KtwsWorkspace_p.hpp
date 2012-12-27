#ifndef KTWSWORKSPACE_P_HPP_
#define KTWSWORKSPACE_P_HPP_

#include "KtwsGlobal.hpp"
#include "KtwsWorkspace.hpp"

#include <QHash>
#include <QString>
#include <QAction>
#include <QSystemTrayIcon>
#include <QDebug>

namespace Ktws {
class Worksheet;

struct SessionInfo {
	QString name;
	QDateTime timestamp;
};

QHash<QUuid, SessionInfo> scanSessions();

class WorkspaceImpl : public QObject {
    Q_OBJECT
	Q_DISABLE_COPY(WorkspaceImpl)

    Workspace *m_cat;
    Workspace::SessionStatus m_session_status;
    QUuid m_cur_session_id;
    QHash<QUuid, Worksheet *> m_worksheet_table;
    QHash<QString, WorksheetHandler *> m_worksheet_handler_table;
    QHash<QUuid, SessionInfo> m_session_table;
    QVariantHash m_cur_session_settings;
    QString m_user_status_str;
    QList<QAction *> m_global_actions;
	QAction *m_da_about, *m_da_quit, *m_da_sessions, *m_da_worksheets;

public:
    static WorkspaceImpl *instance();

    explicit WorkspaceImpl(Workspace *cat)
    	: QObject(cat), m_cat(cat),
    	  m_session_status(Workspace::SessionNone),
    	  m_cur_session_id(),
    	  m_worksheet_table(),
    	  m_worksheet_handler_table(),
    	  m_session_table(scanSessions()),
    	  m_cur_session_settings(),
    	  m_user_status_str(),
    	  m_global_actions(),
    	  m_da_about(nullptr), m_da_quit(nullptr), m_da_sessions(nullptr), m_da_worksheets(nullptr)
    {

    }
    virtual ~WorkspaceImpl() {}

    // Session management
    SessionStatus sessionStatus() const { return m_session_status; }
    QUuid currentSessionId() const { return m_cur_session_id; }
    int sessionCount() const { return m_session_table.count(); }
    QList<QUuid> sessionIds() const { return m_session_table.keys(); }
    const SessionInfo *querySession(const QUuid &id) { return m_session_table.value(id, nullptr); }
    bool selectSession(const QUuid &id);
    bool mkSession(const QString &new_name, const QUuid &clone_src = QUuid());
    bool rmSession(const QUuid &id);
    bool renameSession(const QString &new_name, const QUuid &id);

    // Settings and data
    QVariantHash &globalSettings() { return m_cur_session_settings; }
    QString baseDataDir() const;
    bool deserializeGlobalSettings();
    bool serializeGlobalSettings();
    bool deserializeSessionSettings(const QUuid &session_id, QVariantHash &target);
    bool serializeSessionSettings(const QUuid &session_id, const QVariantHash &source);
    void clearSessionSettings(const QUuid &session_id);
    bool deserializeWorksheetSettings(const QUuid &session_id, const QUuid &worksheet_id, QVariantHash &target);
    bool serializeWorksheetSettings(const QUuid &session_id, const QUuid &worksheet_id, const QVariantHash &source);
    void clearWorksheetSettings(const QUuid &session_id, const QUuid &worksheet_id);

    // Global notifications and status
    QString &userStatus() { return m_user_status_str; }
    const QString &userStatus() const { return m_user_status_str; }

    // Global actions
    QList<QAction *> &globalActions() { return m_global_actions; }
    const QList<QAction *> &globalActions() const { return m_global_actions; }
    QAction *defaultAction(DefaultAction da);

    // Worksheet management
    int worksheetCount() const { return m_worksheet_table.count(); }
    QList<QUuid> worksheetIds() const { return m_worksheet_table.keys(); }

    int worksheetHandlerCount() const { return m_worksheet_handler_table.count(); }
    QList<QString> worksheetHandlerClassNames() const { return m_worksheet_handler_table.keys(); }
    bool registerWorksheetHandler(const QString &class_name, WorksheetHandler *handler) {
    	if(m_worksheet_handler_table.contains(class_name)) {
    		qWarning() << QString("Ktws::Workspace: attempt to register duplicate worksheet handler class \"%1\"")
    			.arg(class_name);
    		return false;
    	} else if(!handler) {
    		qWarning() << QString("Ktws::Workspace: attempt to register null handler for class \"%1\"")
    			.arg(class_name);
    		return false;
    	} else {
    		m_worksheet_handler_table.insert(class_name, handler);
    		return true;
    	}
    }
    WorksheetHandler *worksheetHandler(const QString &class_name) {
		return m_worksheet_handler_table.value(class_name, nullptr);
	}
    void unregisterWorksheetHandler(const QString &class_name) {
    	if(m_worksheet_handler_table.contains(class_name)) {
    		m_worksheet_handler_table.remove(class_name);
    	} else {
    		qWarning() << QString("Ktws::Workspace: attempt to unregister not-registered worksheet handler for class \"%1\"")
    			.arg(class_name);
    	}
    }

    WorksheetRef attachWorksheetHelper(const QString &class_name, const QUuid &id, bool add_saved);
    void detachWorksheetHelper(WorksheetRef worksheet, bool already_closed, bool rm_saved);
};
} // namespace Ktws

#endif // KTWSWORKSPACE_P_HPP_
