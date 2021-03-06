// -*- mode: c++ -*-
/****************************************************************************
*****                                                                  *****
*****                   Classification: UNCLASSIFIED                   *****
*****                    Classified By:                                *****
*****                    Declassify On:                                *****
*****                                                                  *****
****************************************************************************
*
* Developed by: NUWC Keyport 
*               Code 234
*               610 Dowell Street
*               Keyport WA, 98367
* @Khue Vu
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*/
#ifndef PLATFORMENGINE_H
#define PLATFORMENGINE_H

#include "PIIncludes.h"
#include <vector>

class TrackMessage;
class TrackData;

class PlatformEngine
{
protected:

	double startTime_;
	int lastPlatformTarget_;

	///Vector of unique IDs for each platform
	std::vector<PIData::UniqueID_t> platformVec_;

	///Vector of unique table IDs for each platform
	std::vector<DataTableId_t> platformTableIdVec1_;
	std::vector<DataTableId_t> platformTableIdVec2_;
	std::vector<DataTableId_t> platformTableIdVec3_;

	///table IDs for scenario level data tables
	DataTableId_t scenTableId1_;
	DataTableId_t scenTableId2_;
	DataTableId_t scenTableId3_;
	DataTableId_t scenTableId4_;

	void findPlatform_(double timeVal, const std::string& name);
	void createNewPlatform_(double newTime);
	void updatePlatform_(double timeVal, int platId);

	PIData::PIPlatformPoint calcPlatformPoint_(double timeVal, int platId) const;	///Calculates a platform point based on the given time

	//////////////// 
	//bool putPoint(TrackData* data);

public:

	//Constructor
	PlatformEngine();
	//Destructor
	virtual ~PlatformEngine();

	///Initialize the engine with a starting time
	void initialize(double startTime);
	///Advance the engine to a new time value; call this function when idle
	void advanceToTime(double newTime);
	///Handles callback functions from regCBPreAuthorizePlatformData
	int screenDataFromPlatform2(PIData::UniqueID_t id, PIData::PIPlatformPoint* point) const;



	/// Creates a SIMDIS scenario if needed, before platforms are created
	void createScenarioIfNecessary (int refYear, const double* originLLA);

	int getPlatformList();
};


#endif /* PLATFORMENGINE_H */