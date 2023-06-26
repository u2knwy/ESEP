/*
 * EventManager.cpp
 *
 *  Created on: 25.05.2023
 *      Author: Maik
 */

#include "EventManager.h"
#include "configuration/Configuration.h"
#include "events.h"
#include "logger/logger.hpp"

#include <errno.h>
#include <iostream>
#include <watchdog/Watchdog.h>
#include <sstream>
#include <string>
#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <sys/neutrino.h>

#define STR_MSG (_IO_MAX + 1)
#define DATA_MSG (_IO_MAX + 2)

EventManager::EventManager() : internal_chid(-1), internal_coid(-1), server_coid(-1) {
    isMaster = Configuration::getInstance().systemIsMaster();
    rcvInternalRunning = false;
    rcvExternalRunning = false;
    externConnected = false;
    if(isMaster) {
    	ownServiceName = ATTACH_POINT_LOCAL_M;
    	otherServiceName = ATTACH_POINT_LOCAL_S;
    } else {
    	ownServiceName = ATTACH_POINT_LOCAL_S;
		otherServiceName = ATTACH_POINT_LOCAL_M;
    }
    attachedService = nullptr;
}

EventManager::~EventManager() {
    disconnectFromService();
    stopService();
    ConnectDetach(internal_coid);
    ChannelDestroy(internal_chid);
}
void EventManager::connectionLost(){
	Logger::debug("[EventManager] Disconnected from external");
	disconnected = true;
}

void EventManager::openInternalChannel() {
    internal_chid = ChannelCreate(0);
    if (internal_chid == -1) {
        Logger::error("[EventManager] Creating internal channel failed");
        throw std::runtime_error("ChannelCreate failed");
    }
    internal_coid = ConnectAttach(0, 0, internal_chid, _NTO_SIDE_CHANNEL, 0);
    if (internal_coid == -1) {
        Logger::error("[EventManager] Attaching to internal channel failed");
        throw std::runtime_error("ConnectAttach failed");
    }
}

void EventManager::sendToSelf(Event event) {
    int res = MsgSendPulse(internal_coid, -1, (int) event.type, event.data);
    if (res < 0) {
        Logger::error("Failed to send pulse message to self");
    }
}

int EventManager::connectInternalClient() {
    if (internal_chid == -1) {
        openInternalChannel();
    }
    int coid = ConnectAttach(0, 0, internal_chid, _NTO_SIDE_CHANNEL, 0);
    if (coid == -1) {
        Logger::error("ConnectAttach failed");
    }
    return coid;
}

void EventManager::rcvInternalEventsThread() {
    openInternalChannel();
    rcvInternalRunning = true;
    Logger::debug("[EventManager] Ready to receive internal events");
    while (rcvInternalRunning) {
        _pulse pulse;
        int rtrn = MsgReceivePulse(internal_chid, &pulse, sizeof(pulse), NULL);
        if (rtrn < 0) {
            Logger::error("Error during MsgReceivePulse: " +
                          std::to_string(errno));
        }
        if (pulse.code == PULSE_STOP_THREAD) {
            rcvInternalRunning = false;
            continue;
        }
        Event ev{(EventType) pulse.code, pulse.value.sival_int};
        if((isMaster && ev.type == EventType::WD_S_HEARTBEAT)
        || (!isMaster && ev.type == EventType::WD_M_HEARTBEAT)){
        	Logger::debug("attempted rebound msg");
        	continue; }
        handleEvent(ev);
        if(ev.type == EventType::WD_CONN_LOST){ disconnected = true; }
        if(ev.type == EventType::WD_CONN_REESTABLISHED){ disconnected = false; }
        if(disconnected){continue;}

        sendExternalEvent(ev);
    }
    Logger::debug("[EventManager] Stopped receiving internal events");
}

