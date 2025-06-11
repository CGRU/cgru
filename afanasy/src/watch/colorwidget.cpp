#include "colorwidget.h"

#include "../libafqt/attrcolor.h"
#include "../libafqt/qenvironment.h"

#include "watch.h"

#include <QColorDialog>
#include <QLabel>
#include <QLayout>
#include <QtGui/QColor>
#include <QtGui/QPainter>

ColorWidget::ColorWidget(QWidget *parent, afqt::AttrColor *color) : QWidget(parent), clr(color)
{
	setMinimumWidth(230);
	setFixedHeight(18);
}

ColorWidget::~ColorWidget() {}

void ColorWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	QRect r(rect());

	painter.fillRect(r, afqt::QEnvironment::clr_item.c);

	painter.setPen(afqt::QEnvironment::clr_textbright.c);
	painter.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, clr->getLabel());

	static const int offset = 100;
	static const int margin = 2;

	painter.fillRect(r.x() + r.width() - offset, r.y() + margin, offset - margin,
					 r.height() - margin - margin, clr->c);
}

void ColorWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	// printf("ColorWidget::mouseDoubleClickEvent:\n");
	QColorDialog *dialog = new QColorDialog(clr->c, this);
	dialog->setModal(false);
	dialog->setOptions(QColorDialog::NoButtons);
	connect(dialog, SIGNAL(currentColorChanged(const QColor &)), this,
			SLOT(currentColorChanged(const QColor &)));
	connect(dialog, SIGNAL(finished(int)), this, SLOT(finished(int)));
	Watch::repaintStart();
	dialog->open();
	//   dialog->open( this, "accepted");
}
void ColorWidget::currentColorChanged(const QColor &color)
{
	clr->c = color;
	repaint();
}

void ColorWidget::finished(int)
{
	// printf("ColorWidget::finished:\n");
	Watch::repaint();
	Watch::repaintFinish();
}

void ColorWidget::accepted()
{
	printf("ColorWidget::accepted:\n");
	Watch::repaint();
	Watch::repaintFinish();
}
