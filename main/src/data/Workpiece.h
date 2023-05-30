/*
 * Workpiece.h
 *
 *  Created on: 24.05.2023
 *      Author: Maik
 */

#pragma once

enum WorkpieceType {
	WS_F,
	WS_BOM,
	WS_BUM,
	WS_OB,
	UNKNOWN
};

enum WorkpieceStatus {
	FBM1_START_HM,
	FBM1_HM_MD,
	FBM1_MD_SW,
	FBM1_SW_RAMP,
	FBM1_SW_END,
	FBM2_START_HM,
	FBM2_HM_MD,
	FBM2_MD_SW,
	FBM2_SW_RAMP,
	FBM2_SW_END
};

class Workpiece {
public:
	Workpiece(int id);
	virtual ~Workpiece() {};

	/**
	 * Get workpiece ID
	 *
	 * @return ID of the workpiece
	 */
	int getId();

	/**
	 * Sets the type of the workpiece
	 *
	 * @param type Workpiece type
	 */
	void setType(WorkpieceType type);

	/**
	 * Sets the average measured height at FBM1
	 *
	 * @param height Measured average height on FBM1
	 */
	void setAvgHeightFBM1(float height);

	/**
	 * Sets the result of metal sensor at FBM1
	 *
	 * @param metal true if metal was detected (default: false)
	 */
	void setMetalFBM1(bool metal);

	/**
	 * Sets the maximum measured height at FBM2
	 *
	 * @param height Measured maximum height on FBM2
	 */
	void setMaxHeightFBM2(float height);

	/**
	 * Sets the result of metal sensor at FBM2
	 *
	 * @param metal true if metal was detected (default: false)
	 */
	void setMetalFBM2(bool metal);

	/**
	 * Sets if the workpiece was flipped.
	 *
	 * @param flipped true if workpiece was flipped (default: false)
	 */
	void setFlipped(bool flipped);

	/**
	 * Get the type of the workpiece
	 *
	 * @return Workpiece Type (TYPE_A, TYPE_B, TYPE_C or UNKNOWN (default))
	 */
	WorkpieceType getType();

	WorkpieceStatus getStatus();
	void setStatus(WorkpieceStatus newStatus);
private:
	int id{-1};
	float avgHeightFBM1{0.0};
	bool metalFBM1{false};
	float maxHeightFBM2{0.0};
	bool metalFBM2{false};
	bool flipped{false};
	WorkpieceType type{UNKNOWN};
	WorkpieceStatus status{FBM1_START_HM};
};
