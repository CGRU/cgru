#pragma once

#include "../libafanasy/common/dlRWLock.h"

#include "../libafanasy/msgqueue.h"
#include "../libafanasy/render.h"
#include "../libafanasy/renderupdate.h"

#include "taskprocess.h"

class Parser;
class PyRes;

class RenderHost: public af::Render
{
private:  // This is a singleton class
    RenderHost();

public:
    ~RenderHost();

    /// Get singleton instance
    static RenderHost * getInstance();
	
	/// Some getters and setters
	inline bool noOutputRedirection() { return m_no_output_redirection; }
	
	void connectionEstablished();

	inline bool isConnected()  const { return          m_connected; }
	inline bool notConnected() const { return false == m_connected; }

	/**
	* @brief Some message was failed to send.
	* At first it counts this function call.
	* If count > af_render_connectretries connection is lost.
	* @param i_any_case Do not count, connection os lost in any case.
	*/
	void connectionLost( bool i_any_case = false);

	/**
	* @brief Render was successfuly registered on server and got good (>0) id.
	* @param i_id Render ID from server to store.
	*/
	void setRegistered( int i_id);

	/**
	* @brief Task monitoring cycle, checking how task processes are doing
	*/
	void refreshTasks();

	/**
	* @brief Send message to server and receive answer
	*/
	af::Msg * updateServer();

	/**
	* @brief Get machine resources.
	* Custom resources also called there.
	*/
	void getResources();

	#ifdef WINNT
	/**
	* @brief Close windows on windows.
	*/
	void windowsMustDie();
	#endif

	/**
	* @brief Create new TaskProcess.
	* @param i_task Task data
	*/
	void runTask( af::TaskExec * i_task);

	/**
	* @brief Stop task process.
	* @param i_taskpos Index of the task
	*/
    void stopTask( const af::MCTaskPos & i_taskpos);

	/**
	* @brief Close (delete) class that controls child process.
	* @param i_taskpos Index of the task
	*/
    void closeTask( const af::MCTaskPos & i_taskpos);

	bool hasTasks() const { return m_taskprocesses.size(); }
	int getTasksCount() const { return m_taskprocesses.size(); }
	inline int getRunningTasksCount() const
		{ int c = 0; for( int i = 0; i < m_taskprocesses.size(); i++) if( m_taskprocesses[i]->isRunning()) c++; return c;}

	/**
	* @brief Add task update data to send to server on next update.
	* @param i_tup Task update data class
	*/
	inline void addTaskUp( af::MCTaskUp * i_tup) { m_up.addTaskUp( i_tup);}

	/**
	* @brief Write task output on next update.
	* This needed when you ask running task output from GUI.
	* @param i_taskpos Index of the task
	*/
	void upTaskOutput( const af::MCTaskPos & i_taskpos);

	/**
	* @brief Start (or stop) to send task output on each update.
	* @param i_tp Index of the task
	* @param i_subscribe Turn listening on or off
	*/
	void listenTask( const af::MCTaskPos & i_tp, bool i_subscribe);

	/**
	* @brief Close windows on windows.
	* @param i_str Render data for Python service class
	*/
	void wolSleep( const std::string & i_str);

private:
	/**
	* @brief Set update message type.
	* @param i_type New type to set.
	*/
	void setUpdateMsgType( int i_type);

private:
	/// Windows to kill on windows
	/// Bad mswin applications like to raise a gui window with an error and waits for some 'Ok' button.
    std::vector<std::string> m_windowsmustdie;

	/// Custom resources classes
    std::vector<PyRes*> m_pyres;

	/// Whether the render is connected or not
    bool m_connected;
	/// Count times render failed to send update message to server
	int  m_connection_lost_count;

	/// Heartbeat message to sent at each update.
	/// It is initially a `TRenderRegister` and as soon as the server
	/// registered the render, it becomes a `TRenderUpdate`.
    int m_updateMsgType;

	/// List of task processed being currently ran by the render
    std::vector<TaskProcess*> m_taskprocesses;

	/// Whether the task outputs must be redirected. Used essentially by TaskProcess
	bool m_no_output_redirection;

	/// Class to collect data to send to server on update.
	af::RenderUpdate m_up;

	/// Time when render has at least on task:
	time_t m_has_tasks_time;
};
