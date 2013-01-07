#include "KtwsWorksheet.hpp"
#include "KtwsWorksheet_p.hpp"
#include "KtwsWorkspace.hpp"
#include "KtwsSession.hpp"
#include "KtwsSerialization_p.hpp"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QSettings>

namespace Ktws {
Worksheet::Worksheet(const QString &class_name, Workspace *wspace, const QUuid &wsheet_id, QWidget *parent)
    : QMainWindow(parent),
      d(new WorksheetImpl(class_name, wspace, wspace->currentSession(), wsheet_id))
{
	// Read in stored settings
    QSettings *cfg = settings();

    // Restore any window state
    if(cfg->contains("ktws_wgeom")) restoreGeometry(cfg->value("ktws_wgeom").toByteArray());
    if(cfg->contains("ktws_wstate")) setWindowState(Qt::WindowState(cfg->value("ktws_wstate").toInt()));
    if(cfg->contains("ktws_mstate")) restoreState(cfg->value("ktws_mstate").toByteArray());
}

Worksheet::~Worksheet() {
    delete d;
}

QString Worksheet::className() const { return d->m_class_name; }
QUuid Worksheet::id() const { return d->m_wsheet_id; }

Workspace *Worksheet::workspace() const { return d->m_wspace; }
Session *Worksheet::session() const {
	return d->m_session;
}

QSettings *Worksheet::settings() {
    if(!d->m_settings) {
        d->m_settings = openSessionSettings(d->m_wspace->appId(), d->m_wsheet_id);
        d->m_settings->setParent(this);
    }
    return d->m_settings;
}

void Worksheet::restoreStateAgain() {
    QSettings *cfg = settings();
    if(cfg->contains("ktws_mstate")) restoreState(cfg->value("ktws_mstate").toByteArray());
}

void Worksheet::closeEvent(QCloseEvent *event) {
	bool confirm = true;
    if(!d->m_wspace->isSessionTransition()) confirm = confirmClose(d->m_explicit_close);

    if(confirm) {
        if(d->m_wspace->isSessionTransition()) {
            QSettings *cfg = settings();
    	    cfg->setValue("ktws_wgeom", saveGeometry());
    	    cfg->setValue("ktws_wstate", int(windowState()));
            cfg->sync();
        } else deleteWorksheet(d->m_wspace->appId(), d->m_session->id(), d->m_wsheet_id);

        d->m_wspace->handleWorksheetClose(d->m_wsheet_id);
    	event->accept();
    } else event->ignore();
}

bool Worksheet::confirmClose(bool explicit_close) { return true; }

bool Worksheet::explicitClose() {
    d->m_explicit_close = true;
    bool rc = close();
    d->m_explicit_close = false;
    return rc;
}
} // namespace Ktws
