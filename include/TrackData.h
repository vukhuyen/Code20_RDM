
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
#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <string>
#include <map>
#include "PIIncludes.h"

class TrackData
{

protected:

	///Data points for position data, sorted by time
	/**
	* Map of data points sorted by time.  This is maintained in a map
	* so that we can iterate through and assign orientation and velocity
	* values to the data points after all the data has been read in
	*/
	std::map<double, PIData::PIPlatformPoint> dataPoints_;

	///Name of the platform to create
	std::string callsign_;		//callsign name
	uint64_t originalID_;	//platform id
	std::string iconName_;		//platform icon
	Tspi* tspi_;


	int refYear_;	///Earliest year encountered

public:

	//Create a new TrackDta with the given name
	TrackData(const std::string& name = "Unknown Track");

	//Add a data point to the internal lists
	/**
	* Add a new data point to the internal lists, to be later added to
	* SIMDIS.  The time is in seconds since reference year, lat and lon
	* are in radians, and altitude is in meters.
	* @param timeVal Time value of the data point
	* @param refYear Reference year of the data point (e.g. 2008)
	* @param lat Latitude of posit in radians
	* @param lon Longitude of posit in radians
	* @param alt Altitude of posit in meters
	*/
	void addData(double timeVal, int refYear, double lat, double lon, double alt);
	void addData(double timeVal, int refYear, Tspi* tspi);

	void setCallsign(std::string& name) {callsign_ = name;}
	std::string getCallsign() {return callsign_;}
	void setIcon(std::string& iconName){iconName_=iconName;};
	std::string getIcon(){return iconName_;};
	void setOrigId(uint64_t originalID ){originalID_ = originalID;};
	uint64_t getOrigId(){return originalID_;};
	void setTspi(Tspi* tpsi){tspi_=tpsi;};
	Tspi* getTpsi(){return tspi_;};

	int getRefYear() const{return refYear_;}

	void getOriginLLA(double* lla) const;
	
	///Calculates the velocity values and commits the results to SIMDIS
	PIData::UniqueID_t sendToSIMDIS();
};

class Tspi
{
protected:
	double lat_;					//Reference latitude position in degree
	double lon_;					//Reference longitude position in degree

public:
	void setLat(double lat){lat_ = lat;};
	void setLon(double lon){lon_= lon;};

	double getLat_(){return lat_;};
	double getLon_(){return lon_;};
};

#endif