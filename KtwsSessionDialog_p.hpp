#ifndef KTWORKSPACESESSIONDIALOG_HPP
#define KTWORKSPACESESSIONDIALOG_HPP

#include "KtWorkspaceAppGlobal.hpp"
#include "KtWorkspaceApp.hpp"

#include <QDialog>
#include <QPushButton>
#include <QAbstractListModel>
#include <QStringList>
#include <QListView>

class KtWorkspaceSessionModel : public QAbstractListModel {
    Q_OBJECT
    Q_DISABLE_COPY(KtWorkspaceSessionModel)
    QStringList m_sessions;
    KtWorkspaceApp *m_app;

public:
    explicit KtWorkspaceSessionModel(KtWorkspaceApp *app, QObject *parent = 0)
        : QAbstractListModel(parent), m_app(app) {
            m_sessions = m_app->allSessions();
    }
    virtual ~KtWorkspaceSessionModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    QString nameFromIndex(const QModelIndex &index) const {
        if(!index.parent().isValid() && index.row() < m_sessions.count()) return m_sessions[index.row()];
        else return QString();
    }
    bool renameSession(const QString &new_name, const QModelIndex &index);
    bool createSession(const QString &new_name, const QModelIndex &src_index = QModelIndex());
    bool deleteSession(const QModelIndex &index);
    bool selectSession(const QModelIndex &index);
};

class KtWorkspaceSessionDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(KtWorkspaceSessionDialog)
    QPushButton *m_btn_reject, *m_btn_switch, *m_btn_create, *m_btn_clone, *m_btn_rename, *m_btn_delete;
    KtWorkspaceSessionModel *m_smodel;
    QListView *m_slview;
    KtWorkspaceApp *m_app;

public:
    explicit KtWorkspaceSessionDialog(KtWorkspaceApp *app = KtWorkspaceApp::instance(), QWidget *parent = 0);
    ~KtWorkspaceSessionDialog();

public slots:
    virtual void accept();

private slots:
    void onCreate();
    void onClone();
    void onRename();
    void onDelete();
    void onCurChange(const QModelIndex &current, const QModelIndex &previous);
};

#endif // !define(KTWORKSPACESESSIONDIALOG_HPP)
