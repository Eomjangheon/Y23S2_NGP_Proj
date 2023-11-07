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

bool InitializeInGameThread(GAME_LEVEL* level, EventQueues eventQueues[NUM_OF_PLAYER]);
