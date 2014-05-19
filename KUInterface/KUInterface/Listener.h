#ifndef __LISTENER_H_
#define __LISTENER_H_

using namespace std;
#include <vector>
#include <stdafx.h>
#include <memory>
interface IReceiver
{
	virtual void OnEvent(const void* pData) = 0;
};

interface INotifier
{
	virtual void SendNotify(const void* pData) = 0;
	virtual void AddReceiver(IReceiver* pReceiver) = 0;
	virtual void ClearReceiver() = 0;
};

class BaseNotifier: public INotifier
{
public:
	void SendNotify(const void* pData);
	void AddReceiver(IReceiver* pReceiver);
	void ClearReceiver();

protected:
	vector<shared_ptr<IReceiver>> m_ReceiverList;
};

#endif