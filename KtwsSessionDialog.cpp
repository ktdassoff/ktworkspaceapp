#include "KtwsSessionDialog.hpp"
#include "KtwsSessionDialog_p.hpp"
#include "KtwsSession.hpp"
#include "KtwsWorkspace.hpp"

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QListView>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>

namespace Ktws {
// Model functions
SessionModel::SessionModel(Workspace *wspace, QObject *parent)
    : QAbstractItemModel(parent), d(new SessionModelImpl(wspace))
{
    d->m_sessions = d->m_wspace->sessions();
    connect(wspace, SIGNAL(sessionStarted(Session *)), SLOT(onSessionTransitioned(Session *)));
    connect(wspace, SIGNAL(sessionEnded(Session *)), SLOT(onSessionTransitioned(Session *)));
}
SessionModel::~SessionModel() {
    delete d;
}

QModelIndex SessionModel::index(int row, int column, const QModelIndex &parent) const {
    if(parent.isValid() || column < 0 || column > columnCount() || row < 0 || row > rowCount()) return QModelIndex();
    else return createIndex(row, column);
}
QModelIndex SessionModel::parent(const QModelIndex &index) const {
    return QModelIndex();
}
QVariant SessionModel::data(const QModelIndex &index, int role) const {
    if(!index.parent().isValid() || index.row() < d->m_sessions.count()) {
        if(role == Qt::DisplayRole) {
            switch(index.column()) {
            case 0: return d->m_sessions[index.row()]->name();
            case 1: return d->m_sessions[index.row()]->id();
            case 2: return d->m_sessions[index.row()]->lastUsedTimestamp();
            }
        } else if(role == Qt::FontRole && d->m_sessions[index.row()]->isCurrent()) {
            QFont fon;
            fon.setBold(true);
            return fon;
        }
    }
    return QVariant();
}
bool SessionModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(index.row() >= 0 && index.row() < d->m_sessions.count() && role == Qt::EditRole) {
        d->m_sessions[index.row()]->setName(value.toString());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}
QVariant SessionModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case 0: return tr("Name");
        case 1: return tr("UUID");
        case 2: return tr("Last used");
        }
    }
    return QVariant();
}
bool SessionModel::hasChildren(const QModelIndex &parent) const {
    return !parent.isValid();
}
int SessionModel::rowCount(const QModelIndex &parent) const {
    if(parent.isValid()) return 0;
    else return d->m_sessions.count();
}
int SessionModel::columnCount(const QModelIndex &parent) const {
    if(parent.isValid()) return 0;
    else return 3;
}
Qt::ItemFlags SessionModel::flags(const QModelIndex &index) const {
    if(index.parent().isValid() || index.row() > d->m_sessions.count()) return Qt::NoItemFlags;
    else return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex SessionModel::currentSessionIndex() const {
    for(int i = 0; i < d->m_sessions.count(); i++) {
        if(d->m_sessions[i]->isCurrent()) return index(i, 0);
    }
    return QModelIndex();
}
bool SessionModel::isCurrentSession(const QModelIndex &index) const {
    return index.row() < d->m_sessions.count() && d->m_sessions[index.row()]->isCurrent();
}

bool SessionModel::renameSession(const QString &new_name, const QModelIndex &index) {
    if(index.parent().isValid() || index.row() >= d->m_sessions.count() || index.column() != 0) return false;
    else return setData(index, new_name);
}
bool SessionModel::createSession(const QString &new_name, const QModelIndex &src_index) {
    if(src_index.isValid()) {
        if(src_index.parent().isValid() || src_index.row() >= d->m_sessions.count()) return false;
        else {
            Session *sr = d->m_sessions[src_index.row()]->clone(new_name);
            if(sr) {
                beginInsertRows(QModelIndex(), d->m_sessions.count(), d->m_sessions.count());
                d->m_sessions.append(sr);
                endInsertRows();
                return true;
            }
        }
    } else {
        Session *sr = d->m_wspace->createSession(new_name);
        if(sr) {
            beginInsertRows(QModelIndex(), d->m_sessions.count(), d->m_sessions.count());
            d->m_sessions.append(sr);
            endInsertRows();
            return true;
        }
    }
    return false;
}
bool SessionModel::deleteSession(const QModelIndex &index) {
    if(index.parent().isValid() || index.row() >= d->m_sessions.count()) return false;
    else {
        d->m_sessions[index.row()]->remove();
        beginRemoveRows(QModelIndex(), index.row(), index.row());
        d->m_sessions.removeAt(index.row());
        endRemoveRows();
        return true;
    }
}
bool SessionModel::selectSession(const QModelIndex &index) {
    if(index.parent().isValid() || index.row() >= d->m_sessions.count()) return false;
    else return d->m_sessions[index.row()]->switchTo();
}

void SessionModel::onSessionTransitioned(Session *session) {
    for(int i = 0; i < d->m_sessions.count(); i++) {
        if(d->m_sessions[i]->id() == session->id()) {
            emit dataChanged(index(i, 0), index(i, columnCount()));
            break;
        }
    }
}

// Dialog functions
SessionDialog::SessionDialog(Workspace *wspace, QWidget *parent)
    : QDialog(parent), d(new SessionDialogImpl(wspace, new SessionModel(wspace, this)))
{
        d->m_btn_reject = new QPushButton(tr("Cancel"));
        connect(d->m_btn_reject, SIGNAL(clicked()), SLOT(reject()));
        d->m_btn_switch = new QPushButton(tr("Switch"));
        connect(d->m_btn_switch, SIGNAL(clicked()), SLOT(accept()));
        d->m_btn_create = new QPushButton(tr("Create session..."));
        connect(d->m_btn_create, SIGNAL(clicked()), SLOT(onCreate()));
        d->m_btn_clone = new QPushButton(tr("Clone session..."));
        connect(d->m_btn_clone, SIGNAL(clicked()), SLOT(onClone()));
        d->m_btn_rename = new QPushButton(tr("Rename session..."));
        connect(d->m_btn_rename, SIGNAL(clicked()), SLOT(onRename()));
        d->m_btn_delete = new QPushButton(tr("Delete session"));
        connect(d->m_btn_delete, SIGNAL(clicked()), SLOT(onDelete()));
        d->m_slview = new QListView;
        d->m_slview->setModel(d->m_smodel);
        connect(d->m_slview->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), SLOT(onCurChange(QModelIndex, QModelIndex)));
        d->m_slview->setCurrentIndex(d->m_smodel->currentSessionIndex());

        QDialogButtonBox * dbb = new QDialogButtonBox(Qt::Horizontal, this);
        dbb->addButton(d->m_btn_switch, QDialogButtonBox::AcceptRole);
        dbb->addButton(d->m_btn_reject, QDialogButtonBox::RejectRole);
        dbb->addButton(d->m_btn_create, QDialogButtonBox::ActionRole);
        dbb->addButton(d->m_btn_clone, QDialogButtonBox::ActionRole);
        dbb->addButton(d->m_btn_rename, QDialogButtonBox::ActionRole);
        dbb->addButton(d->m_btn_delete, QDialogButtonBox::ActionRole);
        d->m_btn_switch->setDefault(true);

        QBoxLayout * bl = new QBoxLayout(QBoxLayout::TopToBottom, this);
        bl->addWidget(d->m_slview);
        bl->addWidget(dbb);
        setLayout(bl);

        setWindowTitle(QCoreApplication::applicationName() + ": Sessions");
}
SessionDialog::~SessionDialog() {
    delete d;
}

