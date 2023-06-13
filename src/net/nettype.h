#pragma once

enum NetMode
{
	NONE,
	TCP,
	UDP,
	KCP,
	MAX,
};

enum NetStatus
{
	Disconnected,
	Connecting,
	Connected,
	Disconnecting
};

enum
{
	BuffMax = 10240,
};