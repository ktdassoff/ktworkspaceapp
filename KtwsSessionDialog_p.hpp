#ifndef KTWSSESSIONDIALOG_P_HPP_
#define KTWSSESSIONDIALOG_P_HPP_

#include <KtwsGlobal.hpp>

#include <QList>
class QPushButton;
class QListView;

namespace Ktws {
class Workspace;
class Session;

struct SessionModelImpl {
	QList<Session *> m_sessions;
	Workspace *m_wspace;
	
	SessionModelImpl(Workspace *wspace)
		: m_sessions(), m_wspace(wspace)
	{}
};

struct SessionDialogImpl {
    Workspace *m_wspace;
    SessionModel *m_smodel;
    QListView *m_slview;
    QPushButton *m_btn_reject, *m_btn_switch, *m_btn_create, *m_btn_clone, *m_btn_rename, *m_btn_delete;
	
	SessionDialogImpl(Workspace *wspace, SessionModel *smodel)
		: m_wspace(wspace), m_smodel(smodel), m_slview(nullptr),
		  m_btn_reject(nullptr), m_btn_switch(nullptr), m_btn_create(nullptr),
		  m_btn_clone(nullptr), m_btn_rename(nullptr), m_btn_delete(nullptr)
	{}
};
} // namespace Ktws

#endif // KTWSSESSIONDIALOG_P_HPP_