const char * MSG_INSNAME = "Enter new session name:";
const char * MSG_NOSOP = "Unable to %1!\nPerhaps this is the name of an existing session.";

void SessionDialog::accept() {
    if(!d->m_smodel->selectSession(d->m_slview->currentIndex())) QMessageBox::critical(this, QCoreApplication::applicationName(), tr("Unable to switch session for an unknown reason!"));
    QDialog::accept();
}


void SessionDialog::onCreate() {
    QString new_name = QInputDialog::getText(this, QCoreApplication::applicationName(), tr(MSG_INSNAME));
    if(!new_name.isEmpty()) {
        if(d->m_smodel->createSession(new_name)) {
            for(int i = 0; i < d->m_smodel->rowCount(); i++) {
                QModelIndex idx = d->m_smodel->index(i, 0);
                if(d->m_smodel->data(idx).toString() == new_name) {
                    d->m_slview->setCurrentIndex(idx);
                    break;
                }
            }
        } else QMessageBox::critical(this, QCoreApplication::applicationName(), tr(MSG_NOSOP).arg("create session"));
    }
}

void SessionDialog::onClone() {
    QString new_name = QInputDialog::getText(this, QCoreApplication::applicationName(), tr(MSG_INSNAME));
    if(!new_name.isEmpty()) {
        if(d->m_smodel->createSession(new_name, d->m_slview->currentIndex())) {
            for(int i = 0; i < d->m_smodel->rowCount(); i++) {
                QModelIndex idx = d->m_smodel->index(i, 0);
                if(d->m_smodel->data(idx).toString() == new_name) {
                    d->m_slview->setCurrentIndex(idx);
                    break;
                }
            }
        } else QMessageBox::critical(this, QCoreApplication::applicationName(), tr(MSG_NOSOP).arg("clone session"));
    }
}

void SessionDialog::onRename() {
    QString new_name = QInputDialog::getText(this, QCoreApplication::applicationName(), tr(MSG_INSNAME));
    if(!new_name.isEmpty()) {
        if(!d->m_smodel->renameSession(new_name, d->m_slview->currentIndex())) QMessageBox::critical(this, QCoreApplication::applicationName(), tr(MSG_NOSOP).arg("rename session"));
    }
}

void SessionDialog::onDelete() {
    int rc = QMessageBox::question(this, windowTitle(), QString("Are you sure you wish to delete the session <i>%1</i>?").arg(d->m_smodel->data(d->m_slview->currentIndex()).toString()), QMessageBox::Yes | QMessageBox::No);
    if(rc == QMessageBox::Yes) d->m_smodel->deleteSession(d->m_slview->currentIndex());
}

void SessionDialog::onCurChange(const QModelIndex &current, const QModelIndex &previous) {
    bool c = d->m_smodel->isCurrentSession(current);

    d->m_btn_switch->setEnabled(current.isValid() && !c);
    d->m_btn_delete->setEnabled(current.isValid() && !c);
}
} // namespace Ktws
