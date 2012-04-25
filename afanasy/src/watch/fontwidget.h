#pragma once

#include "../libafqt/name_afqt.h"

#include <QtGui/QWidget>

class QLineEdit;

class FontWidget : public QWidget
{
Q_OBJECT
public:
   FontWidget( QWidget * parent, afqt::Attr * attrString);
   ~FontWidget();

private slots:
   void editingFinished();

private:
   afqt::Attr * attr;
   QLineEdit * lineedit;
};
