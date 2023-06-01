/*
 * mainfsm_basestate.h
 *
 *  Created on: 24.05.2023
 *      Author: Maik
 */

#pragma once

#include "MainActions.h"
#include "MainContextData.h"
#include <memory>

enum MainState {
	MAIN_NONE,
	STANDBY,
	RUNNING,
	SERVICEMODE,
	ERROR,
	ESTOP
};

class MainBasestate {
protected:
	MainBasestate* substateEStop;
	std::shared_ptr<MainActions> actions;
	std::shared_ptr<MainContextData> data;

public:
	virtual ~MainBasestate(){};

	virtual void setData(std::shared_ptr<MainContextData>) {
		this->data = data;
	}
	virtual void setAction(std::shared_ptr<MainActions> actions) {
		this->actions = actions;
	}

	virtual MainState getCurrentState() {
		return MAIN_NONE;
	};

	virtual void entry() {}
	virtual void exit() {}

	virtual void initSubStateEStop() {}

	virtual bool isSubEndState() { return false; };

	// Light Barriers
	virtual bool master_LBA_Blocked() 	{ return false; }	// Anfang blockiert
	virtual bool master_LBA_Unblocked() { return false; }	// Anfang wieder frei
	virtual bool master_LBW_Blocked() 	{ return false; }	// Weiche blockiert
	virtual bool master_LBW_Unblocked() { return false; }	// Weiche wieder frei
	virtual bool master_LBE_Blocked() 	{ return false; }	// Ende blockiert
	virtual bool master_LBE_Unblocked() { return false; }	// Ende wieder frei
	virtual bool master_LBR_Blocked() 	{ return false; }	// Rampe blockiert
	virtual bool master_LBR_Unblocked() { return false; }	// Rampe wieder frei

	virtual bool slave_LBA_Blocked() 	{ return false; }	// Anfang blockiert
	virtual bool slave_LBA_Unblocked() 	{ return false; }	// Anfang wieder frei
	virtual bool slave_LBW_Blocked() 	{ return false; }	// Weiche blockiert
	virtual bool slave_LBW_Unblocked() 	{ return false; }	// Weiche wieder frei
	virtual bool slave_LBE_Blocked() 	{ return false; }	// Ende blockiert
	virtual bool slave_LBE_Unblocked() 	{ return false; }	// Ende wieder frei
	virtual bool slave_LBR_Blocked() 	{ return false; }	// Rampe blockiert
	virtual bool slave_LBR_Unblocked() 	{ return false; }	// Rampe wieder frei

	// Buttons
	virtual bool master_btnStart_PressedShort() { return false; }
	virtual bool master_btnStart_PressedLong() 	{ return false; }
	virtual bool master_btnStop_Pressed() 		{ return false; }
	virtual bool master_btnReset_Pressed() 		{ return false; }

	virtual bool slave_btnStart_PressedShort() 	{ return false; }
	virtual bool slave_btnStart_PressedLong() 	{ return false; }
	virtual bool slave_btnStop_Pressed() 		{ return false; }
	virtual bool slave_btnReset_Pressed() 		{ return false; }

	// EStop
	virtual bool master_EStop_Pressed() 	{ return false; }
	virtual bool master_EStop_Released()	{ return false; }

	virtual bool slave_EStop_Pressed() 		{ return false; }
	virtual bool slave_EStop_Released() 	{ return false; }
};
