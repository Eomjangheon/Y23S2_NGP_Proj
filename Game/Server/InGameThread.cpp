#include "stdafx.h"
#include "InGameThread.h"

// ���� �õ带 ���� ���� ����
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(0, 15);

// ������ �Լ��� ������ �ʱ�ȭ �Լ�
// ���̵�� 1 ����ü �̿�
void InitializeInGameThread(GAME_LEVEL level, array<EventQueues, NUM_OF_PLAYER> eventQueues, array<Packet, NUM_OF_PLAYER> playerPackets)
{
	ClientInfo clientInfo;
	int seed = dis(gen);

	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		// ClientInfoQueue���� Packet ������ pop
		ClientInfoQueue[(int)level].WaitPop(clientInfo);
		// stateMask �ʱ�ȭ
		playerPackets[i].stateMask = 0;
		// ���� ����
		playerPackets[i].stateMask |= (1 << (int)STATE_MASK::GAME_START);
		// �÷��̾� ��ȣ
		playerPackets[i].stateMask |= (i << (int)STATE_MASK::PLAYER_NUM);
		// �ʱ���ġ����, opengl�� ��ǥ��ȯ�� Client����
		playerPackets[i].stateMask &= ~(1 << (int)STATE_MASK::POS_FLAG);
		playerPackets[i].x = initialX[i];
		playerPackets[i].y = initialY[i];
		// ���� �õ� ��
		playerPackets[i].stateMask |= seed;
		// toClientEventQueue
		clientInfo.toClientEventQueue = eventQueues[i].toClientEventQueue;
		clientInfo.toServerEventQueue = eventQueues[i].toServerEventQueue;
		eventQueues[i].toClientEventQueue->Push(playerPackets[i]);
		
	}
}

static void ToServerQueueCheck(vector<int> alivePlayer, array<EventQueues, NUM_OF_PLAYER> eventQueues, array<Packet, NUM_OF_PLAYER> playerPackets)
{
	for (auto player : alivePlayer)
	{
		eventQueues[player].toServerEventQueue->TryPop(playerPackets[player]);
	}
}

static void PushWinPacket(EventQueues winnerQueues)
{
	Packet winPack;
	winnerQueues.toServerEventQueue->WaitPop(winPack);
	winPack.stateMask |= (1 << (int)STATE_MASK::RESULT);
	winnerQueues.toClientEventQueue->Push(winPack);
}

static void PushPacket(vector<int> alivePlayer, array<EventQueues, NUM_OF_PLAYER> eventQueues, array<Packet, NUM_OF_PLAYER> playerPackets)
{
	for (auto player : alivePlayer)
	{
		eventQueues[player].toClientEventQueue->Push(playerPackets[player]);
	}
}

void InGameThread(GAME_LEVEL level)
{
	bool timeReset = false;
	chrono::system_clock::time_point start;
	chrono::duration<double> time;

	array<EventQueues, NUM_OF_PLAYER> eventQueues;
	array<Packet, NUM_OF_PLAYER> playerPackets;
	memset(&playerPackets, 0, sizeof(Packet) * NUM_OF_PLAYER);

	// �÷��̾ ������ erase(player_num);
	vector<int>alivePlayer(NUM_OF_PLAYER);
	iota(alivePlayer.begin(), alivePlayer.end(), 0);

	InitializeInGameThread(level, eventQueues, playerPackets);

	while (true) {
		if (alivePlayer.size() == 0)
			break;
		if (alivePlayer.size() == 1) {
			PushWinPacket(eventQueues[alivePlayer.front()]);
			alivePlayer.clear();
			break;
		}
		ToServerQueueCheck(alivePlayer, eventQueues, playerPackets);
		// ����Ŭ������ �����˻�
		//Physics.Caculate(eventQueues)
		// ��� ������Ʈ
		// �浹 ����� �⵹���� push [0__1__10]
		// �÷��̾� ����� erase(player_num);
		// 1�� ���� �� winPlayer�� ���
		//PushPacket(eventQueues);
		// �׿� ���� ���ӵ� push
		PushPacket(alivePlayer, eventQueues, playerPackets);

		if (timeReset == false) {
			timeReset = true;
			start = chrono::system_clock::now();
		}
		time = chrono::system_clock::now() - start;
		// ��ġ ����
		if (time.count() >= 0.3) {
			// ��ġ ���� ����� �������� push
			// ��ġ push���� Packet ���� [0__0__10]
			PushPacket(alivePlayer, eventQueues, playerPackets);
			timeReset == false;
		}
	}
}