void EventManager::rcvExternalEventsThread() {
    Logger::debug("[EventManager] Ready to receive external events");
    rcvExternalRunning = true;
    while (rcvExternalRunning) {
        // Waiting for a message and read first header
        header_t header;
        int rcvid = MsgReceive(attachedService->chid, &header, sizeof (header_t), NULL);
        if (rcvid == -1) { // Error occurred
            Logger::error("[EventManager] MsgReceive @GNS failed");
            break;
        }
        if (rcvid == 0) {// Pulse was received
        	if(isMaster){
        	handleEvent(Event{WD_S_HEARTBEAT});
        	}else{
        		handleEvent(Event{WD_M_HEARTBEAT});
        	}
        	handle_pulse(header, rcvid);
            continue;
        }
        // continue while (1) loop
        // some sync msg was received
        if ((_IO_BASE <= header.type) && (header.type <= _IO_MAX)) {
            // Some QNX IO msg generated by gns was received
            handle_ONX_IO_msg(header, rcvid);
            continue;
        }
        // A sync msg (presumable ours) was received; handle it
        handle_app_msg(header, rcvid);
    }
    rcvExternalRunning = false;
    Logger::debug("[EventManager] Stopped receiving external events");
}

void EventManager::handle_pulse(header_t hdr, int rcvid){
    switch (hdr.code) {
    case _PULSE_CODE_DISCONNECT:
        Logger::debug("Server received _PULSE_CODE_DISCONNECT");
        Logger::info("[EventManager] Other system was disconnected");
        /* A client disconnected all its connections (called
         * name_close() for each name_open() of our name) or
         * terminated. */
        ConnectDetach(hdr.scoid);
        sendToSelf(Event{EventType::WD_CONN_LOST});
        externConnected = false;
        break;
    case _PULSE_CODE_UNBLOCK:
    	Logger::debug("Server received _PULSE_CODE_UNBLOCK");
        /* REPLY blocked client wants to unblock (was hit by
         * a signal or timed out). It's up to you if you
         * reply now or later. */
        break;
    default:
        /* A pulse sent by one of your processes or a
         * _PULSE_CODE_COIDDEATH or _PULSE_CODE_THREADDEATH
         * from the kernel? */
    	if(hdr.code > 0){
    	Event ev;
        ev.type = (EventType) hdr.code;
        ev.data = hdr.value.sival_int;
        handleEvent(ev);
    	}
        break;
    }
}

void EventManager::handle_ONX_IO_msg(header_t hdr, int rcvid){
    if (hdr.type == _IO_CONNECT ) {
        // QNX IO msg _IO_CONNECT was received; answer with EOK
    	Logger::debug("Server received _IO_CONNECT (sync. msg)");
        MsgReply( rcvid, EOK, NULL, 0 );
    } else {
    	// Some other QNX IO message was received; reject it
    	Logger::warn("Server received unexpected (sync.) msg type = " + std::to_string(hdr.type));
    	MsgError( rcvid, ENOSYS );
    }
}

void EventManager::handle_app_msg(header_t hdr, int rcvid){
    if (DATA_MSG == hdr.type) {
    	Logger::debug("handle_app_msg: DATA_MSG not supported.");
        MsgError(rcvid,EPERM);
    } else if (STR_MSG == hdr.type) {
        // read app header
        app_header_t app_header;
        MsgRead(rcvid, &app_header, sizeof(app_header), sizeof(header_t));

        Event ev;
        ev.type = (EventType) app_header.eventnr;
        ev.data = app_header.data;
        std::stringstream ss;
        ss << "External event received: " << EVENT_TO_STRING(ev.type);
        if(ev.data != -1) {
			ss << " - data: " << ev.data;
        }
    	Logger::debug(ss.str());
        MsgReply(rcvid, EOK, "OK", 2); // send reply

        handleEvent(ev);
    } else { // Wrong msg type
    	Logger::warn("Server: Wrong message type: " + std::to_string(hdr.type));
        MsgError(rcvid,EPERM);
    }
}

void EventManager::createService() {
    if ((attachedService = name_attach(NULL, ownServiceName.c_str(), NAME_FLAG_ATTACH_GLOBAL)) == NULL) {
        Logger::error("name_attach failed");
    } else {
        Logger::info("Service created: " + ownServiceName);
    }
}

