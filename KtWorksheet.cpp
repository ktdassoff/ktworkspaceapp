#include "KtWorksheet.hpp"
#include "KtWorksheetImpl.hpp"
#include "KtWorkspaceAppImpl.hpp"

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <QMoveEvent>

KtWorksheet::KtWorksheet(const QString &class_name, const QUuid &id, const QString &title, const QIcon &custom_icon, QWidget *parent, KtWorkspaceApp *wsApp)
    : self(new KtWorksheetImpl), wsApp(wsApp), m_explicit_close(false) {
        self->class_name = class_name;
        if(id.isNull()) self->id = QUuid::createUuid();
        else self->id = id;
        self->wscfg = wsApp->getSessionSettings().release();
        self->wscfg->beginGroup(QString("ktws_") + self->id.toString());
        if(title.isNull()) {
            if(self->wscfg->contains("ktws_title")) {
                self->title = self->wscfg->value("ktws_title").toString();
                setWindowTitle(self->title);
            } else {
                self->title = wsApp->getWorksheetHandler(class_name)->classDisplayName(class_name);
                setWindowTitle(self->title);
            }
        } else {
            self->title = title;
            setWindowTitle(self->title);
        }
        setCustomIcon(custom_icon);

        // Restore any window state
        if(self->wscfg->contains("ktws_wgeom")) restoreGeometry(self->wscfg->value("ktws_wgeom").toByteArray());
        if(self->wscfg->contains("ktws_wstate")) setWindowState(Qt::WindowState(self->wscfg->value("ktws_wstate").toInt()));

        setAttribute(Qt::WA_DeleteOnClose);
}

KtWorksheet::~KtWorksheet() {
    delete self->wscfg;
    delete self;
}

QString KtWorksheet::title() const { return self->title; }
void KtWorksheet::setTitle(const QString & val) {
    if(val.isEmpty()) self->wscfg->remove("ktws_title");
    else {
        self->title = val;
        self->wscfg->setValue("ktws_title", val);
    }
    emit titleChanged(val);
}

QString KtWorksheet::className() const { return self->class_name; }
QUuid KtWorksheet::id() const { return self->id; }

QIcon KtWorksheet::customIcon() const { return self->custom_icon; }
void KtWorksheet::setCustomIcon(const QIcon & val) {
    if(val.isNull()) {
        KtWorkspaceWorksheetHandler * handler = wsApp->self->worksheet_handlers.value(self->class_name, 0);
        if(!handler) {
            qWarning() << QString("No handler registered for KtWorksheet class name of %1!").arg(self->class_name);
            self->custom_icon = wsApp->self->app_icon;
        } else if(handler->classIcon(self->class_name).isNull()) self->custom_icon = wsApp->self->app_icon;
        else self->custom_icon = handler->classIcon(self->class_name); 
    } else self->custom_icon = val;
    setWindowIcon(self->custom_icon);
}
void KtWorksheet::unsetCustomIcon() { setCustomIcon(QIcon()); }

QSettings * KtWorksheet::getWorksheetSettings() const {
    return self->wscfg;
}

void KtWorksheet::closeEvent(QCloseEvent *event) {
    if(m_explicit_close) {
        bool confirm = confirmClose(ExplicitClose);
        if(confirm) {
            wsApp->worksheetClose(this);
            self->wscfg->sync();
            event->accept();
        } else event->ignore();
    } else if(wsApp->isSessionTransition()) {
        bool confirm = confirmClose(SessionEndClose);
        if(confirm) {
            self->wscfg->setValue("ktws_wgeom", saveGeometry());
            self->wscfg->setValue("ktws_wstate", int(windowState()));
            self->wscfg->sync();
            event->accept();
        } else event->ignore();
    } else {
        bool confirm = confirmClose(GeneralClose);
        if(confirm) {
            wsApp->worksheetClose(this);
            self->wscfg->sync();
            event->accept();
        } else event->ignore();
    }
}

bool KtWorksheet::confirmClose(CloseType close_type) { return true; }

bool KtWorksheet::explicitClose() {
    m_explicit_close = true;
    bool rc = close();
    m_explicit_close = false;
    return rc;
}
