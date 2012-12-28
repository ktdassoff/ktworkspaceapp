#ifndef KTWSWORKSPACE_P_HPP_
#define KTWSWORKSPACE_P_HPP_

#include <KtwsGlobal.hpp>
#include "KtwsWorkspace.hpp"

#include <QUuid>
#include <QCoreApplication>
#include <QAction>

namespace Ktws {
struct WorkspaceImpl {
    Workspace::SessionStatus m_session_status;
    QUuid m_cur_session_id;
    QHash<QUuid, Worksheet *> m_worksheet_table;
    QHash<QString, WorksheetHandler *> m_worksheet_handler_table;
    QHash<QUuid, Session *> m_session_table;
    QString m_user_status_str;
    QList<QAction *> m_global_actions;
	QAction *m_da_about, *m_da_quit, *m_da_sessions;

	WorkspaceImpl()
    	: m_session_status(Workspace::SessionNone),
    	  m_cur_session_id(),
    	  m_worksheet_table(),
    	  m_worksheet_handler_table(),
    	  m_session_table(),
    	  m_cur_session_settings(),
    	  m_user_status_str(),
    	  m_global_actions(),
          m_da_about(nullptr),
          m_da_quit(nullptr),
          m_da_sessions(nullptr)
    {}
};
} // namespace Ktws

#endif // KTWSWORKSPACE_P_HPP_
