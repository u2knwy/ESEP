/*
 * Watchdog.cpp
 *
 *  Created on: 13.06.2023
 *      Author: Maik
 */

#include "Watchdog.h"
#include "logger/logger.hpp"
#include "configuration/Configuration.h"

#include <thread>
#include <chrono>
#include <sys/neutrino.h>

Watchdog::Watchdog(std::shared_ptr<EventManager> eventManager) {
	this->eventManager = eventManager;
	this->isMaster = Configuration::getInstance().systemIsMaster();

	if(isMaster) {
		eventManager->subscribe(EventType::WD_S_HEARTBEAT, std::bind(&Watchdog::handleEvent, this, std::placeholders::_1));
	} else {
		eventManager->subscribe(EventType::WD_M_HEARTBEAT, std::bind(&Watchdog::handleEvent, this, std::placeholders::_1));
	}
}

Watchdog::~Watchdog() {
	stop();
}

void Watchdog::handleEvent(Event event) {
	switch(event.type) {
	case WD_M_HEARTBEAT:
	case WD_S_HEARTBEAT:
		heartbeatReceived();
		break;
	default:
		break;
	}
}

void Watchdog::start() {
	stop();
	th_send = std::thread(&Watchdog::sendingThread, this);
	th_receive = std::thread(&Watchdog::receivingThread, this);
}

void Watchdog::stop() {
	receivingRunning = false;
	if(th_receive.joinable()) {
		th_receive.join();
	}
	sendingRunning = false;
	if(th_send.joinable()) {
		th_send.join();
	}
}

void Watchdog::sendingThread() {
	Logger::debug("[WD] Started sending heartbeats...");
	sendingRunning = true;
	while(sendingRunning) {
		Logger::debug("[WD] Sending heartbeat...");
		sendHeartbeat();
		std::this_thread::sleep_for(std::chrono::seconds(WD_SEND_INTERVAL_SEC));
	}
	sendingRunning = false;
	Logger::debug("[WD] Stopped sending heartbeats");
}

void Watchdog::sendHeartbeat() {
	if(isMaster) {
		eventManager->sendEvent(Event{WD_M_HEARTBEAT});
	} else {
		eventManager->sendEvent(Event{WD_S_HEARTBEAT});
	}
}

void Watchdog::receivingThread() {
	using namespace std::chrono;
	Logger::debug("[WD] Started receiving heartbeats...");
	receivingRunning = true;
	lastReceiveTime = steady_clock::now();
	while(receivingRunning) {
		// every second: check if timeout has occurred
		std::this_thread::sleep_for(seconds(1));
		const auto now = steady_clock::now();
		int elapsed_sec = duration_cast<seconds>(now - lastReceiveTime).count();
		if(elapsed_sec > WD_TIMEOUT_SEC) {
			Logger::error("[WD] Heartbeat received " + std::to_string(elapsed_sec) + " seconds ago -> ERROR!!!");
		} else {
			Logger::info("[WD] Heartbeat received " + std::to_string(elapsed_sec) + " seconds ago -> OK!");
		}
	}
	Logger::debug("[WD] Stopped receiving heartbeats");
	receivingRunning = false;
}

void Watchdog::heartbeatReceived() {
	using namespace std::chrono;
	Logger::debug("[WD] Received heartbeat");
	lastReceiveTime = steady_clock::now();
}