void EventManager::stopService() {
    if (name_detach(attachedService, 0) == -1) {
        Logger::error("[EventManager] Failed detaching Service");
    } else {
		Logger::info("[EventManager] GNS Service was stopped");
		externConnected = false;
    }
}

void EventManager::connectToService(const std::string& name) {
    Logger::info("[EventManager] Connecting to service: " + name + " ...");
    externConnected = false;
    while(!externConnected) {
        server_coid = name_open(name.c_str(), NAME_FLAG_ATTACH_GLOBAL);
        externConnected = server_coid != -1;
        if(!externConnected) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    externConnected = true;
    Logger::info("[EventManager] Connected to service: " + name);
}

void EventManager::disconnectFromService() {
    if (name_close(server_coid) == -1) {
        Logger::error("[EventManager] Failed detaching from remote Service");
    } else {
		Logger::info("[EventManager] Disconnected from remote GNS Service");
    }
    externConnected = false;
}

void EventManager::subscribe(EventType type, EventCallback callback) {
    if (subscribers.find(type) == subscribers.end()) {
        // If event doesn't exist yet, create a new list and add it to the map
        subscribers[type] = std::vector<EventCallback>();
    }
    subscribers[type].push_back(callback);
}

int EventManager::subscribeToAllEvents(EventCallback callback) {
    int nEvents = 0;
    for (int i = static_cast<int>(EventType::PULSE_STOP_THREAD);
         i <= static_cast<int>(EventType::ERROR_S_SELF_SOLVED); i++) {
        EventType eventType = static_cast<EventType>(i);
        subscribe(eventType, callback);
        nEvents++;
    }
    return nEvents;
}

void EventManager::unsubscribe(EventType type, EventCallback callback) {

}

void EventManager::handleEvent(const Event &event) {
   if(event.type == EventType::WD_M_HEARTBEAT && isMaster){ return; }
   if(event.type == EventType::WD_S_HEARTBEAT && !isMaster){ return; }

    std::stringstream ss;
    ss << "[EventManager] handleEvent: " << EVENT_TO_STRING(event.type);

    if (event.data != -1)
        ss << ", data: " << event.data;

    if (subscribers.find(event.type) != subscribers.end()) {
        //Logger::debug("[EventManager] Notifiying " + std::to_string(subscribers[event.type].size()) + " subscribers about Event " + EVENT_TO_STRING(event.type));
        int i = 1;
        for (const auto &callback : subscribers[event.type]) {
            callback(event);
        }
    } else {
        ss << " -> No subscribers for Event!";
    }


    if(event.type ==EventType::WD_M_HEARTBEAT || event.type== WD_S_HEARTBEAT){
    	return;
    }
    Logger::debug(ss.str());
}

void EventManager::sendExternalEvent(const Event &event) {
	if(!externConnected || server_coid < 0) {
		return;
	}
    // TODO: Send event to other system
    //int res = MsgSendPulse(this->server_coid, -1, (int) event.type, event.data);


    if (-1 == MsgSendPulse(server_coid, -1,event.type, event.data)){
        perror("Client: MsgSendPulse failed");
    }

    // Answer form server should be structured, too.
    //Logger::debug("Client: Answer from server: " + r_msg);

}

int EventManager::start() {
    createService();
    thRcvExternal = std::thread(&EventManager::rcvExternalEventsThread, this);
    if(isMaster) {
        connectToService(ATTACH_POINT_LOCAL_S);
    } else {
        connectToService(ATTACH_POINT_LOCAL_M);
    }
    thRcvInternal = std::thread(&EventManager::rcvInternalEventsThread, this);
    return 0;
}

int EventManager::stop() {
    int res = MsgSendPulse(internal_coid, -1, PULSE_STOP_THREAD, 0);
    if (res < 0) {
        Logger::error("[EventManager] Error during MsgSendPulse: " +
                      std::to_string(errno));
    }
    thRcvInternal.join();

    disconnectFromService();
    stopService();
    return 0;
}
