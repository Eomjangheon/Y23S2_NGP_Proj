#include "stdafx.h"
#include "InGameThread.h"

// ���� �õ带 ���� ���� ����
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(0, 15);

double elapsedTime = 0.f;

Physics ps;
CollisionManager cm;

// ������ �Լ��� ������ �ʱ�ȭ �Լ�
static void InitializeInGameThread(GAME_LEVEL level, array<EventQueues, NUM_OF_PLAYER>* eventQueues, array<Packet, NUM_OF_PLAYER>* playerPackets, array<PlayerInfo, NUM_OF_PLAYER>* players)
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
		(*players)[i].Pos.x = initialX[i];
		(*players)[i].Pos.y = initialY[i];
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
static bool ToServerQueueCheck(vector<int> alivePlayer, array<EventQueues, NUM_OF_PLAYER>* eventQueues, array<Packet, NUM_OF_PLAYER>* playerPackets, array<PlayerInfo, NUM_OF_PLAYER>* players)
{
	int dataNum = alivePlayer.size();
	Packet tmp;
	for (auto player : alivePlayer)
	{
		if ((*eventQueues)[player].toServerEventQueue->TryPop(tmp) == true)
		{
			(*playerPackets)[player].x = tmp.x;
			(*playerPackets)[player].y = tmp.y;
			(*players)[player].Acc.x = tmp.x;
			(*players)[player].Acc.y = tmp.y;
		}
		else
		{
			dataNum--;
		}
	}
	if (dataNum < 1)
		return false;
	return true;
}

// ���� ���� ���� üũ
static void CheckPlayerExitGame(vector<int>* alivePlayer, array<Packet, NUM_OF_PLAYER>* playerPackets, array<PlayerInfo, NUM_OF_PLAYER>* players)
{
	for (int i = 0; i < NUM_OF_PLAYER; ++i)
	{
		if ((*playerPackets)[i].x == numeric_limits<float>::infinity() &&
			(*playerPackets)[i].y == numeric_limits<float>::infinity())
		{
			(*playerPackets)[i].stateMask &= ~(1 << (int)STATE_MASK::PLAYING);
			(*playerPackets)[i].stateMask &= ~(3 << (int)STATE_MASK::LIFE);
			(*playerPackets)[i].stateMask &= ~(1 << (int)STATE_MASK::POS_FLAG);
			(*players)[i].Pos.x = END_OF_X + 1.f;
			(*players)[i].Pos.y = END_OF_Y + 1.f;
			(*players)[i].Acc.x = 0;
			(*players)[i].Acc.y = 0;
			(*players)[i].Vel.x = 0;
			(*players)[i].Vel.y = 0;
			alivePlayer->erase(std::remove(alivePlayer->begin(), alivePlayer->end(), i), alivePlayer->end());
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

static void ModifyPacketVel(vector<int> alivePlayer, array<Packet, NUM_OF_PLAYER>* playerPackets, array<PlayerInfo, NUM_OF_PLAYER>* players)
{
	for (auto player : alivePlayer)
	{
		(*playerPackets)[player].stateMask |= (1 << (int)STATE_MASK::POS_FLAG);
		(*playerPackets)[player].x = (*players)[player].Vel.x;
		(*playerPackets)[player].y = (*players)[player].Vel.y;
	}
}

static void ModifyPacketPos(vector<int> alivePlayer, array<Packet, NUM_OF_PLAYER>* playerPackets, array<PlayerInfo, NUM_OF_PLAYER>* players)
{
	for (auto player : alivePlayer)
	{
		(*playerPackets)[player].stateMask &= ~(1 << (int)STATE_MASK::POS_FLAG);
		(*playerPackets)[player].x = (*players)[player].Pos.x;
		(*playerPackets)[player].y = (*players)[player].Pos.y;
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

	//bool timeReset = false;
	//chrono::system_clock::time_point start;
	//chrono::duration<double> time;
	std::chrono::system_clock::time_point prevTime = std::chrono::system_clock::now();

	array<PlayerInfo, NUM_OF_PLAYER> players;
	array<Packet, NUM_OF_PLAYER> playerPackets;
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
#ifdef _DEBUG_INGAME
	cout << "�ʱ� ��Ŷ������ Ȯ��" << endl;
	PrintPacketData(playerPackets);
#endif // _DEBUG_INGAME
	while (true) {
		auto now = std::chrono::system_clock::now();
		elapsedTime = static_cast<std::chrono::duration<double>>(now - prevTime).count();
		ToServerQueueCheck(alivePlayer, &eventQueues, &playerPackets, &players);
		//CheckPlayerExitGame(&alivePlayer, &playerPackets, &players);
		if (alivePlayer.size() == 0)
			break;
		if (alivePlayer.size() == 1) {
			PushWinPacket(&eventQueues[alivePlayer.front()]);
			alivePlayer.clear();
			break;
		}
		// �浹 üũ
		//for (int i = 0; i < NUM_OF_PLAYER; ++i) {
		//	if (i == 0 || i == 1)
		//	{
		//		if (cm.DoCollideAB(&players[i], &players[i + 1]))
		//			collision = true;
		//		if (i == 0) {
		//			if (cm.DoCollideAB(&players[i], &players[i + 2]))
		//				collision = true;
		//		}
		//	}
		//	if (cm.DoCollideWithWall(&players[i]))
		//		collision = true;
		//}
		// �ӵ� ���
		for (auto p : alivePlayer) {
			ps.CaculateVelocity(&players[p], elapsedTime);
		}
		// ��ġ ���
		for (auto p : alivePlayer) {
			ps.CaculatePosition(&players[p]);
		}
		//ModifyPacketPos(alivePlayer, &playerPackets, &players);
		if (elapsedTime >= 0.16667f) {
			ModifyPacketVel(alivePlayer, &playerPackets, &players);
#ifdef _DEBUG_INGAME
			cout << "�ӵ� ��Ŷ������ Ȯ��" << endl;
			PrintPacketData(playerPackets);
#endif // _DEBUG_INGAME
			PushPacket(alivePlayer, &eventQueues, playerPackets);
			//	// ��ġ ���� ����� �������� push
			//	// ��ġ push���� Packet ���� [0__0__10]
//#ifdef _DEBUG_INGAME
//			cout << "��ġ ��Ŷ������ Ȯ��" << endl;
//			PrintPacketData(playerPackets);
//#endif // _DEBUG_INGAME
//			PushPacket(alivePlayer, &eventQueues, playerPackets);
			prevTime = now;
		}
	}
}