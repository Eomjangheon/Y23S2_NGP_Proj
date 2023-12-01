#pragma once
#include <memory>
#include "PacketManager.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...

PacketManager::PacketManager()
{
}

PacketManager::~PacketManager()
{
	// ���� �ݱ�
	closesocket(m_sock);
	cout << "close Sock" << endl;
	// ���� ����
	WSACleanup();
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ���� �Լ� ���� ���
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[����] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void PacketManager::Initialize(GAME_LEVEL level)
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		err_quit("Client Socket Init Error");
	}

	// ���� ����
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET) {
		err_quit("socket()");
	}

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, m_serverIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(m_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		err_quit("connect()");
	}

	//
	/*DWORD optval = 10;
	retval = setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&optval, sizeof(optval));*/

	//���� ���̵� ������ send
	retval = send(m_sock, (char*)&level, sizeof(level), 0);
	cout << retval;
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}
}

void PacketManager::Reset()
{
	Packet packet;
	packet.stateMask = 0;
	send(m_sock, (char*)&packet, sizeof(packet), 0);
	cout << "���� ����" << endl;
	// ���� �ݱ�
	closesocket(m_sock);
	cout << "close Sock" << endl;
	// ���� ����
	WSACleanup();
}
#include<bitset>
void PacketManager::SendPacket(BYTE flag, float x, float y)
{
	//Ű �Է½� �� �Լ��� ȣ���ؼ� ������ send�� ����
	Packet packet;

	packet.stateMask = flag;
	packet.x = x;
	packet.y = y;
	cout << "sendPacket x:" << x << " y:" << y << " bit:";
	cout << bitset<8>(packet.stateMask);
	int retval = send(m_sock, (char*)&packet, sizeof(Packet), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
	}

}

bool PacketManager::RecvPacket(Packet* packet)
{

	int retval = recv(m_sock, (char*)packet, sizeof(Packet), 0);
	//cout << "retval:"<< retval << endl;
	//if (retval == SOCKET_ERROR) {
	//	err_display("recv()");
	//}
	if (retval < 0) {
		return false;
	}
	return true;

}

void PacketManager::SetSocketOpt()
{
	DWORD optval = 10;
	int retval = setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&optval, sizeof(optval));
}

void PacketManager::SetIPAddress(char* ip)
{
	m_serverIP = ip;
}

shared_ptr<queue<Packet>> PacketManager::GetPacketQueue()
{
	return m_toClientEventQue;
}

