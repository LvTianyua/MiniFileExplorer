#pragma once
#include "stdafx.h"

class QMyPushButton : public QPushButton
{
    Q_OBJECT

public:

    explicit QMyPushButton(QWidget* parent = nullptr);
    ~QMyPushButton() = default;
    void setPicName(const QString& strPicName);

protected:

    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent*);

private:

    //ö�ٰ�ť�ļ���״̬
    enum ButtonStatus { NORMAL, ENTER, PRESS, NOSTATUS };
    ButtonStatus m_status = NORMAL;
    QString m_strPicName;

    int m_nBtnWidth = 0; //��ť���
    int m_nBtnHeight = 0; //��ť�߶�
    bool m_bMousePress = false; //��ť����Ƿ���
};

