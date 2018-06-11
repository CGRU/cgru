#include <iostream>
#include "JobObject.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>


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
QString JobObject::depends() const
 { return m_depends; }
std::string JobObject::repr() const
{  return m_json_represent; }
int JobObject::elapsed_time_int() const
{  return m_elapsed_time_int; }
QString JobObject::annotation() const
 { return m_annotation; }

void JobObject::update(const QString &user_name
                  ,JobState::State status
                  ,const QString &time_creation
                  ,int blocks_num
                  ,const QString &time_elapsed
                  ,const QString &blade_mask
                  ,const QString &exclude_blade_mask
                  ,const QString &software
                  ,int priority
                  ,int slot
                  ,int progress
                  ,const QString &name
                  ,int id
                  ,int block_order
                  ,const QList<QString> &blades
                  ,const QString &block_name
                  ,int job_id
                  ,int blades_length
                  ,const QString &approx_time
                  ,const QString &depends
                  ,const QString &user_color
                  ,int errors_avoid_blades
                  ,const std::string& json_represent
                  ,int elapsed_time_int
                  ,const QString &annotation)
{

         m_user_name = user_name;
         m_status = status;
         m_time_creation = time_creation;
         m_blocks_num = blocks_num;
         m_time_elapsed = time_elapsed;
         m_blade_mask = blade_mask;
         m_exclude_blade_mask = exclude_blade_mask;
         m_software = software;
         m_priority = priority;
         m_slot = slot;
         m_progress = progress;
         m_name = name;
         m_id = id;
         m_block_order = block_order;
         m_blades = blades;
         m_block_name = block_name;
         m_job_id = job_id;
         m_blades_length = blades_length;
         m_approx_time = approx_time;
         m_depends = depends;
         m_user_color = user_color;
         m_errors_avoid_blades = errors_avoid_blades;
         m_json_represent = json_represent;
         m_elapsed_time_int = elapsed_time_int;
         m_annotation = annotation;
}


