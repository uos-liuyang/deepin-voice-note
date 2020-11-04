/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     liuyanga <liuyanga@uniontech.com>
*
* Maintainer: liuyanga <liuyanga@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "middleview.h"
#include "vnoteapplication.h"
#include "globaldef.h"
#include "middleviewdelegate.h"
#include "middleviewsortfilter.h"
#include "common/actionmanager.h"
#include "common/standarditemcommon.h"
#include "common/vnoteitem.h"
#include "task/exportnoteworker.h"
#include "common/setting.h"
#include "db/vnoteitemoper.h"
#include "moveview.h"

#include <QMouseEvent>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QDrag>
#include <QMimeData>

#include <DApplication>
#include <DFileDialog>
#include <DLog>

/**
 * @brief MiddleView::MiddleView
 * @param parent
 */
MiddleView::MiddleView(QWidget *parent)
    : DListView(parent)
{
    initModel();
    initDelegate();
    initMenu();
    initUI();

    setContextMenuPolicy(Qt::NoContextMenu);
    this->setDragEnabled(true);
    this->setDragDropMode(QAbstractItemView::DragOnly);
    this->setAcceptDrops(false);

}

/**
 * @brief MiddleView::initModel
 */
void MiddleView::initModel()
{
    m_pDataModel = new QStandardItemModel(this);

    m_pSortViewFilter = new MiddleViewSortFilter(this);
    m_pSortViewFilter->setDynamicSortFilter(false);
    m_pSortViewFilter->setSourceModel(m_pDataModel);

    this->setModel(m_pSortViewFilter);
}

/**
 * @brief MiddleView::initDelegate
 */
void MiddleView::initDelegate()
{
    m_pItemDelegate = new MiddleViewDelegate(this);
    this->setItemDelegate(m_pItemDelegate);
}

/**
 * @brief MiddleView::initMenu
 */
void MiddleView::initMenu()
{
    m_noteMenu = ActionManager::Instance()->noteContextMenu();
}

/**
 * @brief MiddleView::setSearchKey
 * @param key 搜索关键字
 */
void MiddleView::setSearchKey(const QString &key)
{
    m_searchKey = key;
    m_pItemDelegate->setSearchKey(key);
}

/**
 * @brief MiddleView::setCurrentId
 * @param id
 */
void MiddleView::setCurrentId(qint64 id)
{
    m_currentId = id;
}

/**
 * @brief MiddleView::getCurrentId
 * @return 绑定的id
 */
qint64 MiddleView::getCurrentId()
{
    return m_currentId;
}

/**
 * @brief MiddleView::addRowAtHead
 * @param note
 */
void MiddleView::addRowAtHead(VNoteItem *note)
{
    if (nullptr != note) {
        QStandardItem *item = StandardItemCommon::createStandardItem(note, StandardItemCommon::NOTEITEM);
        m_pDataModel->insertRow(0, item);
        sortView(false);
        QModelIndex index = m_pDataModel->index(item->row(), 0);
        DListView::setCurrentIndex(m_pSortViewFilter->mapFromSource(index));
        this->scrollTo(currentIndex());
    }
}

/**
 * @brief MiddleView::appendRow
 * @param note
 */
void MiddleView::appendRow(VNoteItem *note)
{
    if (nullptr != note) {
        QStandardItem *item = StandardItemCommon::createStandardItem(note, StandardItemCommon::NOTEITEM);
        m_pDataModel->appendRow(item);
    }
}

/**
 * @brief MiddleView::clearAll
 */
void MiddleView::clearAll()
{
    m_pDataModel->clear();
}

/**
 * @brief MiddleView::deleteCurrentRow
 * @return 移除的记事项绑定的数据
 */
VNoteItem *MiddleView::deleteCurrentRow()
{
    QModelIndex index = currentIndex();
    VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
                              StandardItemCommon::getStandardItemData(index));

    m_pSortViewFilter->removeRow(index.row());

    return noteData;
}

/**
 * @brief MiddleView::getCurrVNotedata
 * @return 当前选中的记事项数据
 */
VNoteItem *MiddleView::getCurrVNotedata() const
{
    QModelIndex index = currentIndex();
    VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
                              StandardItemCommon::getStandardItemData(index));

    return noteData;
}

/**
 * @brief MiddleView::onNoteChanged
 */
void MiddleView::onNoteChanged()
{
    sortView();
}

/**
 * @brief MiddleView::rowCount
 * @return 记事项数目
 */
qint32 MiddleView::rowCount() const
{
    return m_pDataModel->rowCount();
}

/**
 * @brief MiddleView::setCurrentIndex
 * @param index
 */
void MiddleView::setCurrentIndex(int index)
{
    DListView::setCurrentIndex(m_pSortViewFilter->index(index, 0));
}

/**
 * @brief MiddleView::editNote
 */
void MiddleView::editNote()
{
    edit(currentIndex());
}

/**
 * @brief MiddleView::saveAsText
 */
