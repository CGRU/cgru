#include "JobObject.h"

QString JobObject::user_name() const
 { return m_user_name; }
QString JobObject::time_creation() const
 { return m_time_creation; }
int JobObject::block_order() const
 { return m_block_order; }
QString JobObject::software() const
 { return m_software; }
int JobObject::priority() const
 { return m_priority; }
int JobObject::progress() const
 { return m_progress; }
QString JobObject::name() const
 { return m_name; }
int JobObject::id() const
 { return m_id; }
QList<QString> JobObject::blades() const
 { return m_blades; }
QString JobObject::block_name() const
 { return m_block_name; }
int JobObject::blades_length() const
 { return m_blades_length; }
QString JobObject::approxTime()
 { return m_block_name; }

