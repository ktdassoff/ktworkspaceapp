#ifndef KTWSWORKSHEET_HPP_
#define KTWSWORKSHEET_HPP_

#include <KtwsGlobal.hpp>
#include "KtwsWorkspace.hpp"

#include <QMainWindow>
#include <QVariant>
class QString;
class QUuid;

namespace Ktws {
struct WorksheetImpl;
class KTWORKSPACEAPP_EXPORT Worksheet : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY(Worksheet)

    WorksheetImpl *d;

    Q_PROPERTY(QString className READ className CONSTANT)
    Q_PROPERTY(QUuid id READ id CONSTANT)

public:
    Worksheet(const QString &class_name, Workspace *wspace, const QUuid &session_id, const QUuid &wsheet_id, QWidget *parent = nullptr);
    virtual ~Worksheet();

    QString className() const;
    QUuid id() const;
    Worksheet *clone() const;

public slots:
    bool explicitClose();

protected:
	Workspace *workspace() const;
	SessionRef session() const;

    QVariantHash &settings();
    const QVariantHash &settings() const;
    void replaceSettings(const QVariantHash &settings);

    virtual void closeEvent(QCloseEvent *event);
    enum CloseType {
        GeneralClose,
        ExplicitClose,
        SessionEndClose
    };
    virtual bool confirmClose(CloseType close_type);
};
} // namespace Ktws

#endif // KTWSWORKSHEET_HPP_
