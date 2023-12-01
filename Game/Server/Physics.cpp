#include "stdafx.h"
#include "physics.h"

Physics::Physics() { };

Physics::~Physics() {};

void Physics::CaculateVelocity(PlayerInfo* players, float elapsedTime)
{
	vec2f tmpVel = players->Vel;
	players->Vel = players->Acc * elapsedTime + players->Vel;

	vec2f friction = players->Vel * -1;
	float mag = sqrtf(friction.x * friction.x + friction.y * friction.y);
	if (mag > FLT_EPSILON) {
		friction = friction / mag;
		friction = friction * COEF;
		cout << "[����]" << friction.x << ", " << friction.y << endl;
		cout << "[����]" << friction.x << ", " << friction.y << endl;
		vec2f resultVel = players->Vel + friction;
		if (resultVel.x * players->Vel.x < 0.f)
			players->Vel.x = 0.f;
		else
			players->Vel.x = resultVel.x;
		if (resultVel.y * players->Vel.y < 0.f)
			players->Vel.y = 0.f;
		else
			players->Vel.y = resultVel.y;
	}
	if (players->Vel.x > MAX_SPEED)
		players->Vel.x = MAX_SPEED;
	if (players->Vel.x < -MAX_SPEED)
		players->Vel.x = -MAX_SPEED;

	if (players->Vel.y > MAX_SPEED)
		players->Vel.y = MAX_SPEED;
	if (players->Vel.y < -MAX_SPEED)
		players->Vel.y = -MAX_SPEED;

	if (players->Vel.x == players->Vel.y || players->Vel.x * players->Vel.x + players->Vel.y * players->Vel.y > MAX_SPEED * MAX_SPEED)
	{
		if (players->Vel.x > MAX_SPEED / ROOT_TWO)
			players->Vel.x = MAX_SPEED / ROOT_TWO;
		if (players->Vel.x < -MAX_SPEED / ROOT_TWO)
			players->Vel.x = -MAX_SPEED / ROOT_TWO;

		if (players->Vel.y > MAX_SPEED / ROOT_TWO)
			players->Vel.y = MAX_SPEED / ROOT_TWO;
		if (players->Vel.y < -MAX_SPEED / ROOT_TWO)
			players->Vel.y = -MAX_SPEED / ROOT_TWO;
	}
	//cout <<"[�ӵ�]" << players->Vel.x << ", " << players->Vel.y << endl;
	//players->Acc = players->Vel - tmpVel;
}

void Physics::CaculatePosition(PlayerInfo* players)
{
	players->Pos = players->Pos + players->Vel;
}

void Physics::AfterColideWithWall(float* Acc, float* Vel, float diff)
{
	*Acc = -(*Vel * 2) - diff;
}

void Physics::AfterColideWithPlayer(PlayerInfo* A, PlayerInfo* B)
{
	vec2f unitVec = DecideUnitVec(A->Pos, B->Pos);
	vec2f diff = unitVec * (A->Radius + B->Radius) / 2;
	AfterForce(&A->Acc, &B->Acc, unitVec, A->Mass, B->Mass);
	A->Acc = A->Acc + diff;
	B->Acc = B->Acc - diff;
	//cout << AccA->x << ", " << AccA->y << endl;
}

vec2f Physics::DecideUnitVec(vec2f posA, vec2f posB)
{		//��ġ���� ��������
	float x = posA.x - posB.x;
	float y = posA.y - posB.y;

	float nor_x = x / sqrt(x * x + y * y);
	float nor_y = y / sqrt(x * x + y * y);
	return vec2f(nor_x, nor_y);
}

// ���� ź�� �� �� ��� ������ ��Ģ
vec2f Physics::AfterColisionVelocityA(vec2f velA, vec2f velB, vec2f unitVec, float massA, float massB)
{
	vec2f vel1 = unitVec * vec2f().dot(velA, unitVec);
	vec2f vel2 = unitVec * vec2f().dot(velB, unitVec);
	vec2f velSub = vel1 - vel2;
	vec2f J = (velSub * -1 * massB) / ((massA + massB) / 2);
	vec2f vel1After = vel1 + J;
	return vel1After;
}

vec2f Physics::AfterColisionVelocityB(vec2f velA, vec2f velB, vec2f unitVec, float massA, float massB)
{
	vec2f vel1 = unitVec * vec2f().dot(velA, unitVec);
	vec2f vel2 = unitVec * vec2f().dot(velB, unitVec);
	vec2f velSub = vel1 - vel2;
	vec2f J = (velSub * massA) / ((massA + massB) / 2);
	vec2f vel2After = vel2 + J;
	return vel2After;
}

// �ۿ� ���ۿ�
void Physics::AfterForce(vec2f* AccA, vec2f* AccB, vec2f unitVec, float massA, float massB)
{
	vec2f forceA = *AccA * massA;
	vec2f forceB = *AccB * massB;
	vec2f dirForceA = unitVec * vec2f().dot(forceA, unitVec);
	vec2f dirForceB = unitVec * vec2f().dot(forceB, unitVec);
	vec2f afterForce = dirForceA + dirForceB;
	*AccA = afterForce / massA;
	*AccB = (afterForce * -1) / massB;
}