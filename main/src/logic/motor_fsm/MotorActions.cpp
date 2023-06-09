/*
 * MotorActions.cpp
 *
 *  Created on: 09.06.2023
 *      Author: Maik
 */

#include "MotorActions.h"
#include "logger/logger.hpp"
#include "configuration/Configuration.h"

MotorActions::MotorActions(std::shared_ptr<EventManager> mngr, bool master) {
	this->eventManager = mngr;
	this->isMaster = master;
}

MotorActions::~MotorActions() {
	// TODO Auto-generated destructor stub
}

void MotorActions::motorStop() {
	Logger::debug("[MotorFSM] Motor stop");
	Event ev;
	ev.type = isMaster ? EventType::MOTOR_M_STOP : EventType::MOTOR_S_STOP;
	eventManager->sendEvent(ev);
}

void MotorActions::motorRightFast() {
	Logger::debug("[MotorFSM] Motor right fast");
	Event ev;
	ev.type = isMaster ? EventType::MOTOR_M_FAST : EventType::MOTOR_S_FAST;
	eventManager->sendEvent(ev);
}

void MotorActions::motorRightSlow() {
	Logger::debug("[MotorFSM] Motor right slow");
	Event ev;
	ev.type = isMaster ? EventType::MOTOR_M_SLOW : EventType::MOTOR_S_SLOW;
	eventManager->sendEvent(ev);
}