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

    //枚举按钮的几种状态
    enum ButtonStatus { NORMAL, ENTER, PRESS, NOSTATUS };
    ButtonStatus m_status = NORMAL;
    QString m_strPicName;

    int m_nBtnWidth = 0; //按钮宽度
    int m_nBtnHeight = 0; //按钮高度
    bool m_bMousePress = false; //按钮左键是否按下
};

