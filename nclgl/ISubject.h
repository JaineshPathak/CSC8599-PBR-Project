#pragma once
#include "IObserver.h"
#include <memory>

class ISubject
{
public:
	virtual ~ISubject() {}
	virtual void AddObserver(std::shared_ptr<IObserver> observer) = 0;
	virtual void RemoveObserver(std::shared_ptr<IObserver> observer) = 0;
	virtual void NotifyObservers() = 0;
};