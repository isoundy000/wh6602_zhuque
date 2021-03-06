#ifndef ATTEMPER_ENGINE_SINK_HEAD_FILE
#define ATTEMPER_ENGINE_SINK_HEAD_FILE

#pragma once

#include "Stdafx.h"
#include "Commonality.h"
#include "ServerList.h"
//#include "InitParamter.h"
#include "ServerParameter.h"
#include <map>
#include <string>

typedef struct tagMachineCode
{
	tagMachineCode()
	{
		memset(this,  0, sizeof(*this));
	}

	void SetVal(std::string strAccount)
	{
		OverdueTime = CTime::GetCurrentTime().GetTime();
		this->strAccount = strAccount;
	}

	// 删除比较
	bool compare(LPCTSTR lpStrAccount)
	{
		return (0 == this->strAccount.compare(lpStrAccount)) ? true : false;
	}

	// 进入比较
	bool compare(LPCTSTR lpStrAccount, DWORD dwSecond)
	{
		if (0 == this->strAccount.compare(lpStrAccount))
		{
			OverdueTime = CTime::GetCurrentTime().GetTime();
			return false;
		}
		else
		{
			// 保存的机器码有时间期
			if (OverdueTime+dwSecond <= CTime::GetCurrentTime().GetTime())
			{
				OverdueTime = CTime::GetCurrentTime().GetTime();
				this->strAccount = lpStrAccount;
				return false;
			}
			return true;
		}
	}

	__time64_t OverdueTime;
	std::string strAccount;
}MachineData;

//////////////////////////////////////////////////////////////////////////

typedef std::map<std::string, MachineData *> MMachine;

//绑定参数
struct tagBindParameter
{
	DWORD							dwSocketID;							//网络标识
	DWORD							dwClientIP;							//连接地址
	DWORD							dwActiveTime;						//激活时间
};

//////////////////////////////////////////////////////////////////////////

//调度引擎钩子
class CAttemperEngineSink : public IAttemperEngineSink
{
	//友元定义
	friend class CLogonService;

	//变量定义
protected:
	//CInitParamter *					m_pInitParamter;					//配置参数
	CServerParameter *              m_pInitParamter1;
	tagBindParameter *				m_pBindParameter;					//连接信息

	//辅助组件
protected:
	CServerList						m_ServerList;						//服务器列表

	//接口变量
protected:
	ITimerEngine *					m_pITimerEngine;					//时间引擎
	IDataBaseEngine *				m_pIDataBaseEngine;					//数据引擎
	ITCPNetworkEngine *				m_pITCPNetworkEngine;				//网络引擎
	ITCPSocketService *				m_pITCPSocketCorrespond;			//协调服务
    CMD_GP_UserGradeInfo*           m_pGradeInfo;                       //等级信息
	int                             m_iGradeNum;                        //等级数目
	MMachine						m_hmMachineCode;					//登陆玩家机器码集
	//函数定义
public:
	//构造函数
	CAttemperEngineSink();
	//析构函数
	virtual ~CAttemperEngineSink();

	//基础接口
public:
	//释放对象
	virtual void __cdecl Release() { return; }
	//接口查询
	virtual void * __cdecl QueryInterface(const IID & Guid, DWORD dwQueryVer);

	//异步接口
public:
	//启动事件
	virtual bool __cdecl OnAttemperEngineStart(IUnknownEx * pIUnknownEx);
	//停止事件
	virtual bool __cdecl OnAttemperEngineStop(IUnknownEx * pIUnknownEx);

	//事件接口
public:
	//控制事件
	virtual bool __cdecl OnEventControl(WORD wIdentifier, VOID * pData, WORD wDataSize) { return true; }
	//自定事件
	virtual bool __cdecl OnEventAttemperData(WORD wRequestID, VOID * pData, WORD wDataSize) { return true; }

	//内核事件
public:
	//时间事件
	virtual bool __cdecl OnEventTimer(DWORD dwTimerID, WPARAM wBindParam);
	//数据库事件
	virtual bool __cdecl OnEventDataBase(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize);

	//连接事件
public:
	//连接事件
	virtual bool __cdecl OnEventTCPSocketLink(WORD wServiceID, INT nErrorCode);
	//关闭事件
	virtual bool __cdecl OnEventTCPSocketShut(WORD wServiceID, BYTE cbShutReason);
	//读取事件
	virtual bool __cdecl OnEventTCPSocketRead(WORD wServiceID, CMD_Command Command, VOID * pData, WORD wDataSize);

	//网络事件
public:
	//应答事件
	virtual bool __cdecl OnEventTCPNetworkBind(DWORD dwClientIP, DWORD dwSocketID);
	//关闭事件
	virtual bool __cdecl OnEventTCPNetworkShut(DWORD dwClientIP, DWORD dwActiveTime, DWORD dwSocketID);
	//读取事件
	virtual bool __cdecl OnEventTCPNetworkRead(CMD_Command Command, VOID * pData, WORD wDataSize, DWORD dwSocketID);

	//数据库函数
private:
	//用户登录失败
	bool OnDBLogonError(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//用户登录成功
	bool OnDBLogonSuccess(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//用户更新成功
	bool OnUpdateUserInfoSucc(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//用户更新失败
	bool OnUpdateUserInfoFail(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//用户公告 彭文添加
	//bool OnDBLogonPlacard(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//下载成功
	bool OnDBDownloadFaceSuccess(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//成功消息
	bool OnDBRCustomFaceUploadResult(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//删除结果
	bool OnDBRCustomFaceDeleteResult(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//修改结果
	bool OnDBRModifyIndividual(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//等级数目
	bool OnDBRGradeInfoNum(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//一条等级信息
	bool OnDBRGradeInfoOne(DWORD dwContextID, VOID * pData, WORD wDataSize);
    //返回免费领取的结果
	bool OnDBRGiftMoneyResult(DWORD dwContextID, VOID * pData, WORD wDataSize);
	//网络函数
private:
	//登录消息处理
	bool OnSocketMainLogon(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);
	//用户信息处理
	bool OnSocketMainUser(WORD wSubCmdID, VOID * pData, WORD wDataSize, DWORD dwSocketID);

	//中心网络
private:
	//列表消息处理
	bool OnCenterMainServerList(WORD wSubCmdID, VOID * pData, WORD wDataSize);
};

//////////////////////////////////////////////////////////////////////////

#endif