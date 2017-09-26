#pragma once
#ifndef __WINDOWMANAGER_H__
#define __WINDOWMANAGER_H__

#include "Globals.h"
#include "imgui.h"
#include <list>
#include <vector>
#include "MathGeoLib.h"


struct Active
{
	bool active = false;

	void OpenClose()
	{
		active = !active;
	}
	bool IsActive()
	{
		return active;
	}

};

class WindowManager
{
public:
	WindowManager()
	{
		
	}
	virtual ~WindowManager()
	{

	}

	virtual bool Start()
	{
		return true;
	}
	virtual update_status Update(float dt)
	{

		return update_status();
	}
	virtual bool CleanUp()
	{
		return true;
	}

	virtual void SpecialFunction(const std::string name)
	{

	}
	/*virtual void OpenClose()
	{
		active = !active;
	}*/

	/*virtual bool IsActive()
	{
		return active;
	}*/

public:
	std::vector<Active> active;

	// or std::vector<bool> active; YES
	//ModuleObjects* obj = nullptr;

};

#endif // __WINDOWMANAGER_H__ 