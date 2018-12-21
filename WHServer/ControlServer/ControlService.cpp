#include "StdAfx.h"
#include "ControlService.h"

//////////////////////////////////////////////////////////////////////////
CInitParamter					g_InitParamter;						//���ò���

//���캯��
CControlService::CControlService()
{
}

//��������
CControlService::~CControlService()
{
}

//��ʼ����
bool CControlService::StartService()
{
	//ֹͣ����
	ASSERT(m_ServiceStatus!=ServiceStatus_Service);
	if (m_ServiceStatus==ServiceStatus_Service) return false;
	
	//��ֹ����
	StopService();

	//���ز���
//	m_InitParamter.LoadInitParamter();

	//�������
	if ((m_TimerEngine.GetInterface()==NULL)&&(m_TimerEngine.CreateInstance()==false)) return false;
	if ((m_DataBaseEngine.GetInterface()==NULL)&&(m_DataBaseEngine.CreateInstance()==false)) return false;
	if ((m_AttemperEngine.GetInterface()==NULL)&&(m_AttemperEngine.CreateInstance()==false)) return false;
//	if ((m_TCPNetworkEngine.GetInterface()==NULL)&&(m_TCPNetworkEngine.CreateInstance()==false)) return false;
	if ((m_TCPSocketCorrespond.GetInterface()==NULL)&&(m_TCPSocketCorrespond.CreateInstance()==false)) return false;

	//����ӿ�
	IUnknownEx * pIAttemperEngine=m_AttemperEngine.GetInterface();
//	IUnknownEx * pITCPNetworkEngine=m_TCPNetworkEngine.GetInterface();

	//�ص��ӿ�
	IUnknownEx * pIDataBaseEngineSink=QUERY_OBJECT_INTERFACE(m_DataBaseEngineSink,IUnknownEx);
	IUnknownEx * pIAttemperEngineSink=QUERY_OBJECT_INTERFACE(m_AttemperEngineSink,IUnknownEx);

	//�󶨽ӿ�
	if (m_AttemperEngine->SetAttemperEngineSink(pIAttemperEngineSink)==false) return false;
	if (m_DataBaseEngine->SetDataBaseEngineSink(pIDataBaseEngineSink)==false) return false;

	//�ں����
	if (m_TimerEngine->SetTimerEngineEvent(pIAttemperEngine)==false) return false;
//	if (m_AttemperEngine->SetNetworkEngine(pITCPNetworkEngine)==false) return false;
	if (m_TCPSocketCorrespond->SetTCPSocketEvent(pIAttemperEngine)==false) return false;
//	if (m_TCPNetworkEngine->SetTCPNetworkEngineEvent(pIAttemperEngine)==false) return false;

	//���Ȼص�
	m_AttemperEngineSink.m_pInitParamter=&g_InitParamter;
	m_AttemperEngineSink.m_pITimerEngine=m_TimerEngine.GetInterface();
	m_AttemperEngineSink.m_pIDataBaseEngine=m_DataBaseEngine.GetInterface();
//	m_AttemperEngineSink.m_pITCPNetworkEngine=m_TCPNetworkEngine.GetInterface();
	m_AttemperEngineSink.m_pITCPSocketCorrespond=m_TCPSocketCorrespond.GetInterface();

	//���ݿ�ص�
	m_DataBaseEngineSink.m_pInitParamter=&g_InitParamter;
	m_DataBaseEngineSink.m_pIDataBaseEngineEvent=QUERY_OBJECT_PTR_INTERFACE(pIAttemperEngine,IDataBaseEngineEvent);

	//��������
	WORD wMaxConnect=g_InitParamter.m_wMaxConnect;
	WORD wServicePort=g_InitParamter.m_wListenPort;
	//Flag��Ϊ1��ʾ��Ϣ���ü���
//	if (m_TCPNetworkEngine->SetServiceParameter(wServicePort,wMaxConnect,1)==false) return false;

	//ʱ������
	if (m_TimerEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//��������
	if (m_TCPSocketCorrespond->StartService()==false)
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
	if (m_DataBaseEngine->StartService()==false)
	{
		ASSERT(FALSE);
		return false;
	}

	//��������
//	if (m_TCPNetworkEngine->StartService()==false)
//	{
//		ASSERT(FALSE);
//		return false;
//	}

	//���ñ���
	m_ServiceStatus=ServiceStatus_Service;

	return true;
}

//ֹͣ����
bool CControlService::StopService()
{
	//���ñ���
	m_ServiceStatus=ServiceStatus_Stop;

	//ֹͣ����
	if (m_TimerEngine.GetInterface()!=NULL) m_TimerEngine->ConcludeService();
	if (m_AttemperEngine.GetInterface()!=NULL) m_AttemperEngine->ConcludeService();
	if (m_DataBaseEngine.GetInterface()!=NULL) m_DataBaseEngine->ConcludeService();
//	if (m_TCPNetworkEngine.GetInterface()!=NULL) m_TCPNetworkEngine->ConcludeService();
	if (m_TCPSocketCorrespond.GetInterface()!=NULL) m_TCPSocketCorrespond->ConcludeService();

	return true;
}

//�·�����
bool CControlService::SendCommand(VOID * pData, WORD wDataSize)
{
	if (m_AttemperEngine.GetInterface()!=NULL)
	{
		return m_TCPSocketCorrespond->SendData(MDM_CS_CONTROL_COMMAND,SUB_CS_CONTROL_COMMAND_REQ,pData, wDataSize);
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////