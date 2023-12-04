#include "stdafx.h"
#include "ClientServerThread.h"
thread_local shared_ptr<LockQueue<Packet>> m_toServerEventQueue = make_shared<LockQueue<Packet>>();
thread_local shared_ptr<LockQueue<Packet>> m_toClientEventQueue = make_shared<LockQueue<Packet>>();;

void ClientServerThread(SOCKET client)
{
	Initialize(client);

	MainLoop(client);
}

void Initialize(SOCKET client)
{
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[24];

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
	printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));

	//���� 1ȸ ���̵� ����
	GAME_LEVEL level = GAME_LEVEL::NONE;
	int retval = recv(client, (char*)&level, sizeof(level), 0);
	switch (level) {
	case GAME_LEVEL::NONE:
		cout << "NONE" << endl;

		break;

	case GAME_LEVEL::EASY:
		cout << "EASY" << endl;
		break;

	case GAME_LEVEL::NORMAL:
		cout << "NORMAL" << endl;
		break;

	case GAME_LEVEL::HARD:
		cout << "HARD" << endl;
		break;

	}

	//���� 1ȸ TLS�� �̺�Ʈť�� 2�� ������
	//ClientInfoQueue�� �־��ش�.
	ClientInfo clientInfo;
	clientInfo.level = level;
	clientInfo.sock = client;
	clientInfo.toClientEventQueue = m_toClientEventQueue;
	clientInfo.toServerEventQueue = m_toServerEventQueue;

	ClientInfoQueue[(int)level].Push(clientInfo);

	DWORD optval = 10;
	retval = setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&optval, sizeof(optval));

	optval = true;
	setsockopt(client, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(optval));
}

void MainLoop(SOCKET client)
{
	int retval;
	while (true) {
		//�������� Ŭ���̾�Ʈ�� ���� ��Ŷ�� �ִ� ����
		//���� send
		{
			Packet packet;
			while (m_toClientEventQueue->TryPop(packet)) {
				retval = send(client, (char*)&packet, sizeof(packet), 0);
				//cout << retval;
				if (retval == SOCKET_ERROR) {
					//err_display("send()");
				}
			}
		}

		//recv() ���� ¥��
		{
			Packet packet;
			while (true) {
			#ifdef _DEBUG_CLIENT_SERVER
				cout << "recv Loop" << endl;
			#endif
				retval = recv(client, (char*)&packet, sizeof(packet), 0);
				if (retval > 0) {
					cout << packet.x << " " << packet.y << " " << packet.stateMask << endl;
					m_toServerEventQueue->Push(packet);
				}


				if (retval < 0) {
				#ifdef _DEBUG_CLIENT_SERVER
					cout << "TIMEOUT" << endl;
				#endif
					break;
	}
				if (retval == 0) {
					cout << "����" << endl;
					packet.stateMask = 0;
					packet.x = numeric_limits<float>::infinity();
					packet.y = numeric_limits<float>::infinity();
					m_toServerEventQueue->Push(packet);
					return;
				}
}
	}
		//���ۿ� ���� ��Ŷ�� ������ ��� ����ť�� Ǫ��


		//������, �ޱⰡ ���� �Ϸ�Ǹ� ������ �纸
		this_thread::yield();
}
	closesocket(client);
}
