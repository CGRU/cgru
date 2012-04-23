#pragma once

#include "../libafqt/name_afqt.h"

#include <QtGui/QWidget>

class QLineEdit;

class FileWidget : public QWidget
{
Q_OBJECT
public:
    FileWidget( QWidget * i_parent, afqt::Attr * i_attr,
                const QString & i_filesmask = QString(""));
    ~FileWidget();

private slots:
    void editingFinished();
    void browse();

private:
    afqt::Attr * m_attr;
    QLineEdit * m_lineedit;

    QString m_filesmask;
};
