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
	m_resources_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	getPublicLayout()->insertWidget(0, m_resources_label);
	//	m_resources_label->setFrameShape(QFrame::StyledPanel);
	//	m_resources_label->setFrameShadow(QFrame::Plain);
}

ParamsPanelTask::~ParamsPanelTask() {}

void ParamsPanelTask::updateResources(const QMap<QString, QVector<float>> &i_resmap)
{
	if (i_resmap.size() == 0)
		return;

	QStringList names;
	QVector<float> mins;
	QVector<float> maxs;
	QVector<float> avgs;

	QMapIterator<QString, QVector<float>> it(i_resmap);
	while (it.hasNext())
	{
		it.next();
		if (it.value().size() == 0)
			continue;

		float min, max, avg;
		min = max = avg = it.value()[0];

		for (int v = 1; v < it.value().size(); v++)
		{
			float val = it.value()[v];
			if (val < min)
				min = val;
			if (val > max)
				max = val;
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

	info += "<table border=1 border-style=\"none\" cellpadding=4 width=100%>";
	info += "<tr>";
	info += "<td align=center>name</td><td align=center>min</td><td align=center>avg</td><td "
			"align=center>max</td>";
	info += "</tr>";
	for (int i = 0; i < names.size(); i++)
	{
		info += "<tr>";
		QStringList td(processResource(names[i], mins[i], avgs[i], maxs[i]));
		for (int c = 0; c < td.size(); c++)
			info += QString("<td align=right><b>%1</b></td>").arg(td[c]);
		info += "</tr>";
	}
	info += "</table>";

	m_resources_label->setText(info);
	m_resources_label->setHidden(false);
}

const QStringList ParamsPanelTask::processResource(const QString &i_name, float min, float avg,
												   float max) const
{
	QStringList out;
	out.append(i_name);
	out.append(QString("%1").arg(min, 0, 'f', 2));
	out.append(QString("%1").arg(avg, 0, 'f', 2));
	out.append(QString("%1").arg(max, 0, 'f', 2));

	if (i_name == "cpu_avg")
	{
		out[0] = "CPU(%)";
		out[1] = QString("%1").arg(int(round(min)));
		out[2] = QString("%1").arg(int(round(avg)));
		out[3] = QString("%1").arg(int(round(max)));
	}
	else if (i_name == "mem_peak_mb")
	{
		out[0] = "MEM(GB)";
		out[1] = QString("%1").arg(min / 1024.0, 0, 'f', 2);
		out[2] = QString("%1").arg(avg / 1024.0, 0, 'f', 2);
		out[3] = QString("%1").arg(max / 1024.0, 0, 'f', 2);
	}
	else if (i_name.endsWith("_gb"))
	{
		out[0] = i_name.left(i_name.size() - 3) + "(GB)";
	}
	else if (min >= (1000.0 * 1000.0 * 1000.0))
	{
		out[1] = QString("%1G").arg(min / (1000.0 * 1000.0 * 1000.0), 0, 'f', 2);
		out[2] = QString("%1G").arg(avg / (1000.0 * 1000.0 * 1000.0), 0, 'f', 2);
		out[3] = QString("%1G").arg(max / (1000.0 * 1000.0 * 1000.0), 0, 'f', 2);
	}
	else if (min >= (1000.0 * 1000.0))
	{
		out[1] = QString("%1M").arg(min / (1000.0 * 1000.0), 0, 'f', 2);
		out[2] = QString("%1M").arg(avg / (1000.0 * 1000.0), 0, 'f', 2);
		out[3] = QString("%1M").arg(max / (1000.0 * 1000.0), 0, 'f', 2);
	}
	else if (min >= 1000.0)
	{
		out[1] = QString("%1K").arg(min / 1000.0, 0, 'f', 2);
		out[2] = QString("%1K").arg(avg / 1000.0, 0, 'f', 2);
		out[3] = QString("%1K").arg(max / 1000.0, 0, 'f', 2);
	}
	else if (min >= 100.0)
	{
		out[1] = QString("%1").arg(int(round(min)));
		out[2] = QString("%1").arg(int(round(avg)));
		out[3] = QString("%1").arg(int(round(max)));
	}

	return out;
}
