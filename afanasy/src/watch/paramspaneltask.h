#pragma once

#include "../libafqt/name_afqt.h"

#include "paramspanel.h"

#include <QWidget>

class QLabel;

class ParamsPanelTask : public ParamsPanel
{
	Q_OBJECT
  public:
	ParamsPanelTask();
	virtual ~ParamsPanelTask();

	void updateResources(const QMap<QString, QVector<float>> &i_resmap);

  private:
	const QStringList processResource(const QString &i_name, float min, float avg, float max) const;

  private:
	QLabel *m_resources_label;
};
