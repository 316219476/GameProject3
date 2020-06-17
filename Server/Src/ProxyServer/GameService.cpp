﻿#include "stdafx.h"
#include "GameService.h"
#include "Position.h"
#include "../Message/Msg_Game.pb.h"
#include "../Message/Msg_RetCode.pb.h"
#include "../Message/Msg_ID.pb.h"
#include "WatcherClient.h"
CGameService::CGameService(void)
{
	m_dwLogicConnID		= 0;
}

CGameService::~CGameService(void)
{
}

CGameService* CGameService::GetInstancePtr()
{
	static CGameService _GameService;

	return &_GameService;
}

BOOL CGameService::Init()
{
	CommonFunc::SetCurrentWorkDir("");

	if(!CLog::GetInstancePtr()->Start("ProxyServer", "log"))
	{
		return FALSE;
	}

	CLog::GetInstancePtr()->LogError("---------服务器开始启动-----------");

	if(!CConfigFile::GetInstancePtr()->Load("servercfg.ini"))
	{
		CLog::GetInstancePtr()->LogError("配制文件加载失败!");
		return FALSE;
	}

	CLog::GetInstancePtr()->SetLogLevel(CConfigFile::GetInstancePtr()->GetIntValue("proxy_log_level"));

	UINT16 nPort = CConfigFile::GetInstancePtr()->GetRealNetPort("proxy_svr_port");
	if (nPort <= 0)
	{
		CLog::GetInstancePtr()->LogError("配制文件proxy_svr_port配制错误!");
		return FALSE;
	}

	INT32  nMaxConn = CConfigFile::GetInstancePtr()->GetIntValue("proxy_svr_max_con");
	if(!ServiceBase::GetInstancePtr()->StartNetwork(nPort, nMaxConn, this))
	{
		CLog::GetInstancePtr()->LogError("启动服务失败!");
		return FALSE;
	}

	ERROR_RETURN_FALSE(m_ProxyMsgHandler.Init(0));

	CLog::GetInstancePtr()->LogError("---------服务器启动成功!--------");

	return TRUE;
}

BOOL CGameService::OnNewConnect(UINT32 nConnID)
{
	m_ProxyMsgHandler.OnNewConnect(nConnID);

	CWatcherClient::GetInstancePtr()->OnNewConnect(nConnID);

	return TRUE;
}

BOOL CGameService::OnCloseConnect(UINT32 nConnID)
{
	if(nConnID == m_dwLogicConnID)
	{
		m_dwLogicConnID = 0;
	}

	m_ProxyMsgHandler.OnCloseConnect(nConnID);

	CWatcherClient::GetInstancePtr()->OnCloseConnect(nConnID);

	return TRUE;
}

BOOL CGameService::OnSecondTimer()
{
	ConnectToLogicSvr();

	CWatcherClient::GetInstancePtr()->OnSecondTimer();

	return TRUE;
}

BOOL CGameService::DispatchPacket(NetPacket* pNetPacket)
{
	if (CWatcherClient::GetInstancePtr()->DispatchPacket(pNetPacket))
	{
		return TRUE;
	}

	if (m_ProxyMsgHandler.DispatchPacket(pNetPacket))
	{
		return TRUE;
	}

	return FALSE;
}

UINT32 CGameService::GetLogicConnID()
{
	return m_dwLogicConnID;
}

BOOL CGameService::ConnectToLogicSvr()
{
	if (m_dwLogicConnID != 0)
	{
		return TRUE;
	}
	UINT32 nLogicPort = CConfigFile::GetInstancePtr()->GetRealNetPort("logic_svr_port");
	ERROR_RETURN_FALSE(nLogicPort > 0);
	std::string strLogicIp = CConfigFile::GetInstancePtr()->GetStringValue("logic_svr_ip");
	CConnection* pConn = ServiceBase::GetInstancePtr()->ConnectTo(strLogicIp, nLogicPort);
	ERROR_RETURN_FALSE(pConn != NULL);
	m_dwLogicConnID = pConn->GetConnectionID();
	return TRUE;
}

BOOL CGameService::Uninit()
{
	ServiceBase::GetInstancePtr()->StopNetwork();
	google::protobuf::ShutdownProtobufLibrary();

	return TRUE;
}

BOOL CGameService::Run()
{
	while(TRUE)
	{
		ServiceBase::GetInstancePtr()->Update();

		CommonFunc::Sleep(1);
	}

	return TRUE;
}

BOOL CGameService::OnMsgGmStopServerReq(NetPacket* pNetPacket)
{
	GmStopServerReq Req;
	Req.ParsePartialFromArray(pNetPacket->m_pDataBuffer->GetData(), pNetPacket->m_pDataBuffer->GetBodyLenth());
	return TRUE;
}


