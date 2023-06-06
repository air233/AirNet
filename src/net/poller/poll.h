#pragma once
#include "basepoll.h"
#include <memory>

class Poll
{
public:
	Poll();
	~Poll();

private:
	std::shared_ptr<BasePoll> m_base_poll;
};


