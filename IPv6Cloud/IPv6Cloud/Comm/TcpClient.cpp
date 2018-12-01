#include "StdAfx.h"
#include "TcpClient.h"

DWORD WINAPI ThreadRecConnect(void *lParam)
{
	CTcpClient *pThis = (CTcpClient *)lParam;
	if (!pThis) return NULL;
	
	while (pThis->m_bWork)
	{
		if(!pThis->m_bConnected)
		{
			if(connect(pThis->m_sCltSocket, pThis->m_addRes->ai_addr, pThis->m_addRes->ai_addrlen) == 0)
			{
				pThis->m_bConnected = true;
			}
		}
		Sleep(RECONNECT_TIME);
	}

	OutputDebugString("ThreadRecConnect ���������߳��˳�\n");

	return NULL;
}

DWORD WINAPI ThreadRecv(void *lParam)
{
	CTcpClient *pThis = (CTcpClient *)lParam;
	if (!pThis) return NULL;

	while (pThis->m_bWork)
	{
		if(pThis->m_bConnected)
		{
			StTcpData struStTcpData;
			memset(&struStTcpData, 0, sizeof(StTcpData));
			struStTcpData.lsize = recv(pThis->m_sCltSocket, struStTcpData.buffer, RECV_LENGTH_LARGE, 0);;
			if (struStTcpData.lsize > 0)
			{
				struStTcpData.buffer[struStTcpData.lsize] = 0;
				pThis->m_QueueRecv.Set(&struStTcpData);
				pThis->m_RecvEvent.Set(1);
			}
			else if(struStTcpData.lsize < 0)
			{
				pThis->m_bConnected = false;
			}
		}

		// �ȴ������߳��˳���Ϣ
		if (pThis->m_ThreadExitEvent.TryWait())
		{
			break;
		}
	}
	OutputDebugString("ThreadRecv �����߳��˳�\n");

	return NULL;
}

DWORD WINAPI ThreadSend(void *lParam)
{
	CTcpClient *pThis = (CTcpClient *)lParam;
	if (!pThis) return NULL;
	
	return NULL;
	while (pThis->m_bWork)
	{
		// �ȴ�����״̬���
		pThis->m_SendEvent.Wait();

		// �ȴ������߳��˳���Ϣ
		if (pThis->m_ThreadExitEvent.TryWait())
		{
			break;
		}

		// ȡ���ݲ�����
		StTcpData struStTcpData;
		if (pThis->m_QueueSend.Get(&struStTcpData))
		{
			send(pThis->m_sCltSocket, struStTcpData.buffer, strlen(struStTcpData.buffer), 0);
		}
	}

	OutputDebugString("CStTcpData �����߳��˳�\n");
	return NULL;
}

CTcpClient::CTcpClient(void)
{
	m_sCltSocket = -1;

	m_threadreconnect = INVALID_HANDLE_VALUE;
	m_threadsend = INVALID_HANDLE_VALUE;
	m_threadrecv = INVALID_HANDLE_VALUE;

	if ((m_sCltSocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		//AfxMessageBox("�ͻ��˴���socketʧ�ܣ�");
    }
	
	memset(&m_addHints, 0, sizeof(m_addHints));
	m_addHints.ai_family=AF_INET6;
	m_addHints.ai_socktype=SOCK_STREAM;
	m_addHints.ai_protocol=IPPROTO_TCP;
	m_addHints.ai_flags=AI_PASSIVE;
	
	m_addRes = NULL;

		//addrinfo hints;
		//addrinfo* res = NULL;
		//memset(&hints,0,sizeof(hints));
		//hints.ai_family=AF_INET6;
		//hints.ai_socktype=SOCK_STREAM;
		//hints.ai_protocol=IPPROTO_TCP;
		//hints.ai_flags=AI_PASSIVE;
		//reVel = getaddrinfo("::1","3000",&hints,&res);

	m_bConnected = false;
	m_bWork = false;
}

CTcpClient::~CTcpClient(void)
{
	m_bWork = false;

	if (m_sCltSocket >= 0)
	{
		// ֹͣ����
		shutdown(m_sCltSocket, 2);//SD_BOTH

		// �ȴ����ͻ��洦�����
		while (m_QueueSend.Count() > 0)
		{
			Sleep(100);
		}

		// �ر�socket
		closesocket(m_sCltSocket);

		// ֪ͨ�����߳̽���
		m_ThreadExitEvent.Set(2);
		m_SendEvent.Set(1);
		m_RecvEvent.Set(1);

		// �ȴ������߳̽���
		if(m_threadreconnect != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(m_threadreconnect, INFINITE);
			CloseHandle(m_threadreconnect);
			m_threadreconnect = INVALID_HANDLE_VALUE;
		}

		// �ȴ������߳̽���
		if(m_threadrecv != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(m_threadrecv, INFINITE);
			CloseHandle(m_threadrecv);
			m_threadrecv = INVALID_HANDLE_VALUE;
		}

		// �ȴ������߳̽���
		if(m_threadsend != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(m_threadsend, INFINITE);
			CloseHandle(m_threadsend);
			m_threadsend = INVALID_HANDLE_VALUE;
		}

		// ���socket��Ч
		m_sCltSocket = -1;
	}

	OutputDebugString("TcpClient �˳�\n");
}

bool CTcpClient::Start(char *pNodeName, char * pServiceName)
{
	if(m_bConnected)
	{
		//AfxMessageBox("�ͻ���ͨ��ģ���Ѿ�������");
		return false;
	}
	int nRet = getaddrinfo(pNodeName, pServiceName, &m_addHints, &m_addRes);
	if(nRet == 0)
	{
		m_bWork = true;

		// ���������߳�
		DWORD dwThreadID = 0;
		m_threadreconnect = CreateThread(NULL, 0, ThreadRecConnect, (void*)this, 0, &dwThreadID);
		if (m_threadreconnect == INVALID_HANDLE_VALUE)
		{
			closesocket(m_sCltSocket);
			m_sCltSocket = -1;
			return false;
		}

		// �����߳�
		m_threadrecv = CreateThread(NULL, 0, ThreadRecv, (void*)this, 0, &dwThreadID);
		if (m_threadrecv == INVALID_HANDLE_VALUE)
		{
			closesocket(m_sCltSocket);
			m_sCltSocket = -1;
			return false;
		}

		// �����߳�
		m_threadsend = CreateThread(NULL, 0, ThreadSend, (void*)this, 0, &dwThreadID);
		if (m_threadsend == INVALID_HANDLE_VALUE)
		{
			closesocket(m_sCltSocket);
			m_sCltSocket = -1;
			return false;
		}
	}
	return m_bWork;
}

long CTcpClient::Send(char *pBuffer, long lSize)
{
	if (!pBuffer || lSize > RECV_LENGTH_LARGE)
	{
		return -1;
	}

	struct StTcpData struData;
	struData.lsize = lSize;
	strcpy(struData.buffer, pBuffer);
	m_QueueSend.Set(&struData);
	m_SendEvent.Set(1);

	return 0;
}