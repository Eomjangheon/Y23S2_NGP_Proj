#pragma once
#include <random>

// �ʱ� ��ġ
const float initialX[3] = { 0, -0.3, 0.3 };
const float initialY[3] = { 0.3, -0.3, -0.3 };

// �ӽ� �Լ�
void initClientInfoQueue();

shared_ptr<shared_ptr<LockQueue<Packet>>[]> InitializeInGameThread(BYTE* level);