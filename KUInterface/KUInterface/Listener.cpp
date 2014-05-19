#include <stdafx.h>
#include "Listener.h"

void BaseNotifier::SendNotify(const void* pData)
{
	for (vector<shared_ptr<IReceiver>>::iterator itr = m_ReceiverList.begin();
		itr != m_ReceiverList.end(); ++itr)
	{
		if (*itr)
			(*itr)->OnEvent(pData);
	}
}

void BaseNotifier::AddReceiver(IReceiver* pReceiver)
{
	if (!pReceiver)
		return;
	m_ReceiverList.push_back(shared_ptr<IReceiver>(pReceiver));
}

void BaseNotifier::ClearReceiver()
{
	m_ReceiverList.clear();
}
