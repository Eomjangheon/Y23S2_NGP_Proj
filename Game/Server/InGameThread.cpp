#include "stdafx.h"
#include "InGameThread.h"

// ���� �õ带 ���� ���� ����
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(0, 15);

// �ӽ� ClientInfoQueue

//void initClientInfoQueue()
//{
//	ClientInfo a[3];
//	for (int i = 0; i < 3; ++i) {
//		shared_ptr<LockQueue<Packet>> p = make_shared<LockQueue<Packet>>();
//		a[i].level = GAME_LEVEL::EASY;
//		a[i].packetQueptr = p;
//		a[i].sock = 0;
//		ClientInfoQueue.Push(a[i]);
//	}
//}
// ---------------------------------------

// ������ �Լ��� ������ �ʱ�ȭ �Լ�
// ���̵�� 1 ����ü �̿�
bool InitializeInGameThread(GAME_LEVEL* level, EventQueues eventQueues[NUM_OF_PLAYER])
{
	ClientInfo clientInfo;
	Packet pack;
	int seed = dis(gen);

	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		// ClientInfoQueue���� Packet ������ pop
		if (!ClientInfoQueue.TryPop(clientInfo)) return false;
		// stateMask �ʱ�ȭ
		pack.stateMask = 0;
		// ���� ����
		pack.stateMask |= (1 << (int)STATE_MASK::GAME_START);
		// �÷��̾� ��ȣ
		pack.stateMask |= (i << (int)STATE_MASK::PLAYER_NUM);
		// �ʱ���ġ����, opengl�� ��ǥ��ȯ�� Client����
		pack.stateMask &= ~(1 << (int)STATE_MASK::POS_FLAG);
		pack.x = initialX[i];
		pack.y = initialY[i];
		// ���� �õ� ��
		pack.stateMask |= seed;
		// toClientEventQueue
		clientInfo.toClientEventQueue = eventQueues[i].toClientEventQueue;
		clientInfo.toServerEventQueue = eventQueues[i].toServerEventQueue;
		eventQueues[i].toClientEventQueue->Push(pack);
		
	}
	// ���̵� ��� ����?
	*level = clientInfo.level;
}


// ���̵��2 ����ü ���� �ּҰ��� ������ �Ҵ�
//void InitializeInGameThread(GAME_LEVEL* level, shared_ptr<shared_ptr<LockQueue<Packet>>[]> toSeverEventQueue,  
//	shared_ptr<shared_ptr<LockQueue<Packet>>[]> toClientEventQueue)


//void InGameThread(GAME_LEVEL *level)
//{
//	bool timeReset = false;
//	chrono::system_clock::time_point start;
//	chrono::duration<double> time;
//	EventQueues eventQueues[NUM_OF_PLAYER];
//	InitializeInGameThread(level, eventQueues);
//
//	while (true) {
//		if (numOfPlayer == 0)
//			break;
//		if (numOfPlater == 1) {
//			PushWinPacket();
//			break;
//		}
//		// ����Ŭ������ �����˻�
//		Physics.Caculate(eventQueues)
//		// ��� ������Ʈ
//		// �浹 ����� �⵹���� push [0__1__10]
//		PushPacket(eventQueues);
//		// �׿� ���� ���ӵ� push
//		PushPacket(eventQueues);
//
//		if (timeReset == false) {
//			timeReset = true;
//			start = chrono::system_clock::now();
//		}
//		time = chrono::system_clock::now() - start;
//		// ��ġ ����
//		if (time.count() >= 0.3) {
//			// ��ġ push [0__0__10]
//			PushPacket(eventQueues);
//			timeReset == false;
//		}
//	}
//}