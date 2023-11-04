#include "stdafx.h"
#include "InGameThread.h"

// ���� �õ带 ���� ���� ����
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(0, 15);

// �ӽ� ClientInfoQueue
LockQueue<ClientInfo> ClientInfoQueue;
void initClientInfoQueue()
{
	ClientInfo a[3];
	for (int i = 0; i < 3; ++i) {
		shared_ptr<LockQueue<Packet>> p = make_shared<LockQueue<Packet>>();
		a[i].level = (BYTE)GAME_LEVEL::EASY;
		a[i].packetQueptr = p;
		a[i].sock = 0;
		ClientInfoQueue.Push(a[i]);
	}
}
// ---------------------------------------

// ������ �Լ��� ������ �ʱ�ȭ �Լ�
shared_ptr<shared_ptr<LockQueue<Packet>>[]> InitializeInGameThread(BYTE* level)
{
	
	// �ּҸ� ���� �����峻 ������ �ʿ�
	shared_ptr<shared_ptr<LockQueue<Packet>>[]> tmpPtr(new shared_ptr<LockQueue<Packet>>[3]);
	ClientInfo info;
	Packet pack;
	int seed = dis(gen);

	for (int i = 0; i < 3; ++i) {
		// ���̵�
		// ClientInfoQueue���� Packet ������ pop
		ClientInfoQueue.TryPop(info);
		// ���� �̺κ��� �� ������ ������ �� �ϰ� ���� �����ʿ䰡 �ִ���
		//*sock = info.sock;
		// stateMask �ʱ�ȭ
		pack.stateMask = 0;
		// ���� ����
		pack.stateMask |= (1 << 7);
		// �÷��̾� ��ȣ
		pack.stateMask |= (i << 5);
		// �ʱ���ġ����, opengl�� ��ǥ��ȯ�� Client����
		pack.stateMask &= ~(1 << 4);
		pack.x = initialX[i];
		pack.y = initialY[i];
		// ���� �õ� ��
		pack.stateMask |= seed;
		// toClientEventQueue
		info.packetQueptr->Push(pack);
		tmpPtr[i] = info.packetQueptr;
	}
	// ���̵�
	*level = (BYTE)info.level;
	return tmpPtr;
}