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

};

