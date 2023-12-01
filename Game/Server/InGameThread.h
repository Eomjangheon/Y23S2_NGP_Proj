#pragma once
#include <random>
#include "CollisionManager.h"

// EventQueue���� ����ü ���̵�� 1
struct EventQueues {
	shared_ptr<LockQueue<Packet>> toServerEventQueue;
	shared_ptr<LockQueue<Packet>> toClientEventQueue;
};

// �ʱ� ��ġ
const float initialX[3] = { 0.f, -0.3f, 0.3f };
const float initialY[3] = { 0.3f, -0.3f, -0.3f };

// �ӽ� �Լ�
//void initClientInfoQueue();

void InGameThread(GAME_LEVEL level, array<EventQueues, NUM_OF_PLAYER> eventQueues);