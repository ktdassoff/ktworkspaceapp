#include "KtwsSessionDialog_p.hpp"

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QListView>
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>

namespace Ktws {
// Model functions
SessionModel::~SessionModel() {}

QModelIndex SessionModel::index(int row, int column, const QModelIndex &parent) const {
    if(parent.isValid() || column < 0 || column > columnCount() || row < 0 || row > rowCount()) return QModelIndex();
    else return createIndex(row, column);
}
QModelIndex SessionModel::parent(const QModelIndex &index) const {
    return QModelIndex();
}
QVariant SessionModel::data(const QModelIndex &index, int role) const {
    if(!index.parent().isValid() || index.row() < m_sessions.count()) {
        if(role == Qt::DisplayRole) {
            switch(index.column()) {
            case 0: return m_sessions[index.row()]->name();
            case 1: return m_sessions[index.row()]->id();
            case 2: return m_sessions[index.row()]->lastUsedTimestamp();
            }
        } else if(role == Qt::FontRole && m_sessions[index.row()]->isCurrent()) {
            QFont fon;
            fon.setBold(true);
            return fon;
        }
    }
    return QVariant();
}
bool SessionModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(index.row() >= 0 && index.row() < m_sessions.count() && role == Qt::EditRole) {
        if(m_sessions[index.row()]->setName(value.toString())) {
            emit dataChanged(index, index);
            return true;
        }
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
int SessionModel::rowCount(const QModelIndex &parent) const {
    if(parent.isValid()) return 0;
    else return m_sessions.count();
}
int SessionModel::columnCount(const QModelIndex &parent) const {
    if(parent.isValid()) return 0;
    else return 3;
}
Qt::ItemFlags SessionModel::flags(const QModelIndex &index) const {
    if(index.parent().isValid() || index.row() > m_sessions.count()) return Qt::NoItemFlags;
    else return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex SessionModel::currentSessionIndex() const {
    for(int i = 0; i < m_sessions.count(); i++) {
        if(m_sessions[i]->isCurrent()) return index(i, 0);
    }
    return QModelIndex();
}
bool SessionModel::isCurrentSession(const QModelIndex &index) const {
    return index.row() < m_sessions.count() && m_sessions[index.row()]->isCurrent();
}

bool SessionModel::renameSession(const QString &new_name, const QModelIndex &index) {
    if(index.parent().isValid() || index.row() >= m_sessions.count() || index.column() != 0) return false;
    else return setData(index, new_name);
}
bool SessionModel::createSession(const QString &new_name, const QModelIndex &src_index) {
    if(src_index.isValid()) {
        if(src_index.parent().isValid() || src_index.row() >= m_sessions.count()) return false;
        else {
            Session *sr = m_sessions[src_index.row()]->clone(new_name);
            if(sr) {
                beginInsertRows(QModelIndex(), m_sessions.count(), m_sessions.count());
                m_sessions.append(sr);
                endInsertRows();
                return true;
            }
        }
    } else {
        Session *sr = m_wspace->createSession(new_name);
        if(sr) {
            beginInsertRows(QModelIndex(), m_sessions.count(), m_sessions.count());
            m_sessions.append(sr);
            endInsertRows();
            return true;
        }
    }
    return false;
}
bool SessionModel::deleteSession(const QModelIndex &index) {
    if(index.parent().isValid() || index.row() >= m_sessions.count()) return false;
    else {
        if(m_sessions[index.row()]->remove()) {
            beginRemoveRows(QModelIndex(), index.row(), index.row());
            m_sessions.removeAt(index.row());
            endRemoveRows();
            return true;
        } else return false;
    }
}
bool SessionModel::selectSession(const QModelIndex &index) {
    if(index.parent().isValid() || index.row() >= m_sessions.count()) return false;
    else return m_sessions[index.row()]->switchTo();
}

// Dialog functions
SessionDialog::SessionDialog(Workspace *wspace, QWidget *parent)
    : QDialog(parent),
    m_btn_reject(new QPushButton(tr("Cancel"), this)),
    m_btn_switch(new QPushButton(tr("Switch"), this)),
    m_btn_create(new QPushButton(tr("Create session..."), this)),
    m_btn_clone(new QPushButton(tr("Clone session..."), this)),
    m_btn_rename(new QPushButton(tr("Rename session..."), this)),
    m_btn_delete(new QPushButton(tr("Delete session"), this)),
    m_smodel(new SessionModel(wspace, this)),
    m_slview(new QListView(this)),
    m_wspace(wspace) {
        connect(m_btn_reject, SIGNAL(clicked()), SLOT(reject()));
        connect(m_btn_switch, SIGNAL(clicked()), SLOT(accept()));
        connect(m_btn_create, SIGNAL(clicked()), SLOT(onCreate()));
        connect(m_btn_clone, SIGNAL(clicked()), SLOT(onClone()));
        connect(m_btn_rename, SIGNAL(clicked()), SLOT(onRename()));
        connect(m_btn_delete, SIGNAL(clicked()), SLOT(onDelete()));
        m_slview->setModel(m_smodel);
        connect(m_slview->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), SLOT(onCurChange(QModelIndex, QModelIndex)));
        m_slview->setCurrentIndex(m_smodel->currentSessionIndex());

        QDialogButtonBox * dbb = new QDialogButtonBox(Qt::Horizontal, this);
        dbb->addButton(m_btn_switch, QDialogButtonBox::AcceptRole);
        dbb->addButton(m_btn_reject, QDialogButtonBox::RejectRole);
        dbb->addButton(m_btn_create, QDialogButtonBox::ActionRole);
        dbb->addButton(m_btn_clone, QDialogButtonBox::ActionRole);
        dbb->addButton(m_btn_rename, QDialogButtonBox::ActionRole);
        dbb->addButton(m_btn_delete, QDialogButtonBox::ActionRole);
        m_btn_switch->setDefault(true);

        QBoxLayout * bl = new QBoxLayout(QBoxLayout::TopToBottom, this);
        bl->addWidget(m_slview);
        bl->addWidget(dbb);
        setLayout(bl);

        setWindowTitle(QCoreApplication::applicationName() + ": Sessions");
}
SessionDialog::~SessionDialog() {}

const char * MSG_INSNAME = "Enter new session name:";
const char * MSG_NOSOP = "Unable to %1!\nPerhaps this is the name of an existing session.";

void SessionDialog::accept() {
    if(!m_smodel->selectSession(m_slview->currentIndex())) QMessageBox::critical(this, QCoreApplication::applicationName(), tr("Unable to switch session for an unknown reason!"));
    QDialog::accept();
}


void SessionDialog::onCreate() {
    QString new_name = QInputDialog::getText(this, QCoreApplication::applicationName(), tr(MSG_INSNAME));
    if(!new_name.isEmpty()) {
        if(m_smodel->createSession(new_name)) {
            for(int i = 0; i < m_smodel->rowCount(); i++) {
                QModelIndex idx = m_smodel->index(i, 0);
                if(m_smodel->data(idx).toString() == new_name) {
                    m_slview->setCurrentIndex(idx);
                    break;
                }
            }
        } else QMessageBox::critical(this, QCoreApplication::applicationName(), tr(MSG_NOSOP).arg("create session"));
    }
}

void SessionDialog::onClone() {
    QString new_name = QInputDialog::getText(this, QCoreApplication::applicationName(), tr(MSG_INSNAME));
    if(!new_name.isEmpty()) {
        if(m_smodel->createSession(new_name, m_slview->currentIndex())) {
            for(int i = 0; i < m_smodel->rowCount(); i++) {
                QModelIndex idx = m_smodel->index(i, 0);
                if(m_smodel->data(idx).toString() == new_name) {
                    m_slview->setCurrentIndex(idx);
                    break;
                }
            }
        } else QMessageBox::critical(this, QCoreApplication::applicationName(), tr(MSG_NOSOP).arg("clone session"));
    }
}

void SessionDialog::onRename() {
    QString new_name = QInputDialog::getText(this, QCoreApplication::applicationName(), tr(MSG_INSNAME));
    if(!new_name.isEmpty()) {
        if(!m_smodel->renameSession(new_name, m_slview->currentIndex())) QMessageBox::critical(this, QCoreApplication::applicationName(), tr(MSG_NOSOP).arg("rename session"));
    }
}

void SessionDialog::onDelete() {
    int rc = QMessageBox::question(this, windowTitle(), QString("Are you sure you wish to delete the session <i>%1</i>?").arg(m_smodel->data(m_slview->currentIndex()).toString()), QMessageBox::Yes | QMessageBox::No);
    if(rc == QMessageBox::Yes) m_smodel->deleteSession(m_slview->currentIndex());
}

void SessionDialog::onCurChange(const QModelIndex &current, const QModelIndex &previous) {
    bool c = m_smodel->isCurrentSession(current);

    m_btn_switch->setEnabled(current.isValid() && !c);
    m_btn_delete->setEnabled(current.isValid() && !c);
}
} // namespace Ktws
