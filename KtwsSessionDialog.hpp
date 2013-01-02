#ifndef KTWSSESSIONDIALOG_HPP_
#define KTWSSESSIONDIALOG_HPP_

#include <KtwsGlobal.hpp>

#include <QDialog>
#include <QAbstractItemModel>

namespace Ktws {
class Workspace;
class Session;

struct SessionModelImpl;
class SessionModel : public QAbstractItemModel {
    Q_OBJECT
    Q_DISABLE_COPY(SessionModel)
    SessionModelImpl *d;

public:
    explicit SessionModel(Workspace *wspace, QObject *parent = 0);
    virtual ~SessionModel();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    QModelIndex currentSessionIndex() const;
    bool isCurrentSession(const QModelIndex &index) const;

    bool renameSession(const QString &new_name, const QModelIndex &index);
    bool createSession(const QString &new_name, const QModelIndex &src_index = QModelIndex());
    bool deleteSession(const QModelIndex &index);
    bool selectSession(const QModelIndex &index);

private slots:
    void onSessionTransitioned(Session *session);
};

struct SessionDialogImpl;
class SessionDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(SessionDialog)
    SessionDialogImpl *d;

public:
    explicit SessionDialog(Workspace *wspace, QWidget *parent = nullptr);
    ~SessionDialog();

public slots:
    virtual void accept();

private slots:
    void onCreate();
    void onClone();
    void onRename();
    void onDelete();
    void onCurChange(const QModelIndex &current, const QModelIndex &previous);
};
}

#endif // KTWSSESSIONDIALOG_HPP_
