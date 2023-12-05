﻿#pragma once
#include "easyStage.h"
#include "player.h"
#include "light.h"
#include "gameWorld.h"
#include "camera.h"
#include "wall.h"
#include "particle.h"
#include "bg.h"
#include "PacketManager.h"

extern Player player;
extern Light light;
extern GameWorld gameWorld;
extern Camera camera;
extern GLuint shaderID;
extern Object* playerPtr;
extern Wall wall;
extern BG backGround;

short seed;
std::mt19937 g_gen;
std::uniform_int_distribution<int> pattern(0, 2);

void EasyStage::init()
{
	cout << "easy Stage" << endl;

	const unsigned short indexSize = 120;
	walls.reserve(indexSize);

	Wall* temp = new Wall(100);
	temp->initBuffer();
	temp->initTexture();
	gameWorld.add_object(temp);
	walls.emplace_back(temp);

	for (int i = 0; i < indexSize - 1; i++) {
		temp = new Wall(100);
		gameWorld.add_object(temp);
		walls.emplace_back(temp);
	}
	patternTime = 1000;

	light.setAmbientLight(0.7);

	backGround.initBuffer();
	backGround.initTexture();
	gameWorld.add_object(&backGround);

	player.initBuffer();
	player.initTexture();
	gameWorld.add_object(playerPtr);

	for (int i = 0; i < 3; i++) {
		//Packet* packet = new Packet();
		//g_PacketManager->RecvPacket(packet);
		Packet* packet = &g_PacketManager->m_initPacket[i];

		seed = packet->stateMask & 0b1111;
		packet->stateMask = packet->stateMask >> 4;

		bool isPos = packet->stateMask & 1;
		packet->stateMask = packet->stateMask >> 1;

		short playerNum = packet->stateMask & 0b11;
		packet->stateMask = packet->stateMask >> 2;

		bool isInit = packet->stateMask & 1;

		cout << packet->x << ", " << packet->y << endl;

		float accX = packet->x;
		float accY = packet->y;

		cout << seed << " " << isPos << " " << playerNum << " " << isInit <<" " << accX<<" "<<accY<< endl;

		if (i == 0) {
			player.setPlayerNum(playerNum);
			player.setPos(glm::vec3(accX, accY, 0.7));
			Player* other = &player;
			otherPlayers.emplace_back(other);
		}
		else {
			Player* other = new Player();
			other->initBuffer();
			other->setPos(glm::vec3(accX, accY, 0.7));
			other->setPlayerNum(playerNum);
			other->initTexture("res/Rock.png");
			gameWorld.add_object(other);
			otherPlayers.emplace_back(other);
		}

	}
	g_PacketManager->SetSocketOpt();

	for (int i = 0; i < 20; ++i)
	{
		Particle* tempP = new Particle(true);
		tempP->initBuffer();
		gameWorld.add_object(tempP);
	}
}
void EasyStage::update()
{
	gameWorld.update_all();
	//light.update(); // ����
	timer++;
	patternTime += g_elapsedTime;


	// Camera rolling test

	Packet* packet = new Packet();
	while (g_PacketManager->RecvPacket(packet)) {
		//short seed = packet->stateMask & 15;
		//cout << "recv packet in stage" << endl;
		bool isWin = packet->stateMask & 1;
		packet->stateMask = packet->stateMask >> 1;

		bool isInGame = packet->stateMask & 1;
		packet->stateMask = packet->stateMask >> 1;

		short life = packet->stateMask & 0b11;
		packet->stateMask = packet->stateMask >> 2;

		bool isAcc = packet->stateMask & 1;
		packet->stateMask = packet->stateMask >> 1;

		short playerNum = packet->stateMask & 0b11;
		packet->stateMask = packet->stateMask >> 2;

		bool isInit = packet->stateMask & 1;

		float accX = packet->x;
		float accY = packet->y;

		if (life == 0) {
			for (auto& p : otherPlayers) {
				if (p->getPlayerNum() == playerNum) {
					cout << "collllllllll" << endl;
					cout << playerNum << endl;
					//p->collision();
					continue;
				}
			}
		}

		if (isAcc) {
			for (auto& p : otherPlayers) {
				if (p->getPlayerNum() == playerNum) {
					//cout << "id:" << playerNum<<" accX: "<< accX<<" accY: " << accY << endl;
					p->setSpeed(glm::vec3(accX, accY, 0));

				}
			}

		}
		else {
			for (auto& p : otherPlayers) {
				if (p->getPlayerNum() == playerNum) {
					//cout << accX << " " << accY << endl;
					glm::vec3 temp = p->getPos();
					if (abs(temp.x - accX) > 0.03 || abs(temp.y - accY) > 0.03) {
						p->setPos(glm::vec3(accX, accY, 0.7));
					}

				}
			}
		}

	}

	if (patternTime > 5) {
		makePattern(pattern(g_gen));
		patternTime = 0;
	}

	glutPostRedisplay();
}
void EasyStage::handleEvent(unsigned char key, bool isDown)
{
	if (isDown)
	{
		switch (key)
		{
		case'c':
			makePattern(1);
			//player.setProtectedMode(true);
			break;
		case 'a':

			player.setMoveLeft(true);
			break;

		case 'd':
			player.setMoveRight(true);
			break;

		case 's':
			player.setMoveDown(true);
			break;

		case 'w':
			player.setMoveUp(true);
			break;

		case 'Q':
		case 'q':
			exit(0);
			break;
		}
	}
	else if (!isDown)
	{
		switch (key)
		{

		case 'w':
			player.setMoveUp(false);
			break;

		case 'a':
			player.setMoveLeft(false);
			break;

		case 's':
			player.setMoveDown(false);
			break;

		case 'd':
			player.setMoveRight(false);
			break;
		}
	}
}
void EasyStage::draw()
{
	camera.setCamera(shaderID, 0); // 0 = �������� / 1 = ��������
	light.setLight(shaderID, camera.getEye());
	gameWorld.draw_all();
}
void EasyStage::out()
{
	g_PacketManager->Reset();
	patternTime = 0;
	cout << "Out Easy Stage" << endl;
}