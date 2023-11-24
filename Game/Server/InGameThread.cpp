
#include "stdafx.h"
#include "InGameThread.h"

// ���� �õ带 ���� ���� ����
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(0, 15);

// ������ �Լ��� ������ �ʱ�ȭ �Լ�
static void InitializeInGameThread(GAME_LEVEL level, array<EventQueues, NUM_OF_PLAYER>* eventQueues, array<Packet, NUM_OF_PLAYER>* playerPackets, array<PlayerPosAcc, NUM_OF_PLAYER>* players)
{
	int seed = dis(gen);

	for (int i = 0; i < NUM_OF_PLAYER; ++i) {
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
		// �÷��̾� �ʱ� ��ġ ����
		(*players)[i].PosX = initialX[i];
		(*players)[i].PosY = initialY[i];
		// ���� �õ� ��
		(*playerPackets)[i].stateMask |= seed;
		(*eventQueues)[i].toClientEventQueue->Push((*playerPackets)[i]);
	}
	for (int i = 0; i < NUM_OF_PLAYER; ++i)
		for (int j = 0; j < NUM_OF_PLAYER; ++j)
			if (i != j)
				(*eventQueues)[i].toClientEventQueue->Push((*playerPackets)[j]);
}

// �ʱ� ���� ���� �� ��Ŷ �ʱ�ȭ
static void InitPacket(array<Packet, NUM_OF_PLAYER>* playerPackets)
{
	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		(*playerPackets)[i].x = 0;
		(*playerPackets)[i].y = 0;
		(*playerPackets)[i].stateMask &= ~(1 << (int)STATE_MASK::GAME_START);
		(*playerPackets)[i].stateMask |= (1 << (int)STATE_MASK::POS_FLAG);
		(*playerPackets)[i].stateMask |= (3 << (int)STATE_MASK::LIFE);
		(*playerPackets)[i].stateMask |= (1 << (int)STATE_MASK::PLAYING);
		(*playerPackets)[i].stateMask &= ~(1 << (int)STATE_MASK::RESULT);
	}
}

// ť���� ������ Pop
static bool ToServerQueueCheck(vector<int> alivePlayer, array<EventQueues, NUM_OF_PLAYER>* eventQueues, array<Packet, NUM_OF_PLAYER>* playerPackets)
{
	int dataNum = alivePlayer.size();
	Packet tmp;
	for (auto player : alivePlayer)
	{
		if ((*eventQueues)[player].toServerEventQueue->TryPop(tmp) == false)
			dataNum--;
		else
		{
			(*playerPackets)[player].stateMask &= ~(1 << (int)STATE_MASK::GAME_START);
			(*playerPackets)[player].stateMask |= (1 << (int)STATE_MASK::POS_FLAG);
			(*playerPackets)[player].stateMask |= (3 << (int)STATE_MASK::LIFE);
			(*playerPackets)[player].stateMask |= (1 << (int)STATE_MASK::PLAYING);
			(*playerPackets)[player].stateMask &= ~(1 << (int)STATE_MASK::RESULT);
			(*playerPackets)[player].x = tmp.x;
			(*playerPackets)[player].y = tmp.y;
		}
	}
	if (dataNum < 1)
		return false;
	return true;
}

// ���� ���� ���� üũ
static void CheckPlayerExitGame(vector<int>* alivePlayer, array<Packet, NUM_OF_PLAYER>* playerPackets)
{
	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		if ((*playerPackets)[i].x == numeric_limits<float>::infinity() &&
			(*playerPackets)[i].y == numeric_limits<float>::infinity())
		{
			(*playerPackets)[i].stateMask &= ~(1 << (int)STATE_MASK::PLAYING);
			(*playerPackets)[i].stateMask &= ~(3 << (int)STATE_MASK::LIFE);
			alivePlayer->erase(std::remove(alivePlayer->begin(), alivePlayer->end(), i));
			cout << "Player" << i + 1 << " exit!" <<  alivePlayer->size() << endl;
		}
	}
}

// �¸��� �¸� stateMask ��Ʈ ����ŷ �� ť�� Push
static void PushWinPacket(EventQueues *winnerQueues)
{
	Packet winPack;
	(*winnerQueues).toServerEventQueue->WaitPop(winPack);
	winPack.stateMask |= (1 << (int)STATE_MASK::RESULT);
	(*winnerQueues).toClientEventQueue->Push(winPack);
}

// ����� �׽�Ʈ�� print�Լ�
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

