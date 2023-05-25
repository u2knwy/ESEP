/*
 * Configuration.cpp
 *
 *  Created on: 07.05.2023
 *      Author: Maik
 */

#include "configuration/Configuration.h"
#include "logger/logger.hpp"
#include "hal/HeightSensor.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

Configuration::Configuration() {
}

Configuration::~Configuration() {
}

void Configuration::readConfigFromFile(const std::string filePath) {
	std::ifstream fileStream(filePath);
	if(fileStream.is_open()) {
		Logger::info("Read config file: " + filePath);
		std::string line;
		while (std::getline(fileStream, line)) {
			std::istringstream iss(line);
			std::string key;
			if(std::getline(iss, key, '=')) {
				std::string value;
				if(std::getline(iss, value)) {
					Logger::debug(key + " = " + value);
					if (key == "ORDER") {
						std::vector<WorkpieceType> workpieceOrder;
						std::string wpType;
						std::istringstream tokenStream(value);
						while (std::getline(tokenStream, wpType, ',')) {
							if(wpType == "F") {
								workpieceOrder.push_back(WorkpieceType::WS_F);
							} else if(wpType == "BOM") {
								workpieceOrder.push_back(WorkpieceType::WS_BOM);
							}
							else if(wpType == "BUM") {
								workpieceOrder.push_back(WorkpieceType::WS_BUM);
							}
							else if(wpType == "OB") {
								workpieceOrder.push_back(WorkpieceType::WS_OB);
							}
						}
						std::stringstream ss;
						for(auto const& wsType: workpieceOrder) {
							ss << " - " << wsType;
						}
						Logger::info("Configured workpiece order: " + ss.str());
					} else if(key == "CAL_OFFSET") {
						calOffset = std::stoi(value);
					} else if(key == "CAL_REF") {
						calRefHigh = std::stoi(value);
					}
				}
			}
		}
		Logger::info("Cal. Offset: " + std::to_string(calOffset));
		Logger::info("Cal. Ref: " + std::to_string(calRefHigh));
	} else {
		Logger::warn("Config file " + filePath + " does not exist -> create new and write default values");
		std::ofstream fileStream;
		fileStream.open(filePath);
		fileStream << "ORDER=F,OB,BOM\n";
		fileStream << "CAL_OFFSET=" << ADC_DEFAULT_OFFSET << "\n";
		fileStream << "CAL_REF=" << ADC_DEFAULT_HIGH << "\n";
		fileStream.close();
	}
}

void Configuration::setMaster(bool isMaster) {
	this->isMaster = isMaster;
}

bool Configuration::systemIsMaster() {
	return isMaster;
}

void Configuration::setPusherMounted(bool pusherIsMounted) {
	this->hasPusher = pusherIsMounted;
}

bool Configuration::pusherMounted() {
	return hasPusher;
}

void Configuration::setDesiredWorkpieceOrder(std::vector<WorkpieceType> order) {
	this->order = order;
}

std::vector<WorkpieceType> Configuration::getDesiredOrder() {
	return order;
}

Calibration Configuration::getCalibration() {
	Calibration cal = {
			.calOffset = this->calOffset,
			.calRef = this->calRefHigh
	};
	return cal;
}
