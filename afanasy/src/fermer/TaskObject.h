#pragma once

#include "state.hpp"

#include <QStringList>

class TaskObject
{
public:
      TaskObject(int id,
                 const QString &frame,
                 int progress,
                 const QString &elapsed,
                 TaskState::State status,
                 const QString &command,
                 const QString &blade_name
            ) :
          m_id(id)   
        , m_frame(frame)
        , m_elapsed(elapsed)
        , m_status(status)
        , m_progress(progress)
        , m_command(command)
        , m_blade_name(blade_name)
      {}

    int id() const;
    QString frame() const;
    int progress() const;
    TaskState::State status() const;
    QString elapsed() const;
    QString command() const;
    QString blade_name() const;

    int m_id;
    QString m_frame;
    int m_progress;
    QString m_elapsed;
    TaskState::State m_status;
    QString m_command;
    QString m_blade_name;


};



