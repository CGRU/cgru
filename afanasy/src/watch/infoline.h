#pragma once

#include "../libafqt/qenvironment.h"

#include <QLineEdit>

class InfoLine : public QLineEdit
{
  public:
	InfoLine(QWidget *parent);
	~InfoLine();

	void clear();

	void displayInfo(const QString &i_message);
	void displayWarning(const QString &i_message);
	void displayError(const QString &i_message);

  private:
};
