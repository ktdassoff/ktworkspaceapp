#include "KtwsWorksheet.hpp"
#include "KtwsWorksheet_p.hpp"
#include "KtwsApplication_p.hpp"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <QMoveEvent>

namespace Ktws {
Worksheet::Worksheet(const QString &class_name, const QUuid &id, QWidget *parent)
    : self(new WorksheetImpl(class_name, id, this) {
        // Restore any window state
        if(self->wscfg->contains("ktws_wgeom")) restoreGeometry(self->wscfg->value("ktws_wgeom").toByteArray());
        if(self->wscfg->contains("ktws_wstate")) setWindowState(Qt::WindowState(self->wscfg->value("ktws_wstate").toInt()));

        setAttribute(Qt::WA_DeleteOnClose);
}

Worksheet::~Worksheet() {
    delete d;
}

QString Worksheet::className() const { return d->className(); }
QUuid Worksheet::id() const { return d->id(); }

QSettings *Worksheet::worksheetSettings() const {
    return self->wscfg;
}

void Worksheet::closeEvent(QCloseEvent *event) {
    if(d->isExplicitClose()) {
        bool confirm = confirmClose(ExplicitClose);
        if(confirm) {
            wsApp->worksheetClose(this);	// TODO
            d->wscfg()->sync();
            event->accept();
        } else event->ignore();
    } else if(wsApp->isSessionTransition()) {
        bool confirm = confirmClose(SessionEndClose);
        if(confirm) {
            d->wscfg()->setValue("ktws_wgeom", saveGeometry());
            d->wscfg()->setValue("ktws_wstate", int(windowState()));
            d->wscfg()->sync();
            event->accept();
        } else event->ignore();
    } else {
        bool confirm = confirmClose(GeneralClose);
        if(confirm) {
            wsApp->worksheetClose(this);	// TODO
            d->wscfg()->sync();
            event->accept();
        } else event->ignore();
    }
}

bool Worksheet::confirmClose(CloseType close_type) { return true; }

bool Worksheet::explicitClose() {
    d->setExplicitClose(true);
    bool rc = close();
    d->setExplicitClose(false);
    return rc;
}
} // namespace Ktws
