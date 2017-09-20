#pragma once
#ifndef __MODULEHARDWARE_H__
#define __MODULEHARDWARE_H__

#include "Module.h"
#include "Globals.h"
#include "imgui.h"
#include "WindowManager.h"


class Hardware : public WindowManager
{
public:

	Hardware(bool enabled = false);
	virtual ~Hardware();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();

	void OpenClose();
	bool IsOpen();
	void Draw();


private:
	bool hardware_activated = false;


};


#endif