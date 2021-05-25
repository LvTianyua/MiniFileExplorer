#include "stdafx.h"
#include "MyTableView.h"
#include "TableModel.h"
QMyTableView::QMyTableView(QWidget* parent /*= nullptr*/)
    : QTableView(parent)
{
}

void QMyTableView::mousePressEvent(QMouseEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid())
    {
        QTableView::mousePressEvent(event);
    }
    else
    {
        setCurrentIndex(index);
    }
} 

void QMyTableView::mouseMoveEvent(QMouseEvent* event)
{
    UpdateRow(indexAt(event->pos()).row());
}

void QMyTableView::leaveEvent(QEvent* event)
{
    UpdateRow(-1);
}

void QMyTableView::UpdateRow(int row)
{
    if (m_currentMouseOnRow == row)
    {
        return;
    }

    if (model())
    {
        QTableModel* pModel = qobject_cast<QTableModel*>(model());
        if (pModel)
        {
            pModel->SetHoverRow(row);
            for (int i = pModel->columnCount() - 1; i >= 0; i--)
            {
                update(pModel->index(m_currentMouseOnRow, i));
                if (row != -1)
                {
                    update(pModel->index(row, i));
                }
            }
            m_currentMouseOnRow = row;
        }
    }
}
