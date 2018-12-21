#include "StdAfx.h"
#include "GameService.h"

//////////////////////////////////////////////////////////////////////////
//�����Ϣ
static char g_filename[128];
void TraceMessageScore(LPCTSTR pszMessage)
{
	if(0 == lstrlen(pszMessage))
	{
		return ;
	}
	
	SYSTEMTIME sys; 
	GetLocalTime( &sys ); 
	char buf[128];
	memset(buf, 0, 128);
	sprintf(buf, "%4d/%02d/%02d %02d:%02d:%02d.%03d ",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds);

	CFile File;
	if ((File.Open(g_filename,CFile::modeWrite)==FALSE)&&
		(File.Open(g_filename,CFile::modeWrite|CFile::modeCreate)==FALSE))
	{
		ASSERT(FALSE);
		return;
	}

	File.SeekToEnd();
	File.Write(buf,lstrlen(buf));
	File.Write(pszMessage,lstrlen(pszMessage));
	File.Write(TEXT("\r\n"),lstrlen(TEXT("\r\n")));

	File.Flush();
	File.Close();

	return;
}

//���캯��
CGameService::CGameService()
{
	m_szModule[0]=0;
	m_hDllInstance=NULL;
	m_pIGameServiceManager=NULL;

	m_ServiceStatus=ServiceStatus_Stop;

	memset(&m_GameUserDBInfo,0,sizeof(m_GameUserDBInfo));
//	memset(&m_GameScoreDBInfo,0,sizeof(m_GameScoreDBInfo));
	m_GameScoreDBInfoVec.clear();
	memset(&m_GameServiceAttrib,0,sizeof(m_GameServiceAttrib));
	memset(&m_GameServiceOption,0,sizeof(m_GameServiceOption));
}

//��������
CGameService::~CGameService()
{
	UnLoadGameServiceModule();
	return;
}

//�ӿڲ�ѯ
void * __cdecl CGameService::QueryInterface(const IID & Guid, DWORD dwQueryVer)
{
	QUERYINTERFACE(IGameService,Guid,dwQueryVer);
	QUERYINTERFACE_IUNKNOWNEX(IGameService,Guid,dwQueryVer);
	return NULL;
}