void MiddleView::saveAsText()
{
    QModelIndex index = currentIndex();
    VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
                              StandardItemCommon::getStandardItemData(index));
    if (nullptr != noteData) {
        //TODO:
        //    Should check if this note is doing save action

        DFileDialog dialog;
        dialog.setFileMode(DFileDialog::DirectoryOnly);
        dialog.setLabelText(DFileDialog::Accept, DApplication::translate("MiddleView", "Save"));
        dialog.setNameFilter("TXT(*.txt)");

        QString historyDir = setting::instance()->getOption(VNOTE_EXPORT_TEXT_PATH_KEY).toString();
        if (historyDir.isEmpty()) {
            historyDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        }
        dialog.setDirectory(historyDir);

        if (QDialog::Accepted == dialog.exec()) {
            // save the directory string to config file.
            setting::instance()->setOption(VNOTE_EXPORT_TEXT_PATH_KEY, dialog.directoryUrl().toLocalFile());

            QString exportDir = dialog.directoryUrl().toLocalFile();
            ExportNoteWorker *exportWorker = new ExportNoteWorker(
                exportDir, ExportNoteWorker::ExportText, noteData);
            exportWorker->setAutoDelete(true);

            QThreadPool::globalInstance()->start(exportWorker);
        }
    }
}

/**
 * @brief MiddleView::saveRecords
 */
void MiddleView::saveRecords()
{
    QModelIndex index = currentIndex();
    VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
                              StandardItemCommon::getStandardItemData(index));
    if (nullptr != noteData) {
        //TODO:
        //    Should check if this note is doing save action

        DFileDialog dialog;
        dialog.setFileMode(DFileDialog::DirectoryOnly);
        dialog.setLabelText(DFileDialog::Accept, DApplication::translate("MiddleView", "Save"));
        dialog.setNameFilter("MP3(*.mp3)");

        QString historyDir = setting::instance()->getOption(VNOTE_EXPORT_VOICE_PATH_KEY).toString();
        if (historyDir.isEmpty()) {
            historyDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        }
        dialog.setDirectory(historyDir);

        if (QDialog::Accepted == dialog.exec()) {
            // save the directory string to config file.
            setting::instance()->setOption(VNOTE_EXPORT_VOICE_PATH_KEY, dialog.directoryUrl().toLocalFile());

            QString exportDir = dialog.directoryUrl().toLocalFile();

            ExportNoteWorker *exportWorker = new ExportNoteWorker(
                exportDir, ExportNoteWorker::ExportAllVoice, noteData);
            exportWorker->setAutoDelete(true);

            QThreadPool::globalInstance()->start(exportWorker);
        }
    }
}

/**
 * @brief MiddleView::mousePressEvent
 * @param event
 */
void MiddleView::mousePressEvent(QMouseEvent *event)
{
    this->setFocus();

    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        m_touchPressPointY = event->pos().y();
        m_touchPressStartMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    if (!m_onlyCurItemMenuEnable) {
        event->setModifiers(Qt::NoModifier);
        DListView::mousePressEvent(event);
    }

    if (event->button() == Qt::RightButton) {
        QModelIndex index = this->indexAt(event->pos());
        if (index.isValid()
                && (!m_onlyCurItemMenuEnable || index == this->currentIndex())) {
            DListView::setCurrentIndex(index);
            m_noteMenu->popup(event->globalPos());
        }
    }
}

/**
 * @brief MiddleView::mouseReleaseEvent
 * @param event
 */
void MiddleView::mouseReleaseEvent(QMouseEvent *event)
{
    m_isTouchSliding = false;
    if (!m_onlyCurItemMenuEnable) {
        DListView::mouseReleaseEvent(event);
    }
}

/**
 * @brief MiddleView::mouseDoubleClickEvent
 * @param event
 */
void MiddleView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_onlyCurItemMenuEnable) {
        DListView::mouseDoubleClickEvent(event);
    }
}

/**
 * @brief MiddleView::mouseMoveEvent
 * @param event
 */
void MiddleView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (!m_onlyCurItemMenuEnable) {
            if (event->source() == Qt::MouseEventSynthesizedByQt) {
                double distY = event->pos().y() - m_touchPressPointY;
                qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
                qint64 timeInterval = currentTime - m_touchPressStartMs;
                //上下移动距离超过10px
                if (m_isTouchSliding) {
                    if (qAbs(distY) > 5)
                        handleTouchSlideEvent(timeInterval, distY, event->pos());
                    return;
                } else {
                    if (timeInterval < 250) {
                        if (qAbs(distY) > 10) {
                            m_isTouchSliding = true;
                            handleTouchSlideEvent(timeInterval, distY, event->pos());
                        }
                        return;
                    }
                }
            }
            DListView::mouseMoveEvent(event);
        }
    }
}

/**
 * @brief LeftView::handleTouchSlideEvent  处理触摸屏move事件，区分点击、滑动、拖拽、长按功能
 * @param timeParam 时间间隔
 * @param distY 纵向移动距离
 * @param point 当前时间发生位置
 */
void MiddleView::handleTouchSlideEvent(qint64 timeParam, double distY, QPoint point)
{
    double param = ((qAbs(distY)) / timeParam) + 0.3;
    verticalScrollBar()->setSingleStep(static_cast<int>(20 * param));
    verticalScrollBar()->triggerAction((distY > 0) ? QScrollBar::SliderSingleStepSub : QScrollBar::SliderSingleStepAdd);
    m_touchPressStartMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_touchPressPointY = point.y();
}