// ���ӵ� ���� ���
static void CaculateAcceleration(vector<int> alivePlayer, array<Packet, NUM_OF_PLAYER>* playerPackets, array<PlayerPosAcc, NUM_OF_PLAYER>* players)
{
	for (auto player : alivePlayer)
	{
		// �÷��̾� ������ ������ �ֱ�
		(*players)[player].AccX = (*playerPackets)[player].x;
		(*players)[player].AccY = (*playerPackets)[player].y;
		// ���� ���ӵ� ������ ���� ���� ���
		//(*players)[player].VelX += (*players)[player].AccX;
		//(*players)[player].PosX += (*players)[player].VelX;
		//(*players)[player].VelY += (*players)[player].AccY;
		//(*players)[player].PosY += (*players)[player].VelY;
		// ���� ���ӵ� ���� playerPacket�� �ֱ�
		//(*playerPackets)[player].stateMask |= (1 << (int)STATE_MASK::POS_FLAG);
		//(*playerPackets)[player].x = (*players)[player].AccX;
		//(*playerPackets)[player].y = (*players)[player].AccY;
	}
}

// ��ġ ���� ���
static void CaculatePosition(vector<int> alivePlayer, array<Packet, NUM_OF_PLAYER>* playerPackets, array<PlayerPosAcc, NUM_OF_PLAYER>* players)
{
	for (auto player : alivePlayer)
	{
		// ���� ��ġ ���� playerPacket�� �ֱ�
		(*playerPackets)[player].stateMask &= ~(1 << (int)STATE_MASK::POS_FLAG);
		(*playerPackets)[player].x = (*players)[player].PosX;
		(*playerPackets)[player].y = (*players)[player].PosY;
	}
}

// ť�� ������ Push
static void PushPacket(vector<int> alivePlayer, array<EventQueues, NUM_OF_PLAYER>* eventQueues, array<Packet, NUM_OF_PLAYER> playerPackets)
{
	for (auto player : alivePlayer)
	{
		for (int i = 0; i < NUM_OF_PLAYER; ++i)
			(*eventQueues)[player].toClientEventQueue->Push(playerPackets[i]);
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

	array<PlayerPosAcc, NUM_OF_PLAYER> players;
	array<Packet, NUM_OF_PLAYER> playerPackets;
	memset(&playerPackets, 0, sizeof(Packet) * NUM_OF_PLAYER);

	// �÷��̾ ������ erase(player_num);
	vector<int>alivePlayer(NUM_OF_PLAYER);
	iota(alivePlayer.begin(), alivePlayer.end(), 0);
	bool queueEmpty = false;
	bool collision = false;

	InitializeInGameThread(level, &eventQueues, &playerPackets, &players);
#ifdef _DEBUG_INGAME
	cout << "��Ŷ������ Ȯ��" << endl;
	PrintPacketData(playerPackets);
#endif // _DEBUG_INGAME
	InitPacket(&playerPackets);
#ifdef _DEBUG_INGAME
	cout << "�ʱ� ��Ŷ������ Ȯ��" << endl;
	PrintPacketData(playerPackets);
#endif // _DEBUG_INGAME
	while (true) {
		if (ToServerQueueCheck(alivePlayer, &eventQueues, &playerPackets) == false)
			queueEmpty = true;
		//CheckPlayerExitGame(&alivePlayer, &playerPackets);
		if (alivePlayer.size() == 0)
			break;
		if (alivePlayer.size() == 1) {
			PushWinPacket(&eventQueues[alivePlayer.front()]);
			alivePlayer.clear();
			break;
		}
		// ����Ŭ������ �����˻�
		//Physics.Caculate(eventQueues)
		// ��� ������Ʈ
		// �浹 ����� �⵹���� push [0__1__10]
		// �÷��̾� ����� erase(player_num);
		// 1�� ���� �� winPlayer�� ���
		//PushPacket(eventQueues);
		// �׿� ���� ���ӵ� push
		//CaculateAcceleration(alivePlayer, &playerPackets, &players);
		if (queueEmpty && !collision)
		{
			queueEmpty = false;
			continue;
		}
#ifdef _DEBUG_INGAME
		cout << "���ӵ� ��Ŷ������ Ȯ��" << endl;
		PrintPacketData(playerPackets);
#endif // _DEBUG_INGAME
		PushPacket(alivePlayer, &eventQueues, playerPackets);
		if (timeReset == false) {
			timeReset = true;
			start = chrono::system_clock::now();
		}
		time = chrono::system_clock::now() - start;
		// ��ġ ����
//		if (time.count() >= 0.3) {
//			// ��ġ ���� ����� �������� push
//			// ��ġ push���� Packet ���� [0__0__10]
//			CaculatePosition(alivePlayer, &playerPackets, &players);
//#ifdef _DEBUG_INGAME
//			cout << "��ġ ��Ŷ������ Ȯ��" << endl;
//			PrintPacketData(playerPackets);
//#endif // _DEBUG_INGAME
//			PushPacket(alivePlayer, &eventQueues, playerPackets);
//			timeReset == false;
//		}
	}
}