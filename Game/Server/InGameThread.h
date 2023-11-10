#pragma once
#include <random>

// EventQueue���� ����ü ���̵�� 1
struct EventQueues {
	shared_ptr<LockQueue<Packet>> toServerEventQueue;
	shared_ptr<LockQueue<Packet>> toClientEventQueue;
};

// �ʱ� ��ġ
const float initialX[3] = { 0, -0.3, 0.3 };
const float initialY[3] = { 0.3, -0.3, -0.3 };

// �ӽ� �Լ�
//void initClientInfoQueue();

void InitializeInGameThread(GAME_LEVEL level, array<EventQueues, NUM_OF_PLAYER> eventQueues, array<Packet, NUM_OF_PLAYER> playerPackets);
void InGameThread(GAME_LEVEL level, array<EventQueues, NUM_OF_PLAYER> eventQueues);