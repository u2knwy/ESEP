/*
 * MainContext.cpp
 *
 *  Created on: 24.05.2023
 *      Author: Maik
 */

#include "MainContext.h"
#include "MainContextData.h"
#include "states/Standby.h"
#include "logger/logger.hpp"

#include <memory>

MainContext::MainContext(std::shared_ptr<EventManager> mngr) : eventManager(mngr) {
	this->data = std::unique_ptr<MainContextData>(new MainContextData());
	this->state = new Standby();
	// TODO: @Domi subscribeToEvents funktioniert so nicht (sobald ich mehrere Threads habe die auf Events subscribed sind)
	// "Process 348184 (main) terminated SIGSEGV code=1 fltno=11 ip=0102cda8(/usr/lib/ldqnx.so.2@_band_get_aligned+0x000002b8) mapaddr=0002cda8. ref=00000000"
	//subscribeToEvents();
}

MainContext::~MainContext() {
	delete state;
}

void MainContext::subscribeToEvents() {
	eventManager->subscribe(EventType::START_M_SHORT, std::bind(&MainContext::master_btnStart_PressedShort, this));
	eventManager->subscribe(EventType::START_M_LONG, std::bind(&MainContext::master_btnStart_PressedLong, this));
	eventManager->subscribe(EventType::STOP_M_SHORT, std::bind(&MainContext::master_btnStop_Pressed, this));
	eventManager->subscribe(EventType::RESET_M_SHORT, std::bind(&MainContext::master_btnReset_Pressed, this));
	eventManager->subscribe(EventType::ESTOP_M_PRESSED, std::bind(&MainContext::master_EStop_Pressed, this));
	eventManager->subscribe(EventType::ESTOP_M_RELEASED, std::bind(&MainContext::master_EStop_Released, this));

	eventManager->subscribe(EventType::START_S_SHORT, std::bind(&MainContext::slave_btnStart_PressedShort, this));
	eventManager->subscribe(EventType::START_S_LONG, std::bind(&MainContext::slave_btnStart_PressedLong, this));
	eventManager->subscribe(EventType::STOP_S_SHORT, std::bind(&MainContext::slave_btnStop_Pressed, this));
	eventManager->subscribe(EventType::RESET_S_SHORT, std::bind(&MainContext::slave_btnReset_Pressed, this));
	eventManager->subscribe(EventType::ESTOP_S_PRESSED, std::bind(&MainContext::slave_EStop_Pressed, this));
	eventManager->subscribe(EventType::ESTOP_S_RELEASED, std::bind(&MainContext::slave_EStop_Released, this));
}

MainState MainContext::getCurrentState() {
	return state->getCurrentState();
}

void MainContext::master_LBA_Blocked() {
	Logger::debug("MainContext::master_LBA_Blocked");
	state->master_LBA_Blocked();
}

void MainContext::master_LBA_Unblocked() {
	Logger::debug("MainContext::master_LBA_Unblocked");
	state->master_LBA_Unblocked();
}

void MainContext::master_LBW_Blocked() {
	Logger::debug("MainContext::master_LBW_Blocked");
	state->master_LBW_Blocked();
}

void MainContext::master_LBW_Unblocked() {
	Logger::debug("MainContext::master_LBW_Unblocked");
	state->master_LBW_Unblocked();
}

void MainContext::master_LBE_Blocked() {
	Logger::debug("MainContext::master_LBE_Blocked");
	state->master_LBE_Blocked();
}

void MainContext::master_LBE_Unblocked() {
	Logger::debug("MainContext::master_LBE_Unblocked");
	state->master_LBE_Unblocked();
}

void MainContext::master_LBR_Blocked() {
	Logger::debug("MainContext::master_LBR_Blocked");
	state->master_LBR_Blocked();
}

void MainContext::master_LBR_Unblocked() {
	Logger::debug("MainContext::master_LBR_Unblocked");
	state->master_LBR_Unblocked();
}

void MainContext::slave_LBA_Blocked() {
	Logger::debug("MainContext::slave_LBA_Blocked");
	state->slave_LBA_Blocked();
}

void MainContext::slave_LBA_Unblocked() {
	Logger::debug("MainContext::slave_LBA_Unblocked");
	state->slave_LBA_Unblocked();
}

void MainContext::slave_LBW_Blocked() {
	Logger::debug("MainContext::slave_LBW_Blocked");
	state->slave_LBW_Blocked();
}

void MainContext::slave_LBW_Unblocked() {
	Logger::debug("MainContext::slave_LBW_Unblocked");
	state->slave_LBW_Unblocked();
}

void MainContext::slave_LBE_Blocked() {
	Logger::debug("MainContext::slave_LBE_Blocked");
	state->slave_LBE_Blocked();
}

void MainContext::slave_LBE_Unblocked() {
	Logger::debug("MainContext::slave_LBE_Unblocked");
	state->slave_LBE_Unblocked();
}

void MainContext::slave_LBR_Blocked() {
	Logger::debug("MainContext::slave_LBR_Blocked");
	state->slave_LBR_Blocked();
}

void MainContext::slave_LBR_Unblocked() {
	Logger::debug("MainContext::slave_LBR_Unblocked");
	state->slave_LBR_Unblocked();
}

void MainContext::master_btnStart_PressedShort() {
	Logger::debug("MainContext::master_btnStart_PressedShort");
	state->master_btnStart_PressedShort();
}

void MainContext::master_btnStart_PressedLong() {
	Logger::debug("MainContext::master_btnStart_PressedLong");
	state->master_btnStart_PressedLong();
}

void MainContext::master_btnStop_Pressed() {
	Logger::debug("MainContext::master_btnStop_Pressed");
	state->master_btnStop_Pressed();
}

void MainContext::master_btnReset_Pressed() {
	Logger::debug("MainContext::master_btnReset_Pressed");
	state->master_btnReset_Pressed();
}

void MainContext::slave_btnStart_PressedShort() {
	Logger::debug("MainContext::slave_btnStart_PressedShort");
	state->slave_btnStart_PressedShort();
}

void MainContext::slave_btnStart_PressedLong() {
	Logger::debug("MainContext::slave_btnStart_PressedLong");
	state->slave_btnStart_PressedLong();
}

void MainContext::slave_btnStop_Pressed() {
	Logger::debug("MainContext::slave_btnStop_Pressed");
	state->slave_btnStop_Pressed();
}

void MainContext::slave_btnReset_Pressed() {
	Logger::debug("MainContext::slave_btnReset_Pressed");
	state->slave_btnReset_Pressed();
}

void MainContext::master_EStop_Pressed() {
	Logger::debug("MainContext::master_EStop_Pressed");
	state->master_EStop_Pressed();
}

void MainContext::master_EStop_Released() {
	Logger::debug("MainContext::master_EStop_Released");
	state->master_EStop_Released();
}

void MainContext::slave_EStop_Pressed() {
	Logger::debug("MainContext::slave_EStop_Pressed");
	state->slave_EStop_Pressed();
}

void MainContext::slave_EStop_Released() {
	Logger::debug("MainContext::slave_EStop_Released");
	state->slave_EStop_Released();
}
