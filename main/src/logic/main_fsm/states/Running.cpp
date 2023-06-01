/*
 * Running.cpp
 *
 *  Created on: 24.05.2023
 *      Author: Maik
 */

#include "Running.h"
#include "Standby.h"
#include "EStop.h"
#include "logger/logger.hpp"

#include <iostream>

MainState Running::getCurrentState() {
	return MainState::RUNNING;
};

void Running::entry() {
	Logger::info("Entered Running mode");
	actions->setRunningMode();
	actions->setMotorStop(false);
	actions->setMotorFast(false);
}

void Running::exit() {
	Logger::debug("Running::exit");
}

bool Running::master_LBA_Blocked() {
	Logger::debug("Running::master_LBA_Blocked");
	Workpiece* wp = data->wpManager->addWorkpiece();
	Logger::info("New workpiece inserted: " + wp->getId());
	actions->setMotorFast(true);
	return true;
}

bool Running::master_LBA_Unblocked() {
	Logger::debug("Running::master_LBA_Unblocked");
	return true;
}

bool Running::master_LBW_Blocked() {
	Logger::debug("Running::master_LBW_Blocked");
	return true;
}

bool Running::master_LBW_Unblocked() {
	Logger::debug("Running::master_LBW_Unblocked");
	return true;
}

bool Running::master_LBE_Blocked() {
	Logger::debug("Running::master_LBE_Blocked");
	actions->setMotorStop(true);
	return true;
}

bool Running::master_LBE_Unblocked() {
	Logger::debug("Running::master_LBE_Unblocked");
	actions->setMotorStop(false);
	return true;
}

bool Running::master_LBR_Blocked() {
	Logger::debug("Running::master_LBR_Blocked");
	data->setRampFBM1Blocked(true);
	actions->setMotorFast(false);
	return true;
}

bool Running::master_LBR_Unblocked() {
	Logger::debug("Running::master_LBR_Unblocked");
	data->setRampFBM1Blocked(false);
	return true;
}

bool Running::slave_LBA_Blocked() {
	Logger::debug("Running::slave_LBA_Blocked");
	return true;
}

bool Running::slave_LBA_Unblocked() {
	Logger::debug("Running::slave_LBA_Unblocked");
	return true;
}

bool Running::slave_LBW_Blocked() {
	Logger::debug("Running::slave_LBW_Blocked");
	return true;
}

bool Running::slave_LBW_Unblocked() {
	Logger::debug("Running::slave_LBW_Unblocked");
	return true;
}

bool Running::slave_LBE_Blocked() {
	Logger::debug("Running::slave_LBE_Blocked");
	return true;
}

bool Running::slave_LBE_Unblocked() {
	Logger::debug("Running::slave_LBE_Unblocked");
	return true;
}

bool Running::slave_LBR_Blocked() {
	Logger::debug("Running::slave_LBR_Blocked");
	data->setRampFBM2Blocked(true);
	return true;
}

bool Running::slave_LBR_Unblocked() {
	Logger::debug("Running::slave_LBR_Unblocked");
	data->setRampFBM2Blocked(false);
	return true;
}

bool Running::master_btnStop_Pressed() {
	Logger::debug("Running::master_btnStop_Pressed");
	exit();
	new(this) Standby;
	entry();
	return true;
}

bool Running::slave_btnStop_Pressed() {
	Logger::debug("Running::slave_btnStop_Pressed");
	exit();
	new(this) Standby;
	entry();
	return true;
}

bool Running::master_EStop_Pressed() {
	Logger::debug("Running::master_EStop_Pressed");
	exit();
	new(this) EStop;
	entry();
	return true;
}

bool Running::slave_EStop_Pressed() {
	Logger::debug("Running::slave_EStop_Pressed");
	exit();
	new(this) EStop;
	entry();
	return true;
}
