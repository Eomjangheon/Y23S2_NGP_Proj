
#include "stdafx.h"
#include "InGameThread.h"

// ���� �õ带 ���� ���� ����
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(0, 15);


static void ToServerQueueCheck(vector<int> alivePlayer, array<EventQueues, NUM_OF_PLAYER>* eventQueues, array<Packet, NUM_OF_PLAYER>* playerPackets)
{
	for (auto player : alivePlayer)
	{
		(*eventQueues)[player].toServerEventQueue->TryPop((*playerPackets)[player]);
	}
}

static void PushWinPacket(EventQueues winnerQueues)
{
	Packet winPack;
	winnerQueues.toServerEventQueue->WaitPop(winPack);
	winPack.stateMask |= (1 << (int)STATE_MASK::RESULT);
	winnerQueues.toClientEventQueue->Push(winPack);
}

static void PushPacket(vector<int> alivePlayer, array<EventQueues, NUM_OF_PLAYER>* eventQueues, array<Packet, NUM_OF_PLAYER>* playerPackets)
{
	for (auto player : alivePlayer)
	{
		for (int i = 0; i < NUM_OF_PLAYER; ++i)
			(*eventQueues)[player].toClientEventQueue->Push((*playerPackets)[i]);
	}
}

static void PrintPacketData(array<Packet, NUM_OF_PLAYER> playerPackets)
{
	int num = 1;
	for (auto packet : playerPackets)
	{
		cout << "[PLAYER_" << num << "] stateMask: ";
		cout << bitset<8>(packet.stateMask);
		cout << endl;
		cout << "[PLAYER_" << num << "] x, y: " << packet.x << ", " << packet.y << endl;
		++num;
	}
}

// ������ �Լ��� ������ �ʱ�ȭ �Լ�
void InitializeInGameThread(GAME_LEVEL level, array<EventQueues, NUM_OF_PLAYER>* eventQueues, array<Packet, NUM_OF_PLAYER> *playerPackets, array<PlayerPosAcc, NUM_OF_PLAYER>* players)
{
	//ClientInfo clientInfo;
	int seed = dis(gen);

	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		// ClientInfoQueue���� Packet ������ pop
		//ClientInfoQueue[(int)level].WaitPop(clientInfo);
		// stateMask �ʱ�ȭ
		(*playerPackets)[i].stateMask = 0;
		// ���� ����
		(*playerPackets)[i].stateMask |= (1 << (int)STATE_MASK::GAME_START);
		// �÷��̾� ��ȣ
		(*playerPackets)[i].stateMask |= (i << (int)STATE_MASK::PLAYER_NUM);
		// �ʱ���ġ����, opengl�� ��ǥ��ȯ�� Client����
		(*playerPackets)[i].stateMask &= ~(1 << (int)STATE_MASK::POS_FLAG);
		(*playerPackets)[i].x = initialX[i];
		(*playerPackets)[i].y = initialY[i];
		(*players)[i].PosX = initialX[i];
		(*players)[i].PosY = initialY[i];
		// ���� �õ� ��
		(*playerPackets)[i].stateMask |= seed;
		// toClientEventQueue
		//clientInfo.toClientEventQueue = eventQueues[i].toClientEventQueue;
		//clientInfo.toServerEventQueue = eventQueues[i].toServerEventQueue;
		for (int j = 0; j < NUM_OF_PLAYER ; ++j)
			(*eventQueues)[i].toClientEventQueue->Push((*playerPackets)[j]);
	}
}

void InitPacket(array<Packet, NUM_OF_PLAYER>* playerPackets)
{
	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		(*playerPackets)[i].x = 0;
		(*playerPackets)[i].y = 0;
		(*playerPackets)[i].stateMask |= (1 << (int)STATE_MASK::GAME_START);
		(*playerPackets)[i].stateMask |= (1 << (int)STATE_MASK::POS_FLAG);
		(*playerPackets)[i].stateMask |= (3 << (int)STATE_MASK::LIFE);
		(*playerPackets)[i].stateMask |= (1 << (int)STATE_MASK::PLAYING);
		(*playerPackets)[i].stateMask &= ~(1 << (int)STATE_MASK::RESULT);
	}
}

void CaculateAcceleration(vector<int> alivePlayer, array<Packet, NUM_OF_PLAYER>* playerPackets, array<PlayerPosAcc, NUM_OF_PLAYER>* players)
{
	for (auto player : alivePlayer)
	{
		(*players)[player].AccX = (*playerPackets)[player].x;
		(*players)[player].VelX += (*players)[player].AccX;
		(*players)[player].PosX += (*players)[player].VelX;
		(*players)[player].AccY = (*playerPackets)[player].y;
		(*players)[player].VelY += (*players)[player].AccY;
		(*players)[player].PosY += (*players)[player].VelY;
	}
}

void InGameThread(GAME_LEVEL level, array<EventQueues, NUM_OF_PLAYER> eventQueues)
{
#ifdef _DEBUG_INGAME
	cout << "InGame Thread Start" << endl;
#endif // _DEBUG_INGAME

	bool timeReset = false;
	chrono::system_clock::time_point start;
	chrono::duration<double> time;

	array<Packet, NUM_OF_PLAYER> playerPackets;
	array<PlayerPosAcc, NUM_OF_PLAYER> players;
	memset(&playerPackets, 0, sizeof(Packet) * NUM_OF_PLAYER);

	// �÷��̾ ������ erase(player_num);
	vector<int>alivePlayer(NUM_OF_PLAYER);
	iota(alivePlayer.begin(), alivePlayer.end(), 0);

	InitializeInGameThread(level, &eventQueues, &playerPackets, &players);
#ifdef _DEBUG_INGAME
	cout << "��Ŷ������ Ȯ��" << endl;
	PrintPacketData(playerPackets);
#endif // _DEBUG_INGAME
	InitPacket(&playerPackets);
	while (true) {
		if (alivePlayer.size() == 0)
			break;
		if (alivePlayer.size() == 1) {
			PushWinPacket(eventQueues[alivePlayer.front()]);
			alivePlayer.clear();
			break;
		}
		ToServerQueueCheck(alivePlayer, &eventQueues, &playerPackets);
		// ����Ŭ������ �����˻�
		//Physics.Caculate(eventQueues)
		// ��� ������Ʈ
		// �浹 ����� �⵹���� push [0__1__10]
		// �÷��̾� ����� erase(player_num);
		// 1�� ���� �� winPlayer�� ���
		//PushPacket(eventQueues);
		// �׿� ���� ���ӵ� push
		PushPacket(alivePlayer, &eventQueues, &playerPackets);

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