//��ʼ����
bool __cdecl CGameService::StartService(tagGameServiceParameter * pGameServiceParameter)
{
	//Ч�����
	ASSERT(pGameServiceParameter!=NULL);
	if (pGameServiceParameter==NULL) return false;

	//ֹͣ����
	ASSERT(m_ServiceStatus!=ServiceStatus_Service);
	if (m_ServiceStatus==ServiceStatus_Service) return false;
	
	//��ֹ����
	StopService();

	//���ñ���
	bool bSuccess=false;
	lstrcpyn(m_szModule,pGameServiceParameter->szModule,CountArray(m_szModule));
	CopyMemory(&m_GameUserDBInfo,&pGameServiceParameter->GameUserDBInfo,sizeof(m_GameUserDBInfo));
//	CopyMemory(&m_GameScoreDBInfo,&pGameServiceParameter->GameScoreDBInfo,sizeof(m_GameScoreDBInfo));
	CopyMemory(&m_GameServiceOption,&pGameServiceParameter->GameServiceOption,sizeof(m_GameServiceOption));
	m_GameScoreDBInfoVec.clear();
	for (int i=0; i<pGameServiceParameter->wTreasureDBCount; i++)
	{
		m_GameScoreDBInfoVec.push_back(pGameServiceParameter->GameScoreDBInfo[i]);
	}

	//�������
	if ((m_TimerEngine.GetInterface()==NULL)&&(m_TimerEngine.CreateInstance()==false)) return false;
	if ((m_DataBaseEngine.GetInterface()==NULL)&&(m_DataBaseEngine.CreateInstance()==false)) return false;
	if ((m_AttemperEngine.GetInterface()==NULL)&&(m_AttemperEngine.CreateInstance()==false)) return false;
	if ((m_TCPNetworkEngine.GetInterface()==NULL)&&(m_TCPNetworkEngine.CreateInstance()==false)) return false;
	if ((m_TCPSocketCorrespond.GetInterface()==NULL)&&(m_TCPSocketCorrespond.CreateInstance()==false)) return false;

	//����ģ��
	if (LoadGameServiceModule(m_szModule)==false)
	{
		ShowErrorMessasge(TEXT("��Ϸ����ģ���������ʧ��"),TraceLevel_Exception);
		return false;
	}

	//��������
	WORD wLessConnect=m_GameServiceOption.wTableCount*m_GameServiceAttrib.wChairCount;
	m_GameServiceOption.wMaxConnect=__max(m_GameServiceOption.wMaxConnect,wLessConnect+LESS_CONNECT_COUNT);

	//����ӿ�
	IUnknownEx * pIAttemperEngine=m_AttemperEngine.GetInterface();
	IUnknownEx * pITCPNetworkEngine=m_TCPNetworkEngine.GetInterface();

	//�ص��ӿ�
	IUnknownEx * pIDataBaseEngineSink=QUERY_OBJECT_INTERFACE(m_DataBaseEngineSink,IUnknownEx);
	IUnknownEx * pIAttemperEngineSink=QUERY_OBJECT_INTERFACE(m_AttemperEngineSink,IUnknownEx);

	//�󶨽ӿ�
	if (m_AttemperEngine->SetAttemperEngineSink(pIAttemperEngineSink)==false) return false;
	if (m_DataBaseEngine->SetDataBaseEngineSink(pIDataBaseEngineSink)==false) return false;

	//�ں����
	if (m_TimerEngine->SetTimerEngineEvent(pIAttemperEngine)==false) return false;
	if (m_AttemperEngine->SetNetworkEngine(pITCPNetworkEngine)==false) return false;

	m_AttemperEngineSink.m_pITCPSocketCorrespondVec.clear();
	for (size_t i=0; i<pGameServiceParameter->wCenterCount; i++)
	{
		m_AttemperEngineSink.m_pITCPSocketCorrespondVec.push_back(m_TCPSocketCorrespond.GetNewInterface());
		if((*m_AttemperEngineSink.m_pITCPSocketCorrespondVec.rbegin())->SetTCPSocketEvent(pIAttemperEngine)==false) return false;
		(*m_AttemperEngineSink.m_pITCPSocketCorrespondVec.rbegin())->SetServiceID(i);  //��ʶ�����ĸ�������
	}

	if (m_TCPNetworkEngine->SetTCPNetworkEngineEvent(pIAttemperEngine)==false) return false;

	//���Ȼص�
	m_AttemperEngineSink.m_pGameServiceAttrib=&m_GameServiceAttrib;
	m_AttemperEngineSink.m_pGameServiceOption=&m_GameServiceOption;
	m_AttemperEngineSink.m_pIGameServiceManager=m_pIGameServiceManager;
	for (int i=0; i<MAX_CENTER_SERVER; i++)
	{
		m_AttemperEngineSink.m_dwCenterServer[i]=pGameServiceParameter->dwCenterAddr[i];
		m_AttemperEngineSink.m_wCenterPort[i]=pGameServiceParameter->wCenterPort[i];
	}

	//��������
	m_AttemperEngineSink.m_pITimerEngine=m_TimerEngine.GetInterface();
	m_AttemperEngineSink.m_pIAttemperEngine=m_AttemperEngine.GetInterface();
	m_AttemperEngineSink.m_pIDataBaseEngine=m_DataBaseEngine.GetInterface();
	m_AttemperEngineSink.m_pITCPNetworkEngine=m_TCPNetworkEngine.GetInterface();
	

	//���ݿ�ص�
	m_DataBaseEngineSink.m_pGameUserDBInfo=&m_GameUserDBInfo;
	m_DataBaseEngineSink.m_pGameScoreDBInfoVec=&m_GameScoreDBInfoVec;
	m_DataBaseEngineSink.m_pGameServiceAttrib=&m_GameServiceAttrib;
	m_DataBaseEngineSink.m_pGameServiceOption=&m_GameServiceOption;
	m_DataBaseEngineSink.m_pIDataBaseEngineEvent=QUERY_OBJECT_PTR_INTERFACE(pIAttemperEngine,IDataBaseEngineEvent);

	//��������
	WORD wMaxConnect=m_GameServiceOption.wMaxConnect;
	WORD wServicePort=m_GameServiceOption.wServerPort;
	if (m_TCPNetworkEngine->SetServiceParameter(wServicePort,wMaxConnect)==false) return false;

	//ʱ������
	if (m_TimerEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//��������
	for (size_t i=0; i<pGameServiceParameter->wCenterCount; i++)
	{
		if (m_AttemperEngineSink.m_pITCPSocketCorrespondVec[i]->StartService()==false)
		{
			ASSERT(FALSE);
			return false;
		}
	}

	//��������
	if (m_DataBaseEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//��������
	if (m_AttemperEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//��������
	if (m_TCPNetworkEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//���ñ���
	m_ServiceStatus=ServiceStatus_Service;

	//�����־
	memset(g_filename, 0, 128);
	sprintf(g_filename, "ScoreLoser_%d.txt", m_GameServiceOption.wServerID);

	return true;
}

//ֹͣ����
bool __cdecl CGameService::StopService()
{
	//���ñ���
	m_ServiceStatus=ServiceStatus_Stop;
	
	for (size_t i=0; i<m_AttemperEngineSink.m_pITCPSocketCorrespondVec.size(); i++)
	{
		(*m_AttemperEngineSink.m_pITCPSocketCorrespondVec.rbegin())->ConcludeService();
	}

	//ֹͣ����
	if (m_TimerEngine.GetInterface()!=NULL) m_TimerEngine->ConcludeService();
	if (m_AttemperEngine.GetInterface()!=NULL) m_AttemperEngine->ConcludeService();
	if (m_DataBaseEngine.GetInterface()!=NULL) m_DataBaseEngine->ConcludeService();
	if (m_TCPNetworkEngine.GetInterface()!=NULL) m_TCPNetworkEngine->ConcludeService();	

	//ж��ģ��
	UnLoadGameServiceModule();

	return true;
}

//�ͷ�ģ��
bool CGameService::UnLoadGameServiceModule()
{
	try
	{
		if (m_pIGameServiceManager!=NULL)
		{
			m_pIGameServiceManager->Release();
			m_pIGameServiceManager=NULL;
		}
		if (m_hDllInstance!=NULL)
		{
			AfxFreeLibrary(m_hDllInstance);
			m_hDllInstance=NULL;
		}

		return true;
	}
	catch (...) {}

	return false;
}

//����ģ��
bool CGameService::LoadGameServiceModule(LPCTSTR pszModuleName)
{
	//Ч�����
	ASSERT(pszModuleName!=NULL);
	ASSERT(m_hDllInstance==NULL);

	try
	{
		//����ģ��
		m_hDllInstance=AfxLoadLibrary(pszModuleName);
		if (m_hDllInstance==NULL) 
		{
			TCHAR szDescribe[256]=TEXT("");
			_snprintf(szDescribe,sizeof(szDescribe),TEXT("��%s��ģ�����ʧ��"),pszModuleName);
			ShowErrorMessasge(szDescribe,TraceLevel_Exception);
			throw 0;
		}
		ModuleCreateProc * CreateFunc=(ModuleCreateProc *)GetProcAddress(m_hDllInstance,TEXT("CreateGameServiceManager"));
		if (CreateFunc==NULL) 
		{
			TCHAR szDescribe[256]=TEXT("");
			_snprintf(szDescribe,sizeof(szDescribe),TEXT("��%s����������������"),pszModuleName);
			ShowErrorMessasge(szDescribe,TraceLevel_Exception);
			throw 0;
		}
		m_pIGameServiceManager=(IGameServiceManager *)CreateFunc(IID_IGameServiceManager,VER_IGameServiceManager);
		if (m_pIGameServiceManager==NULL) 
		{
			TCHAR szDescribe[256]=TEXT("");
			_snprintf(szDescribe,sizeof(szDescribe),TEXT("��%s����Ϸ�����ӿڴ���ʧ��"),pszModuleName);
			ShowErrorMessasge(szDescribe,TraceLevel_Exception);
			throw 0;
		}

		//��ȡ����
		m_pIGameServiceManager->GetGameServiceAttrib(m_GameServiceAttrib);

		return true;
	}
	catch (...)	{ UnLoadGameServiceModule(); }

	return false;
}

//��ʾ����
void CGameService::ShowErrorMessasge(LPCTSTR pszString, enTraceLevel TraceLevel)
{
	CTraceService::TraceString(pszString,TraceLevel);
	return;
}

//////////////////////////////////////////////////////////////////////////

//����������
extern "C" __declspec(dllexport) void * __cdecl CreateGameService(const GUID & Guid, DWORD dwInterfaceVer)
{
	//��������
	CGameService * pGameService=NULL;
	try
	{
		pGameService=new CGameService();
		if (pGameService==NULL) throw TEXT("����ʧ��");
		void * pObject=pGameService->QueryInterface(Guid,dwInterfaceVer);
		if (pObject==NULL) throw TEXT("�ӿڲ�ѯʧ��");
		return pObject;
	}
	catch (...) {}

	//��������
	SafeDelete(pGameService);
	return NULL;
}

//////////////////////////////////////////////////////////////////////////