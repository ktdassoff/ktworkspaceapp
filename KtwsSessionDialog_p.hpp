#ifndef SessionDialog_HPP
#define SessionDialog_HPP

#include <KtwsGlobal.hpp>
#include "KtwsWorkspace.hpp"
#include "KtwsSession.hpp"

#include <QDialog>
#include <QPushButton>
#include <QAbstractItemModel>
#include <QStringList>
#include <QListView>

namespace Ktws {
class SessionModel : public QAbstractItemModel {
    Q_OBJECT
    Q_DISABLE_COPY(SessionModel)
    QList<Session *> m_sessions;
    Workspace *m_wspace;

public:
    explicit SessionModel(Workspace *wspace, QObject *parent = 0)
        : QAbstractItemModel(parent), m_wspace(wspace) {
            m_sessions = m_wspace->sessions();
    }
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
};

class SessionDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(SessionDialog)
    QPushButton *m_btn_reject, *m_btn_switch, *m_btn_create, *m_btn_clone, *m_btn_rename, *m_btn_delete;
    SessionModel *m_smodel;
    QListView *m_slview;
    Workspace *m_wspace;

public:
    explicit SessionDialog(Workspace *wspace, QWidget *parent = 0);
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

#endif // !define(SessionDialog_HPP)
