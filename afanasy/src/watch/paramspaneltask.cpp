#include "paramspaneltask.h"

#include <QBoxLayout>
#include <QLabel>
#include <QVector>

#include "../libafanasy/jobprogress.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ParamsPanelTask::ParamsPanelTask()
{
	m_resources_label = new QLabel();
	m_resources_label->setHidden(true);
	getPublicLayout()->insertWidget(0, m_resources_label);
	m_resources_label->setFrameShape(QFrame::StyledPanel);
	m_resources_label->setFrameShadow(QFrame::Plain);
}

ParamsPanelTask::~ParamsPanelTask()
{
}

void ParamsPanelTask::updateJobProgress(const af::JobProgress * i_job_progress)
{
	QMap<QString, QVector<int>> map;

	for (int b = 0; b < i_job_progress->getBlocksNum(); b++)
	{
		for (int t = 0; t < i_job_progress->getTasksNum(b); t++)
		{
			QStringList pair = afqt::stoq(i_job_progress->tp[b][t]->resources).split(' ');
			for (int i = 0; i < pair.size(); i++)
			{
				QStringList res = pair[i].split(':');
				if (res.size() != 2)
					continue;

				map[res[0]].append(res[1].toInt());
			}
		}
	}

	if (map.size() == 0)
		return;

	QStringList names;
	QVector<int> mins;
	QVector<int> maxs;
	QVector<int> avgs;

	QMapIterator<QString, QVector<int>> it(map);
	while (it.hasNext())
	{
		it.next();
		if (it.value().size() == 0)
			continue;

		int min, max, avg;
		min = max = avg = it.value()[0];

		for (int v = 1; v < it.value().size(); v++)
		{
			int val = it.value()[v];
			if (val < min) min = val;
			if (val > max) max = val;
			avg += val;
		}

		names.append(it.key());
		mins.append(min);
		maxs.append(max);
		avgs.append(avg / it.value().size());
	}

	if (names.size() == 0)
		return;

	QString info = "Resources:";

	info += "<table>";
	info += "<tr>";
	info += "<td>name</td><td>min</td><td>avg</td><td>max</td>";
	info += "</tr>";
	for (int i = 0; i < names.size(); i++)
	{
		info += "<tr>";
		QStringList td(processResource(names[i], mins[i], avgs[i], maxs[i]));
		for (int c = 0; c < td.size(); c++)
			info += QString("<td>%1</td>").arg(td[c]);
		info += "</tr>";
	}
	info += "</table>";

	m_resources_label->setText(info);
	m_resources_label->setHidden(false);
}

const QStringList ParamsPanelTask::processResource(const QString & i_name, int min, int avg, int max) const
{
	QStringList out = {i_name, QString("%1").arg(min), QString("%1").arg(avg), QString("%1").arg(max)};

	if (i_name == "cpu_avg")
	{
		out[0] = "CPU(%)";
		out[1] = QString("%1").arg(min);
		out[2] = QString("%1").arg(avg);
		out[3] = QString("%1").arg(max);
	}
	else if (i_name == "mem_peak_mb")
	{
		out[0] = "MEM(GB)";
		out[1] = QString("%1").arg(double(min) / 1024.0, 0, 'f', 2);
		out[2] = QString("%1").arg(double(avg) / 1024.0, 0, 'f', 2);
		out[3] = QString("%1").arg(double(max) / 1024.0, 0, 'f', 2);
	}

	return out;
}

