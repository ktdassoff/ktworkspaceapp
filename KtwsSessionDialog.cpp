#include "KtwsSessionDialog_p.hpp"

#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QListView>
#include <QInputDialog>
#include <QMessageBox>

// Model functions
KtWorkspaceSessionModel::~KtWorkspaceSessionModel() {
    // Dummy
}

QVariant KtWorkspaceSessionModel::data(const QModelIndex &index, int role) const {
    if(index.parent().isValid() || index.row() > m_sessions.count() || index.column() > 0) return QVariant();
    else if(role == Qt::DisplayRole) return m_sessions[index.row()];
    else if(role == Qt::FontRole) {
        QFont fon;
        if(m_sessions[index.row()] == m_app->currentSession()) fon.setBold(true);
        return fon;
    } else return QVariant();
}
bool KtWorkspaceSessionModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(index.row() >= 0 && index.row() < m_sessions.count() && role == Qt::EditRole) {
        m_sessions[index.row()] = value.toString();
        emit dataChanged(index, index);
        return true;
    } else return false;
}
QVariant KtWorkspaceSessionModel::headerData(int section, Qt::Orientation orientation, int role) const {
    return QVariant();
}
int KtWorkspaceSessionModel::rowCount(const QModelIndex &parent) const {
    if(parent.isValid()) return 0;
    else return m_sessions.count();
}
int KtWorkspaceSessionModel::columnCount(const QModelIndex &parent) const {
    if(parent.isValid()) return 0;
    else return 1;
}
Qt::ItemFlags KtWorkspaceSessionModel::flags(const QModelIndex &index) const {
    if(index.parent().isValid() || index.row() > m_sessions.count()) return Qt::NoItemFlags;
    else return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

bool KtWorkspaceSessionModel::renameSession(const QString &new_name, const QModelIndex &index) {
    if(index.parent().isValid() || index.row() >= m_sessions.count()) return false;
    else {
        QString old_name = m_sessions[index.row()];
        if(m_app->renameSession(new_name, old_name)) {
            setData(index, new_name);
            return true;
        } else return false;
    }
}

bool KtWorkspaceSessionModel::createSession(const QString &new_name, const QModelIndex &src_index) {
    if(src_index.isValid()) {
        if(src_index.parent().isValid() || src_index.row() >= m_sessions.count()) return false;
        else {
            QString src_name = m_sessions[src_index.row()];
            if(m_app->createSession(new_name, src_name)) {
                beginInsertRows(QModelIndex(), m_sessions.count(), m_sessions.count());
                m_sessions.append(new_name);
                endInsertRows();
                return true;
            } else return false;
        }
    } else if(m_app->createSession(new_name)) {
        beginInsertRows(QModelIndex(), m_sessions.count(), m_sessions.count());
        m_sessions.append(new_name);
        endInsertRows();
        return true;
    } else return false;
}
bool KtWorkspaceSessionModel::deleteSession(const QModelIndex &index) {
    if(index.parent().isValid() || index.row() >= m_sessions.count()) return false;
    else {
        QString name = m_sessions[index.row()];
        if(m_app->deleteSession(name)) {
            beginRemoveRows(QModelIndex(), index.row(), index.row());
            m_sessions.removeAt(index.row());
            endRemoveRows();
            return true;
        } else return false;
    }
}

bool KtWorkspaceSessionModel::selectSession(const QModelIndex &index) {
    if(index.parent().isValid() || index.row() >= m_sessions.count()) return false;
    else {
        QString name = m_sessions[index.row()];
        return m_app->selectSession(name);
    }
}

// Dialog functions
KtWorkspaceSessionDialog::KtWorkspaceSessionDialog(KtWorkspaceApp *app, QWidget *parent)
    : QDialog(parent),
    m_btn_reject(new QPushButton(tr("Cancel"), this)),
    m_btn_switch(new QPushButton(tr("Switch"), this)),
    m_btn_create(new QPushButton(tr("Create session..."), this)),
    m_btn_clone(new QPushButton(tr("Clone session..."), this)),
    m_btn_rename(new QPushButton(tr("Rename session..."), this)),
    m_btn_delete(new QPushButton(tr("Delete session"), this)),
    m_smodel(new KtWorkspaceSessionModel(app, this)),
    m_slview(new QListView(this)),
    m_app(app) {
        connect(m_btn_reject, SIGNAL(clicked()), SLOT(reject()));
        connect(m_btn_switch, SIGNAL(clicked()), SLOT(accept()));
        connect(m_btn_create, SIGNAL(clicked()), SLOT(onCreate()));
        connect(m_btn_clone, SIGNAL(clicked()), SLOT(onClone()));
        connect(m_btn_rename, SIGNAL(clicked()), SLOT(onRename()));
        connect(m_btn_delete, SIGNAL(clicked()), SLOT(onDelete()));
        m_slview->setModel(m_smodel);
        connect(m_slview->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), SLOT(onCurChange(QModelIndex, QModelIndex)));
        for(int i = 0; i < m_smodel->rowCount(); i++) {
            if(m_smodel->nameFromIndex(m_smodel->index(i)) == m_app->currentSession()) {
                m_slview->setCurrentIndex(m_smodel->index(i));
                break;
            }
        }

        QDialogButtonBox * dbb = new QDialogButtonBox(Qt::Horizontal, this);
        dbb->addButton(m_btn_switch, QDialogButtonBox::AcceptRole);
        dbb->addButton(m_btn_reject, QDialogButtonBox::RejectRole);
        dbb->addButton(m_btn_create, QDialogButtonBox::ActionRole);
        dbb->addButton(m_btn_clone, QDialogButtonBox::ActionRole);
        dbb->addButton(m_btn_rename, QDialogButtonBox::ActionRole);
        dbb->addButton(m_btn_delete, QDialogButtonBox::ActionRole);

        QBoxLayout * bl = new QBoxLayout(QBoxLayout::TopToBottom, this);
        bl->addWidget(m_slview);
        bl->addWidget(dbb);
        setLayout(bl);

        setWindowTitle(m_app->applicationName() + ": Sessions");
}

