#include "KtwsWorksheet.hpp"
#include "KtwsWorksheet_p.hpp"
#include "KtwsWorkspace.hpp"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <QMoveEvent>

namespace Ktws {
Worksheet::Worksheet(const QString &class_name, Workspace *wspace, const QUuid &session_id, const QUuid &wsheet_id, QWidget *parent)
    : QMainWindow(parent),
      d(new WorksheetImpl(class_name, wspace, session_id, wsheet_id))
{
		// Read in stored settings
		wspace->deserializeWorksheetSettings(session_id, wsheet_id, d->m_settings);

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
SessionRef Worksheet::session() const {
	return d->m_wspace->sessionById(d->m_session_id);
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
	bool confirm = false;
    if(d->m_explicit_close) {
        confirm = confirmClose(ExplicitClose);
        if(confirm) d->m_wspace->handleWorksheetClose(d->m_wsheet_id);
    } else if(d->m_wspace->isSessionTransition()) {
        confirm = confirmClose(SessionEndClose);
    } else {
        confirm = confirmClose(GeneralClose);
        if(confirm) d->m_wspace->handleWorksheetClose(d->m_wsheet_id);
    }

    if(confirm) {
    	d->m_settings["ktws_wgeom"] = saveGeometry();
    	d->m_settings["ktws_wstate"] = int(windowState());
    	d->m_wspace->serializeWorksheetSettings(d->m_session_id, d->m_wsheet_id, d->m_settings);
    	event->accept();
    } else event->ignore();
}

bool Worksheet::confirmClose(CloseType close_type) { return true; }

bool Worksheet::explicitClose() {
    d->m_explicit_close = true;
    bool rc = close();
    d->m_explicit_close = false;
    return rc;
}
} // namespace Ktws
