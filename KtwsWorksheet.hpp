#ifndef KTWSWORKSHEET_HPP_
#define KTWSWORKSHEET_HPP_

#include <KtwsGlobal.hpp>
#include "KtwsWorkspace.hpp"

#include <QMainWindow>
#include <QUuid>
class QSettings;

namespace Ktws {
struct WorksheetImpl;
class KTWORKSPACEAPP_EXPORT Worksheet : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY(Worksheet)

    WorksheetImpl *d;
    friend class WorksheetHandler;

    Q_PROPERTY(QString className READ className CONSTANT)
    Q_PROPERTY(QUuid id READ id CONSTANT)

public:
    Worksheet(const QString &class_name, Workspace *wspace, const QUuid &wsheet_id, QWidget *parent = nullptr);
    virtual ~Worksheet();

    QString className() const;
    QUuid id() const;
    Worksheet *clone() const;

public slots:
    bool explicitClose();

protected:
	Workspace *workspace() const;
	Session *session() const;

    QSettings *settings();
    void restoreWorksheetState();

    virtual void closeEvent(QCloseEvent *event);
    virtual bool confirmClose(bool explicit_close);
};
} // namespace Ktws

#endif // KTWSWORKSHEET_HPP_
