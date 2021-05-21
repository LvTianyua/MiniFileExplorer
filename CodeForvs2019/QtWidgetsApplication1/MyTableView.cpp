#include "stdafx.h"
#include "MyTableView.h"
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
