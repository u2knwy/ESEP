/*
 * Configuration.h
 *
 *  Created on: 07.05.2023
 *      Author: Maik
 */
#pragma once

#include <string>
#include <vector>

enum WorkpieceType {
	WS_F,
	WS_BOM,
	WS_BUM,
	WS_OB,
	UNKNOWN
};

class Configuration {
public:
	static Configuration& getInstance() {
		static Configuration instance;
		return instance;
	}
	void readConfigFromFile(const std::string filePath);
	void setMaster(bool isMaster);
	bool systemIsMaster();
	void setPusherMounted(bool pusherIsMounted);
	bool pusherMounted();
	void setDesiredWorkpieceOrder(std::vector<WorkpieceType> order);
	std::vector<WorkpieceType> getDesiredOrder();
private:
	Configuration();
	virtual ~Configuration();
	Configuration(const Configuration&) = delete;
	Configuration& operator=(const Configuration&) = delete;
	std::vector<WorkpieceType> order;
	bool isMaster{true};
	bool hasPusher{false};
	int calOffset{0};
	int calAdcPerMillimeter{50};
};
