HOWTO
=====

This document is a cheat sheet where to quickly log useful development
information.


Adding a new property to job object
-----------------------------------

Note: I wrote this guide while adding the `project` attribute to jobs, so
assuming that it did not get other use since then, you may simply follow the
`af::Job::m_project` object among the code.

Everything starts of course in libafanasy/job.h. If the "of course" strikes you,
try to get an overview of how afanasy is organized first. Basically libafanasy
regroups the objects shared between server, renders and monitors.


1. Add a private attribute to `af::Job` (conventionally prefixed with `m_`), as
well as accessors:

    class Job : public Node
    {
    public:
    ...
        inline const std::string & getProject() const { return m_project; }
        inline void setProject( const std::string & project) { m_project = project; }
    ...
    protected:
    ...
        std::string m_project;
    ...
    };

Note that if the property will be set only from the Python API, and so through
JSON, you do not really need to implement a setter.


2. Add the attribute to I/O methods, namely binary and JSON read/write (in
libafanasy/job.cpp):

    bool Job::jsonRead(...)
    {
        ...
        jr_string("project", m_project, i_object);
        ...
    }

    void Job::v_jsonWrite(...)
    {
        ...
        if( m_project.size())
            o_str << ",\n\"project\":\""  << af::strEscape( m_project  ) << "\"";
        ...
    }

    void Job::v_readwrite(...)
    {
        ...
        rw_String ( m_project, msg);
        ...
    }

    // Note: I'm not sure what this is for, but it seems to be for variable
    // size properties only
    int Job::v_calcWeight() const
    {
        ...
        // if custom type, you must implement a weight() method
        weight += weigh( m_project);
        ...
    }


3. Optional: You can add the property to the generateInfoStream method if you
want this information to be displayed when printing the object:

    void Job::generateInfoStreamJob(...) const
    {
        ...
        if( m_project.size()) o_str << "\n    " << m_project;
        ...
    }


4. Now, let's go to the Python API. The Python API is not a binding but a
front-end generating JSON messages then sent to the server. The JSON data is
stored into the Job's `data` attribute (a dict). So, in afanasy/python/af.py:

    class Job:
        ...
        def setProject(self, project):
            """Set the name of the project to which this job is related.
            
            :param project: Name of the project
            """
            if project is not None and len(project):
                self.data["project"] = project

Providing some docstring is nice, though currently all entries just say
"Missing DocString"...


5. Now the use will depend of what you want to do with this new property, but a
thing that you are likely to want to do is to display this property in the
monitor.

If you want the property to appear with all other job properties, you can just
edit `ItemJob::updateValues()`, in watch/itemjob.cpp:

    void ItemJob::updateValues(...)
    {
        ...
        properties += QString(" proj:%1").arg( job->getProject());
        ...
    }

If you want to customize a bit more how to draw this information, you may be
interested in editing `ItemJob::paint()`. In this case, you need to store the
property in the `ItemJob` in `updateValues()` first:

    class ItemJob
    {
    public: // or private? I would say private but many are public.
        ...
        QString project;
        ...
    };

    void ItemJob::updateValues(...)
    {
        ...
        project = afqt::stoq( job->getProject());
        ...
    }

    void ItemJob::paint(...)
    {
        ...
        painter->drawText( x+30, y, w-40-rect_user.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, project);
        ...
    }


Adding a new property to render object
--------------------------------------

There are of course a lot of similarities with adding a property to a job. I
will only detail the differences. First, replace "job" by "render" in file paths
and class names.

Adding the property, the getter (and maybe setter) does not change. The I/O
methods are quite the same as well.

Actually, the main think to check is whether the property you want to add really
beyond to `Render` or if it wouldn't be more appropriate to put it in `Host` or
`HostRes`. The former are for static properties, which do not change often or
are set through `farm.json`, while the latter is for dynamic properties,
regularly refreshed.

In `Host`, don't forget to add your property to `mergeParameters(...)`.

In `HostRes`, don't forget to add your property to `copy(...)`.
