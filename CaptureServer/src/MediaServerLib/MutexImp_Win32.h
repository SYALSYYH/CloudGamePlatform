#ifndef MUTEX_IMP_WIN32_H
#define MUTEX_IMP_WIN32_H

#include <Windows.h>
#include <string>
using std::string;

namespace CS
{
	/****************************************************************************
	*
	* CCritSecImp
	*		wait: ���޵ȴ�
	****************************************************************************/
	class CCritSecImp
	{
	public:
		CCritSecImp();
		~CCritSecImp();

	public:
		void Lock();
		void Unlock();

	private:
		CRITICAL_SECTION    m_CritSec;
	};

	/****************************************************************************
	*
	* CMutexImp
	*		wait: ���޵ȴ�
	****************************************************************************/
	class CMutexImp
	{
	public :
		CMutexImp(char* lpName = NULL, bool bInitialOwner = false);
		~CMutexImp( void );

	public:
		void Lock( void );
		void Unlock( void );
		const string& GetName();

	private:
		HANDLE  m_mutex;
		string m_strName;
	};

	/****************************************************************************
	*
	* CSemaphoreImp
	*		wait: ���޵ȴ�
	*		release: ֻ�ͷ�1��
	****************************************************************************/
	class CSemaphoreImp
	{
	public:
		CSemaphoreImp(char* lpName, int nMaxNum = 1, int nInitNum = 1);
		~CSemaphoreImp( void );

	public:
		bool Wait(int timeout = INFINITE);
		void Release( void );
		const string& GetName();

	private:
		HANDLE m_semaphore;
		string m_strName;
	};

	/****************************************************************************
	*
	* CEventImp
	*		default: �ֶ� ������
	****************************************************************************/
	class CEventImp
	{
	public:
		CEventImp(bool bManualReset = false);
		virtual ~CEventImp();

	public:
		bool Set(void);
		bool Wait(unsigned long ulTimeout);
		bool Reset(void);

	protected:
		HANDLE  m_hEvent;
	};
}

#endif
