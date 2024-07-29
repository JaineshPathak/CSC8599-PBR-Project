#pragma once

class IObserver
{
public:
	virtual ~IObserver() {};
	virtual void UpdateData(const int& sizeX, const int& sizeY) = 0;
};