/**
 * @brief MiddleView::eventFilter
 * @param o
 * @param e
 * @return false 不过滤，事件正常处理
 */
bool MiddleView::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);
    if (e->type() == QEvent::FocusIn) {
        m_pItemDelegate->setEditIsVisible(true);
        this->update(currentIndex());
    } else if (e->type() == QEvent::Destroy) {
        m_pItemDelegate->setEditIsVisible(false);
        this->update(currentIndex());
    }
    return false;
}

/**
 * @brief MiddleView::keyPressEvent
 * @param e
 */
void MiddleView::keyPressEvent(QKeyEvent *e)
{
    if (m_onlyCurItemMenuEnable || e->key() == Qt::Key_PageUp || e->key() == Qt::Key_PageDown) {
        e->ignore();
    } else {
        DListView::keyPressEvent(e);
    }
}

/**
 * @brief MiddleView::initUI
 */
void MiddleView::initUI()
{
    //TODO:
    //    HQ & scaler > 1 have one line at
    //the footer of DListView,so add footer
    //to solve this bug
    QWidget *footer = new QWidget(this);
    footer->setFixedHeight(1);
    addFooterWidget(footer);
    //    this->installEventFilter(this);
}

/**
 * @brief MiddleView::setVisibleEmptySearch
 * @param visible true 显示搜索无结果界面
 */
void MiddleView::setVisibleEmptySearch(bool visible)
{
    if (visible && m_emptySearch == nullptr) {
        m_emptySearch = new DLabel(this);
        m_emptySearch->setText(DApplication::translate("MiddleView", "No search results"));
        m_emptySearch->setAlignment(Qt::AlignCenter);
        DFontSizeManager::instance()->bind(m_emptySearch, DFontSizeManager::T6);
        m_emptySearch->setForegroundRole(DPalette::PlaceholderText);
        m_emptySearch->setVisible(visible);
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_emptySearch);
        this->setLayout(layout);
    } else if (m_emptySearch) {
        m_emptySearch->setVisible(visible);
    }
}

/**
 * @brief MiddleView::setOnlyCurItemMenuEnable
 * @param enable true 只有选中项右键菜单可弹出
 */
void MiddleView::setOnlyCurItemMenuEnable(bool enable)
{
    m_onlyCurItemMenuEnable = enable;
    m_pItemDelegate->setEnableItem(!enable);
    this->update();
}

/**
 * @brief MiddleView::closeEditor
 * @param editor
 * @param hint
 */
void MiddleView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(hint);
    DListView::closeEditor(editor, QAbstractItemDelegate::NoHint);
}

/**
 * @brief MiddleView::closeMenu
 */
void MiddleView::closeMenu()
{
    m_noteMenu->close();
}

/**
 * @brief MiddleView::noteStickOnTop
 */
void MiddleView::noteStickOnTop()
{
    QModelIndex index = this->currentIndex();
    VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
                              StandardItemCommon::getStandardItemData(index));
    if (noteData) {
        VNoteItemOper noteOper(noteData);
        if (noteOper.updateTop(!noteData->isTop)) {
            sortView();
        }
    }
}

/**
 * @brief MiddleView::sortView
 * @param adjustCurrentItemBar true 调整当前滚动条
 */
void MiddleView::sortView(bool adjustCurrentItemBar)
{
    m_pSortViewFilter->sortView();
    if (adjustCurrentItemBar) {
        this->scrollTo(currentIndex(), DListView::PositionAtBottom);
    }
}

QModelIndexList MiddleView::getAllSelectNote()
{
    QModelIndexList indexList;
    QModelIndex index = currentIndex();
    if (index.isValid()) {
        indexList.append(index);
    }
    return  indexList;
}

void MiddleView::deleteModelIndexs(const QModelIndexList &indexs)
{
    for (auto &it : indexs) {
        m_pSortViewFilter->removeRow(it.row());
    }
}

void MiddleView::triggerDragNote()
{
    VNoteItem *noteData = getCurrVNotedata();
    if (noteData) {
        if (m_MoveView == nullptr) {
            m_MoveView = new MoveView(this);
        }
        m_MoveView->setNote(noteData);
        QPixmap pixmap = m_MoveView->grab();
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        drag->setMimeData(mimeData);
        drag->setPixmap(pixmap);
        drag->setHotSpot(QPoint(pixmap.width() / 2, pixmap.height() / 2));
        drag->exec(Qt::MoveAction);
        drag->deleteLater();
        emit sigDragEnd();
    }
}

void MiddleView::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions)
    if (!m_onlyCurItemMenuEnable) {
        OpsStateInterface *stateInterface = OpsStateInterface::instance();
        if (!stateInterface->isSearching()) {
            QModelIndex index = currentIndex();
            QPoint pos = mapFromGlobal(QCursor::pos());
            if (index.isValid() && index == indexAt(pos)) {
                triggerDragNote();
            }
        }
    }
}
