#pragma once
#include <qtableview.h>
class QMyTableView : public QTableView
{
    Q_OBJECT

public:
    explicit QMyTableView(QWidget* parent = nullptr);
    ~QMyTableView() = default;

    using QObject::parent;

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

    void UpdateRow(int row);

private:
    int m_currentMouseOnRow = -1;
};

