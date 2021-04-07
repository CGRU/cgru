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

	void updateJobProgress(const af::JobProgress * i_job_progress);

private:
	const QStringList processResource(const QString & i_name, int min, int avg, int max) const;

private:
	QLabel * m_resources_label;
};

