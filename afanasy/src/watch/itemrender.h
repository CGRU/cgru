#pragma once

#include "../libafanasy/render.h"

#include "itemfarm.h"
#include "plotter.h"

class ListRenders;

class ItemRender : public ItemFarm
{
public:
	ItemRender( af::Render * i_render, ListRenders * i_list_renders, const CtrlSortFilter * i_ctrl_sf);
	~ItemRender();

	void v_updateValues(af::Node * i_afnode, int i_msgType);

	inline const QString & getPool() const { return m_pool; }

	inline const QString & getUserName()   const { return m_username;      }
	inline const QString & getIPString()   const { return m_address_ip_str;}
	inline int getCapacity() const { return m_capacity;  }
	inline int getMaxTasks() const { return m_maxtasks;  }

	void v_setSortType(   int i_type1, int i_type2 );
	void v_setFilterType( int i_type );

	inline bool isOnline()        const { return m_online;           }
	inline bool isOffline()       const { return false == m_online;  }
	inline bool isBusy()          const { return m_busy;             }
	inline bool isNimby()         const { return m_nimby;            }
	inline bool isNIMBY()         const { return m_NIMBY;            }
	inline bool isPaused()        const { return m_paused;           }
	inline bool isDirty()         const { return m_dirty;            }
	inline bool isWOLFalling()    const { return m_wolFalling;       }
	inline bool isWOLSleeping()   const { return m_wolSleeping;      }
	inline bool isWOLWaking()     const { return m_wolWaking;        }
	inline bool hasTasks()        const { return m_tasks.size() > 0; }
	
	inline std::list<const af::TaskExec*> getTasks() const
	{
		std::list<const af::TaskExec*> l;
		for (std::list<af::TaskExec*>::const_iterator it = m_tasks.begin() ; it != m_tasks.end() ; ++it)
			l.push_back(*it);
		return l;
	}

	bool calcHeight();

protected:
	virtual void v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const;

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
	ListRenders * m_ListRenders;

	// We need to keep two info strings,
	// because render can update its properties w/o resources at one time,
	// and at the other just resources.
	// So the result string should be a combination of a two strings
	// that are updating separately.
	QString m_info_text_render;
	QString m_info_text_hres;

	af::HostRes m_hres;

	QString m_pool;
	QString m_os;
	QString m_engine;
	QString m_username;
	QString m_tasksusers; ///< For sorting and filtering only
	QString m_tasks_users_counts; ///< One string collection of all tasks users and counts
	int m_capacity;
	int m_maxtasks;

	long long m_time_registered;
	long long m_time_launched;
	long long m_wol_operation_time;
	long long m_idle_time;
	long long m_busy_time;
	long long m_elder_task_time;

	QString m_address_ip_str;
	QString m_address_str;      ///< Client address

	bool m_online;
	bool m_busy;
	bool m_NIMBY;
	bool m_nimby;
	bool m_paused;
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
