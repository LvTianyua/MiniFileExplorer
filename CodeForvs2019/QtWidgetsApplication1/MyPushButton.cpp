#include "MyPushButton.h"

QMyPushButton::QMyPushButton(QWidget* parent /*= nullptr*/)
    :QPushButton(parent)
{
}

void QMyPushButton::setPicName(const QString& strPicName)
{
    m_strPicName = strPicName;
    //setFixedSize(QPixmap(strPicName).size());
}

void QMyPushButton::enterEvent(QEvent*)
{
    m_status = ENTER;
    update();
}

void QMyPushButton::leaveEvent(QEvent*)
{
    m_status = NORMAL;
    update();
}

void QMyPushButton::mousePressEvent(QMouseEvent* event)
{
    //Èôµã»÷Êó±ê×ó¼ü
    if (event->button() == Qt::LeftButton)
    {
        m_bMousePress = true;
        m_status = PRESS;
        update();
    }
}

void QMyPushButton::mouseReleaseEvent(QMouseEvent* event)
{
    //Èôµã»÷Êó±ê×ó¼ü
    if (m_bMousePress && rect().contains(event->pos()))
    {
        m_bMousePress = false;
        m_status = ENTER;
        update();
        emit clicked();
    }
}

void QMyPushButton::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QPixmap pixmap;
    switch (m_status)
    {
    case NORMAL:
    {
        pixmap.load(m_strPicName);
        break;
    }
    case ENTER:
    {
        pixmap.load(m_strPicName + QString("_hover"));
        break;
    }
    case PRESS:
    {
        pixmap.load(m_strPicName + QString("_pressed"));
        break;
    }
    case NOSTATUS:
    {
        pixmap.load(m_strPicName);
        break;
    }
    default:
        pixmap.load(m_strPicName);
    }

    painter.drawPixmap(rect(), pixmap);
}

