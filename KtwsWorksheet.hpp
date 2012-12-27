#ifndef KTWSWORKSHEET_HPP_
#define KTWSWORKSHEET_HPP_

#include <KtwsGlobal.hpp>

#include <QMainWindow>

namespace Ktws {
class Workspace;

class WorksheetImpl;
class KTWORKSPACEAPP_EXPORT Worksheet : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY(Worksheet)

    WorksheetImpl *d;
    friend class WorksheetImpl;

    Q_PROPERTY(QString className READ className CONSTANT)
    Q_PROPERTY(QUuid id READ id CONSTANT)

public:
    Worksheet(const QString &class_name, Workspace *wspace, const QUuid &session_id, const QUuid &wsheet_id, QWidget *parent = nullptr);
    virtual ~Worksheet();

    QString className() const;
    QUuid id() const;

public slots:
    bool explicitClose();

protected:
    QVariantHash &settings();
    const QVariantHash &settings() const;
    void replaceSettings(const QVariantHash &settings);
    void cloneFromWorksheet(const Worksheet *source);

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
