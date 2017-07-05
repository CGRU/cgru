#ifndef STATE_HPP
#define STATE_HPP

#include <QObject>

#if QT_VERSION < 0x050000
    #include <QtDeclarative>
#else
    #include <QtQml/QQmlApplicationEngine>
#endif

#undef ERROR

class TaskState : public QObject
{
    Q_OBJECT

    public:
        enum State
        {
            DONE, READY, SKIPPED, ERROR, RUNNING
        };
        Q_ENUMS(State)

        static void declareQML() {
            qmlRegisterType<TaskState>("TaskStateEnums", 1, 0, "TaskState");
        }
};

Q_DECLARE_METATYPE(TaskState::State)


class BladeState : public QObject
{
    Q_OBJECT

    public:
        enum State
        {
            READY, BUSY, OFFLINE, NIMBY, BIG_NIMBY, PAUSED, DIRTY
        };
        Q_ENUMS(State)

        static void declareQML() {
            qmlRegisterType<BladeState>("BladeStateEnums", 1, 0, "BladeState");
        }
};

Q_DECLARE_METATYPE(BladeState::State)


class JobState : public QObject
{
    Q_OBJECT

    public:
        enum State
        {
            STARTED, READY, RUNNING, DONE, OFFLINE, ERROR, WAITTIME
        };
        Q_ENUMS(State)

        static void declareQML() {
            qmlRegisterType<JobState>("JobStateEnums", 1, 0, "JobState");
        }
};

Q_DECLARE_METATYPE(JobState::State)

#endif    // STATE_HPP