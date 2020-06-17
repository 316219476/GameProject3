﻿#ifndef _GAME_SERVICE_H_
#define _GAME_SERVICE_H_
#include "CenterMsgHandler.h"

class CGameService : public IPacketDispatcher
{
private:
	CGameService(void);
	virtual ~CGameService(void);

public:
	static CGameService* GetInstancePtr();

	BOOL		Init();

	BOOL		Uninit();

	BOOL		Run();

	BOOL		OnNewConnect(UINT32 nConnID);

	BOOL		OnCloseConnect(UINT32 nConnID);

	BOOL		OnSecondTimer();

	BOOL		DispatchPacket( NetPacket* pNetPacket);

public:
	CCenterMsgHandler		m_CenterMsgHandler;

	//*********************消息处理定义开始******************************
	BOOL OnMsgGmStopServerReq(NetPacket* pNetPacket);
	//*********************消息处理定义结束******************************
};

#endif
