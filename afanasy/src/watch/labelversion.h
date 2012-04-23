#pragma once

#include "../libafanasy/environment.h"
#include "../libafqt/qenvironment.h"

#include <QtCore/QTimer>
#include <QtGui/QWidget>

class LabelVersion : public QWidget
{
    Q_OBJECT

public:
    LabelVersion ( QWidget *parent);
    ~LabelVersion ();

    void showMessage( const std::string & str);

    static int getStringStatus( const std::string & str);

    enum Status {
        SS_None = 0,
        SS_Info,
        SS_Alarm,
        SS_Last
    };

protected:
    void paintEvent( QPaintEvent * event);
    void mousePressEvent( QMouseEvent * event);
    void mouseDoubleClickEvent( QMouseEvent * event );

private slots:
    void refreshMessage();

private:
    void resetMessage();

private:
    QFont font;
    QString label;
    QString tooltip;
    QString message;
    //   QPixmap logo;

    QTimer timer;

    int status;

    int phase;
};