KtWorkspaceSessionDialog::~KtWorkspaceSessionDialog() {
    // Dummy
}

const char * MSG_INSNAME = "Enter a name for the new session:";
const char * MSG_BADSNAME = "Invalid session name!\nName must consist only of numbers, letters, underscores, or spaces.";
const char * MSG_NOSOP = "Unable to %1!\nPerhaps this is the name of an existing session.";

void KtWorkspaceSessionDialog::accept() {
    if(!m_smodel->selectSession(m_slview->currentIndex())) QMessageBox::critical(this, m_app->applicationName(), tr("Unable to switch session for an unknown reason!"));
    QDialog::accept();
}


void KtWorkspaceSessionDialog::onCreate() {
    QString new_name = QInputDialog::getText(this, m_app->applicationName(), tr(MSG_INSNAME));
    if(!new_name.isEmpty()) {
        if(KtWorkspaceApp::isValidSessionName(new_name)) {
            if(!m_smodel->createSession(new_name)) QMessageBox::critical(this, m_app->applicationName(), tr(MSG_NOSOP).arg("create session"));
            else {
                for(int i = 0; i < m_smodel->rowCount(); i++) {
                    if(m_smodel->nameFromIndex(m_smodel->index(i)) == new_name) {
                        m_slview->setCurrentIndex(m_smodel->index(i));
                        break;
                    }
                }
            }
        }
        else QMessageBox::critical(this, m_app->applicationName(), tr(MSG_BADSNAME));
    }
}

void KtWorkspaceSessionDialog::onClone() {
    QString new_name = QInputDialog::getText(this, m_app->applicationName(), tr(MSG_INSNAME));
    if(!new_name.isEmpty()) {
        if(KtWorkspaceApp::isValidSessionName(new_name)) {
            if(!m_smodel->createSession(new_name, m_slview->currentIndex())) QMessageBox::critical(this, m_app->applicationName(), tr(MSG_NOSOP).arg("clone session"));
            else {
                for(int i = 0; i < m_smodel->rowCount(); i++) {
                    if(m_smodel->nameFromIndex(m_smodel->index(i)) == new_name) {
                        m_slview->setCurrentIndex(m_smodel->index(i));
                        break;
                    }
                }
            }
        }
        else QMessageBox::critical(this, m_app->applicationName(), tr(MSG_BADSNAME));
    }
}

void KtWorkspaceSessionDialog::onRename() {
    QString new_name = QInputDialog::getText(this, m_app->applicationName(), tr(MSG_INSNAME));
    if(!new_name.isEmpty()) {
        if(KtWorkspaceApp::isValidSessionName(new_name)) {
            if(!m_smodel->renameSession(new_name, m_slview->currentIndex())) QMessageBox::critical(this, m_app->applicationName(), tr(MSG_NOSOP).arg("rename session"));
        }
        else QMessageBox::critical(this, m_app->applicationName(), tr(MSG_BADSNAME));
    }
}

void KtWorkspaceSessionDialog::onDelete() {
    int rc = QMessageBox::question(this, windowTitle(), QString("Are you sure you wish to delete the session <i>%1</i>?").arg(m_smodel->data(m_slview->currentIndex()).toString()), QMessageBox::Yes | QMessageBox::No);
    if(rc == QMessageBox::Yes) m_smodel->deleteSession(m_slview->currentIndex());
}

void KtWorkspaceSessionDialog::onCurChange(const QModelIndex &current, const QModelIndex &previous) {
    QString n = m_smodel->nameFromIndex(current);
    bool e = n.isEmpty();
    bool c = !e && n == m_app->currentSession();

    m_btn_switch->setEnabled(!c);
    m_btn_clone->setEnabled(!e);
    m_btn_rename->setEnabled(!c);
    m_btn_delete->setEnabled(!c);
}
