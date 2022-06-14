#include "infoline.h"

InfoLine::InfoLine(QWidget * parent):
	QLineEdit(parent)
{
	setReadOnly(true);
}

InfoLine::~InfoLine()
{
}

void InfoLine::clear()
{
	QLineEdit::clear();
	setStyleSheet("");
}

void InfoLine::displayInfo(const QString &i_message)
{
	setText(i_message);
	setStyleSheet("");
}

void InfoLine::displayWarning(const QString &i_message)
{
	setText("WARNING: " + i_message);
	setStyleSheet("color: black; background-color: rgb(250,250,150)");
}

void InfoLine::displayError(const QString &i_message)
{
	setText("ERROR: " + i_message);
	setStyleSheet("color: black; background-color: rgb(250,150,150)");
}
