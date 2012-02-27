#pragma once

#include <QtGui/QWidget>

#include "../libafqt/name_afqt.h"

class QLineEdit;

class FileWidget : public QWidget
{
Q_OBJECT
public:
   FileWidget( QWidget * parent, afqt::Attr * attrString);
   ~FileWidget();

private slots:
   void editingFinished();
   void browse();

private:
   afqt::Attr * attr;
   QLineEdit * lineedit;
};
