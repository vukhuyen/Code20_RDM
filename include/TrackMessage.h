/***************************************************************************
Classification: UNCLASSIFIED                   
****************************************************************************
* Developed by: NUWC Keyport 
*               Code 234
*               610 Dowell Street
*               Keyport WA, 98367
* @Khue Vu
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*/

#ifndef TRACKMESSAGE_H
#define TRACKMESSAGE_H

#include <iostream>
#include <sstream>
#include <string>
#include "stlstdlib.h"

class TrackMessage
{
protected:
	
	std::string callsign_;
	std::string platIcon_;
	uint64_t id_;	
	double startTime_;
	double refLat_;
	double refLon_;		
	double tarLat_;
	double tarLon_;

public:
	TrackMessage(){
		callsign_="LFQ";
		platIcon_="aircraft";
		id_=1;
		startTime_=0;
		refLat_=0;
		refLon_=0;
		tarLat_=0;
		tarLon_=0;
	}
	/*
	TrackMessage(std::string callsign, std::string platIcon, uint64_t id, double refLat, double refLon, double tarLat, double tarLon){
		this->callsign_ = callsign;
		this->platIcon_ = platIcon;
		this->id_ = id;
		//this->startTime_= startTime;
		this->refLat_ = refLat;
		this->refLon_= refLon;
		this->tarLat_=tarLat;
		this->tarLon_=tarLon;
	}*/
	
	virtual ~TrackMessage();

	void getData(std::string s);
	std::string getCallsign();
	std::string getIcon();
	uint64_t getId();	
	double getStartTime();
	double getRefLat();
	double getRefLon();		
	double getTarLat();
	double getTarLon();
};

#endif