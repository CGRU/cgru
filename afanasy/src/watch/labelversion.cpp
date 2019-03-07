#include "labelversion.h"

#include "watch.h"
#include "wndtext.h"

#include <QtCore/QFile>
#include <QtGui/QImageReader>
#include <QtGui/QPainter>
#include <QTextEdit>

LabelVersion::LabelVersion(QWidget *parent):
	QWidget(parent),
	m_status(SS_None)
{
	//setSizePolicy( QSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum));
	setMinimumHeight(16);
	setMaximumHeight(16);

	m_label = QString("CGRU VERSION %1").arg(af::Environment::getVersionCGRU().c_str());
	m_tooltip = m_label;
	m_tooltip += QString("\nExecutable path: %1" ).arg(af::Environment::getExecutablePath().c_str());
	m_tooltip += QString("\nBuild at %1"         ).arg(af::Environment::getBuildDate().c_str());
	m_tooltip += QString("\nRevision: %1"        ).arg(af::Environment::getVersionRevision().c_str());
	m_tooltip += QString("\nCompiled version: %1").arg(af::Environment::getVersionCompiled().c_str());
	m_tooltip += QString("\nPython version: %1"  ).arg(af::Environment::getVersionPython().c_str());
	m_tooltip += QString("\nQt version: %1"      ).arg(qVersion());;
	if (false == af::Environment::getVersionGCC( ).empty())
		m_tooltip += QString("\nGCC version: %1" ).arg(af::Environment::getVersionGCC().c_str());
	m_tooltip += QString("\nCGRU_LOCATION=%1"    ).arg(af::Environment::getCGRULocation().c_str());
	m_tooltip += QString("\nAF_ROOT=%1"          ).arg(af::Environment::getAfRoot().c_str());
	m_tooltip += QString("\nHome: %1"            ).arg(af::Environment::getHomeAfanasy().c_str());
	m_tooltip += QString("\nServer: %1"          ).arg(af::Environment::getServerName().c_str());;

	QList<QByteArray> qList(QImageReader::supportedImageFormats());
	m_tooltip += "\nSupported image formats:\n";
	for (int i = 0; i < qList.size(); i++)
		m_tooltip += " " + qList[i];

	m_font.setPointSize(8);
	m_font.setBold(true);
	setToolTip(m_tooltip);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(refreshMessage()));
	m_timer.setInterval( 32);
}

LabelVersion::~LabelVersion()
{
}

void LabelVersion::paintEvent(QPaintEvent * event)
{
	QPainter p(this);

	if (m_status != SS_None)
	{
		if (m_status == SS_Info)
			p.fillRect(rect(), afqt::QEnvironment::clr_Window.c);
		else
			p.fillRect(rect(), afqt::QEnvironment::clr_error.c);
		p.setPen(afqt::QEnvironment::qclr_black);
		p.setFont(m_font);

		static int offset = 0;
		static const int add_offset = 50;
		static int msg_width = 0;
		QRect border(0, 0, 0, 0);
		offset = phase * 2;
		if (msg_width != 0)
			offset = offset % (msg_width + add_offset);
		int offset_cur = -offset;
		int counter = 0;
//printf("\noffset=%d width=%d\n", offset, rect().width());
		while (offset_cur <= rect().width())
		{
			p.drawText(rect().x() + offset_cur,
				rect().y(),
				rect().width() - border.x(),
				rect().height(),
				Qt::AlignVCenter | Qt::AlignLeft,
				m_message,
				&border);
			msg_width = border.width();
			offset_cur += border.width() + add_offset;
			counter ++;
			if( counter > 20 ) break;
		}
		return;
	}

	p.setPen(afqt::QEnvironment::clr_DisabledText.c);
	p.setFont(m_font);
	p.drawText(rect(), Qt::AlignCenter, m_label);
}

void LabelVersion::showMessage( const std::string & str)
{
	std::cout << str << std::endl;
	if ((m_status != SS_None) && (m_message == afqt::stoq(str))) return;
	phase = 0;
	m_message = afqt::stoq(str);
	m_status = getStringStatus(str);
	if (m_status == SS_None) return;
	setToolTip(m_message);
	repaint();
	m_timer.start();
}

void LabelVersion::refreshMessage()
{
	if (m_status == SS_None)
	{
		m_timer.stop();
		return;
	}
	phase++;
	repaint();
}

void LabelVersion::resetMessage()
{
	m_timer.stop();
	setToolTip(m_tooltip);
	m_status = SS_None;
	repaint();
	Watch::displayInfo(m_message);
}

void LabelVersion::mousePressEvent(QMouseEvent * event) { resetMessage();}

int LabelVersion::getStringStatus(const std::string & str)
{
	if (str.find("AFANASY") == 0 ) return SS_Info;
	if (str.find("ALARM")   == 0 ) return SS_Alarm;
	if (str.find("ACHTUNG") == 0 ) return SS_Alarm;
	return SS_None;
}

void LabelVersion::mouseDoubleClickEvent(QMouseEvent * event )
{
	WndText * wnd = new WndText("Info");
	wnd->insertText(m_tooltip);
}
