#pragma once

#include "../libafanasy/render.h"

#include "itemnode.h"
#include "plotter.h"

class ItemRender : public ItemNode
{
public:
	ItemRender( af::Render *render);
	~ItemRender();

	void updateValues( af::Node *node, int type);

    inline const QString & getUserName()   const { return m_username;      }
    inline const QString & getAnnotation() const { return m_annotation;    }
    inline const QString & getIPString()   const { return m_address_ip_str;}
    inline int getPriority() const { return m_priority;  }
    inline int getCapacity() const { return m_capacity;  }
    inline int getMaxTasks() const { return m_maxtasks;  }

	bool setSortType(   int type );
	bool setFilterType( int type );

    inline bool isOnline()        const { return m_online;           }
    inline bool isOffline()       const { return false == m_online;  }
    inline bool isBusy()          const { return m_busy;             }
    inline bool isNimby()         const { return m_nimby;            }
    inline bool isNIMBY()         const { return m_NIMBY;            }
    inline bool isDirty()         const { return m_dirty;            }
    inline bool isWOLFalling()    const { return m_wolFalling;       }
    inline bool isWOLSleeping()   const { return m_wolSleeping;      }
    inline bool isWOLWaking()     const { return m_wolWaking;        }
    inline bool hasTasks()        const { return m_tasks.size() > 0; }

	bool calcHeight();

protected:
    virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:
    static const int ms_HeightHost;
    static const int ms_HeightHostSmall;
    static const int ms_HeightAnnotation;
    static const int ms_HeightTask;
    static const int ms_HeightOffline;

private:
	void deleteTasks();
	void deletePlots();

private:
	std::string m_tooltip_base;
    std::string m_tooltip_resources;

    af::Host    m_host;
    af::HostRes m_hres;

    QString m_version;
    QString m_username;
    QString m_tasksusers; ///< For sorting and filtering only
    QString m_tasks_users_counts; ///< One string collection of all tasks users and counts
    QString m_annotation;
    int m_priority;
    int m_capacity;
    int m_maxtasks;

    long long m_time_registered;
    long long m_time_launched;
    long long m_wol_operation_time;
    long long m_idle_time;
    long long m_busy_time;

    QString m_address_ip_str;
    QString m_address_str;      ///< Client address

    bool m_online;
    bool m_busy;
    bool m_NIMBY;
    bool m_nimby;
    bool m_dirty;
    bool m_wolFalling;
    bool m_wolSleeping;
    bool m_wolWaking;

    int m_capacity_used;
    QString m_capacity_usage;

	QString  m_state;
    std::list<af::TaskExec*> m_tasks;
	std::vector<int32_t> m_tasks_percents;
    std::list<const QPixmap*> m_tasksicons;
    long long m_taskstartfinishtime;
    QString m_taskstartfinishtime_str;
    QString m_offlineState;
    QString m_creationTime;

    Plotter m_plotCpu;
    Plotter m_plotMem;
    Plotter m_plotSwp;
    Plotter m_plotHDD;
    Plotter m_plotNet;
    Plotter m_plotIO;
    int m_plotIO_rn_r, m_plotIO_rn_g, m_plotIO_rn_b;
    int m_plotIO_wn_r, m_plotIO_wn_g, m_plotIO_wn_b;
    int m_plotIO_rh_r, m_plotIO_rh_g, m_plotIO_rh_b;
    int m_plotIO_wh_r, m_plotIO_wh_g, m_plotIO_wh_b;

    std::vector<Plotter*> m_plots;

    int m_plots_height;

    int m_update_counter;
};
