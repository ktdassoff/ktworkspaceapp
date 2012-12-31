#include "KtwsWorksheet.hpp"
#include "KtwsWorksheet_p.hpp"
#include "KtwsWorkspace.hpp"
#include "KtwsSession.hpp"
#include "KtwsSerialization_p.hpp"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <QMoveEvent>

namespace Ktws {
Worksheet::Worksheet(const QString &class_name, Workspace *wspace, const QUuid &wsheet_id, QWidget *parent)
    : QMainWindow(parent),
      d(new WorksheetImpl(class_name, wspace, wspace->currentSession(), wsheet_id))
{
	// Read in stored settings
    readWorksheetSettings(wspace->appId(), d->m_session->id(), wsheet_id, d->m_settings);

    // Restore any window state
    if(d->m_settings.contains("ktws_wgeom")) restoreGeometry(d->m_settings["ktws_wgeom"].toByteArray());
    if(d->m_settings.contains("ktws_wstate")) setWindowState(Qt::WindowState(d->m_settings["ktws_wstate"].toInt()));

    setAttribute(Qt::WA_DeleteOnClose);
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

QVariantHash &Worksheet::settings() {
	return d->m_settings;
}
const QVariantHash &Worksheet::settings() const {
    return d->m_settings;
}
void Worksheet::replaceSettings(const QVariantHash &settings) {
	d->m_settings = settings;
}

void Worksheet::closeEvent(QCloseEvent *event) {
	bool confirm = true;
    if(!d->m_wspace->isSessionTransition()) confirm = confirmClose(d->m_explicit_close);

    if(confirm) {
        if(d->m_wspace->isSessionTransition()) {
    	    d->m_settings["ktws_wgeom"] = saveGeometry();
    	    d->m_settings["ktws_wstate"] = int(windowState());
            writeWorksheetSettings(d->m_wspace->appId(), d->m_session->id(), d->m_wsheet_id, d->m_settings);
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
