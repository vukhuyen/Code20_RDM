
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

#include "TrackData.h"
#include <string>

TrackData::TrackData (const std::string& name)
	: callsign_ (name)
{
	// Initialize reference year to a high value so that it can be MIN'd later
	refYear_ = 9999;
	// This plug-in is not Y-10k compliant!
}

void TrackData::addData (double timeVal, int refYear, double lat, double lon, double alt)
{
	// Save the lowest reference year value for later
	if (refYear < refYear_)
		refYear_ = refYear;

	// Create the PIPlatformPoint structure and add it to internals
	PIPlatformPoint dataPoint;
	dataPoint.time = timeVal;
	dataPoint.position.x = lat;
	dataPoint.position.y = lon;
	dataPoint.position.z = alt;
	dataPoint.referenceFrame.coordSystem = PI_COORDSYS_LLA;

	// No checking for overwriting old points
	dataPoints_[timeVal] = dataPoint;
}

void TrackData::getOriginLLA (double* lla) const
{
	if (dataPoints_.empty())
		return;
	// Return the first data point in a LLA[3]
	std::map<double, PIPlatformPoint>::const_iterator iter = dataPoints_.begin();
	lla[0] = iter->second.position.x;
	lla[1] = iter->second.position.y;
	lla[2] = iter->second.position.z;
}

/**
* Functor class that will create a platform or add data points
* to a previously created platform as required
*/
class SendPlatformPoint
{
protected:
	PIData::UniqueID_t id_;	///< Initialized to 0, set once during createPlatform()
	std::string callsign_;	///< Initialized at construction
public:
	///Create a new functor to create a platform with given name
	SendPlatformPoint(const std::string& name) : id_(0), callsign_(name) {}

	///Functor operator to process a single data point
	void operator()(std::pair<double, PIPlatformPoint> point)
	{
		// ID will be 0 only on first call; create the platform header in this case
		if (id_ == 0)
		{
			cout <<"SendPlatformPoint operator() id_:"<<id_<<endl;
			PIPlatformHeader hdr (callsign_);
			id_ = PIData::createPlatform (&hdr, &point.second);
			cout <<callsign_<<": Create new platform header for TrackData. ID: "<<id_<<endl;
		}
		else
		{
			PIData::addDataPoint (id_, &point.second);
			cout <<callsign_<<": ID exists :"<<id_<<".  Add data point for TrackData"<<endl;
		}
	}
};

void TrackData::sendToSIMDIS()
{
	// Create a functor object to send each data point to SIMDIS
	SendPlatformPoint sendPlatformPoint (callsign_);
	for_each (dataPoints_.begin(), dataPoints_.end(), sendPlatformPoint);
}
