#include "TaskObject.h"

int TaskObject::id() const
 { return m_id; }
QString TaskObject::frame() const
 { return m_frame; }
int TaskObject::progress() const
 { return m_progress; }
QString TaskObject::elapsed() const
 { return m_elapsed; }
TaskState::State TaskObject::status() const
 { return m_status; }
QString TaskObject::command() const
 { return m_command; }
QString TaskObject::blade_name() const
 { return m_blade_name; }
