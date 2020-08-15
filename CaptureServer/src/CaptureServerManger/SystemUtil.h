#ifndef _SYSTEMUTIL_H
#define _SYSTEMUTIL_H

#include <string>
using std::string;

	namespace SYSTEMUTIL
	{
		//ת��UTF8��ʽ
		string GetUtf8(const string & strContent);

		//UTF8ת����ANSI��ʽ
		string GetUcs2(const string & strContent);

		//ת����GB2312��ʽ
		string GetGB2312(const string & strContent);

		//��ȡ��ǰʱ���
		long long GetTimeStamp();

		//��ȡ��ǰ��ִ���ļ�����Ŀ¼
		string GetExeFileExistDir();

		//��ȡ����MAC��ַ
		string GetLocalMacAdress();

		//��ȡ��������
		string GetLocalHostName();

		//��ȡ����IP
		string GetLocalIP();

		//�������жϽ����Ƿ����
		bool IsProcessorExist(string strProcessorName);

		//������ID�жϽ����Ƿ����
		bool IsProcessorExist(int nProcessID);

		//����algounit
		int OpenProcessor(string strProcessorName,char * sParam,int nUnitID,int & nProcessorID,bool bIsShowWindow);

		//����������������
		void TerminateProcessor(string strProcessorName);

		//������ID��������
		void TerminateProcessor(int nProcessID);

		//�ݹ�ɾ���ļ���
		void DeleteDirection(string & strRootDir);

		//ɾ����Ŀ¼
		bool DeleteEmptyDirectoryEx(string pdir);

		//ɾ���ļ�
		void ForceDeleteFile(char * strFile);

		//����ɾ��N��֮ǰ���ļ�
		bool DeleteLocalStoreFile(string & strRoot, int nCircle);

		//����ָ�����ڵ��ļ�
		bool GetHistoryLocalFileInfo(string & strRoot,string & strFileName,int nBeforeDay,tm & tFileCreateTime);

		//��ȡ��ʷ�����ļ�
		bool GetHistoryAlarmFile(string & strRoot,string & strFileName);

		//��ȡ��ʷ����ͼƬ�ļ�
		bool GetHistoryPicFileInfo(string & strRoot,string & strFileName);

		//��ȡ�����ļ����б�
		bool GetFileDirListInfo(string & strRoot,string * pStrFileDirList, int nMaxListNumber, int & nFileDirNumber);

		//����ϵͳʱ��
		bool SetDeviceSystemTime(struct tm & tSynTime);

		//�����ӽ��̽�����Ϣ
		void HandleChildProcessSignal();

#if defined(_WIN32) || defined(_WIN64)
		#include <WTypes.h>
		//for windows special
		//�ļ�sʱ��תtime_t
		time_t FileTimeToTimet(FILETIME const& ft);

		//time_tת�ļ�ʱ��
		FILETIME TimetToFiletime(time_t const& tt);

#else
		//�ж��Ƿ�ΪĿ¼
		int IS_DIR(const char* path);
#endif

	}
#endif