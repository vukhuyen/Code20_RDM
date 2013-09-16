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

#include <time.h>
#include <sys/timeb.h>
#include <string>
#include "stlstdlib.h"
#include "PlatformEngine.h"
#include "TrackData.h"


namespace{
	// Define a few constants

	///Define PI locally
	const double s_PI = 3.141592654;

	/**
	* Fixes angle between {0 < x < 2*PI} (rads)
	* @param x angle in degs
	* @return angle in rads > 0 and < 2*PI
	*/
	inline double ANGFIX(double x)
	{
		return ((x = fmod(x,(s_PI*2.0)))<0.0)?(x+(s_PI*2.0)):x;
	}

	///Reference latitude position in radians
	const double s_ReferenceLat = (22.0*(s_DEG2RAD));
	///Reference longitude position in radians
	const double s_ReferenceLon = (-159.0*(s_DEG2RAD));
}

PlatformEngine::PlatformEngine()
	: startTime_(0)
{	
}

PlatformEngine::~PlatformEngine()
{
}

///Initialize the engine with a starting time SIMULATOR
void PlatformEngine::initialize(double startTime)
{
	startTime_ = startTime;
	platformVec_.clear();
	lastPlatformTarget_ = 1;

	// Make sure that the clock is currently stopped
	PIData::clockStop();

	PIData::initializeScenario("Track Emulation",
		5555, //port number
		PI_NETWORK_PROTOCOL_MULTICAST, //protocol, ignored for now
		true); //live mode

	PIData::setDescription("RDM Platform Demo");
	PIData::setClassification("UNCLASSIFIED");


	// Set reference year of the scenario to the current system year	
	time_t t;
	t = time(NULL); ////get current time 
	struct tm* gmt = gmtime(&t);
	PIData::setReferenceYear(gmt->tm_year+1900);

	setLatitudeOrigin(s_ReferenceLat);
	setLongitudeOrigin(s_ReferenceLon);

	clockSetMode(PI_CLOCKMODE_FREEWHEEL, startTime_); 

	/* File mode
	clockSetMode(PI_CLOCKMODE_STEP, 0);
	clockSetBegin(startTime_);
	clockSetEnd(startTime_);
	clockSetTime(startTime_);*/

	// Go ahead and process the initial time
	advanceToTime(startTime_); 
}

///Initialize the engine with a starting time LIVE MODE
void PlatformEngine::initialize(double startTime, TrackData* track)
{
	startTime_ = startTime;

	// Make sure that the clock is currently stopped
	PIData::clockStop();

	// If the scenario is already initialized, no need to initialize again
	if (!PIData::isScenarioInitialized())
	{
		PIData::initializeScenario("Track Emulation",
			5555, //port number
			PI_NETWORK_PROTOCOL_MULTICAST, //protocol, ignored for now
			true); //live mode
		PIData::setDescription("RDM Platform Demo");
		PIData::setClassification("UNCLASSIFIED");
		PIData::setClassificationColor (PIRGBA (0, 255, 0, 128));
		PIData::setReferenceYear (getRefYear());

		setLatitudeOrigin(track->getTpsi()->getLat_()*(s_DEG2RAD));
		setLongitudeOrigin(track->getTpsi()->getLon_()*(s_DEG2RAD));
	}

	
	clockSetMode(PI_CLOCKMODE_FREEWHEEL, startTime_); 
	advanceToTime(startTime, track);
}


/**
* Creates a platform object with an initial data point
* @param newTime double 
*/


void PlatformEngine::createNewPlatform_(double newTime, TrackData* track)
{
	// The next platform ID is going to be 1 higher than the existing number of platforms
	size_t id = platformVec_.size() + 1;

	//Request a new data point for the platform
	PIPlatformPoint platPoint = calcPlatformPoint_(newTime, static_cast<int>(id));

	//Creat the platform by passing over its header and first data point
	PIData::UniqueID_t hostPlat = track->sendToSIMDIS();

	//Save the UniqueID for later use
	platformVec_.push_back(hostPlat);
}


/**
* Calculates a platform position based on its ID and current time
* @param timeVal Current idle time value
* @param platId Platform identifier, from 1 to s_MaxPlatforms inclusive

	REWRITE THIS FUNCTION TO WORK WITH REAL SCENARIO AND TRACK DATA

*/
PIData::PIPlatformPoint PlatformEngine::calcPlatformPoint_(double timeVal, int platId) const
{
	// Before we create the platform point, figure out (generate) the position
	double xpos = 100 * (sin(timeVal * 0.01));
	double ypos = 100 * (cos(timeVal * 0.01));
	double course = sin(ANGFIX((s_PI * .5) + (timeVal * 0.1)));

	// Each platform is separated by 100 meters
	PIPoint xyzPosition(xpos + (platId - 1) * 100, ypos + (platId - 1) * 100, 100);

	// Create a reference frame for the platform point using an XEast system
	PICoordReferenceFrame referenceFrame(PI_COORDSYS_XEAST,				// Coordinate system
		PIPoint(s_ReferenceLat, s_ReferenceLon, 0.0));	// Reference origin

	// Actually create the structure for the data point
	PIPlatformPoint ppoint(timeVal,			// When does event occur
		xyzPosition,			// Position of the aircraft
		PIPoint(course,0.,0.),		// Orientation
		PIPoint(0.,0.,0.),		// Velocity vector
		PIPoint(0.,0.,0.),		// Acceleration vector
		referenceFrame);


	return ppoint;
}

/**
* Sends a data point update for a given platform at a given time
* @param timeVal Time at which to send update
* @param platId Platform ID to update, 1 to s_MaxPlatforms
*/
void PlatformEngine::updatePlatform_(double timeVal, int platId)
{
	// Add data to the platform
	PIData::PIPlatformPoint platPoint = calcPlatformPoint_(timeVal, platId);
	PIData::addDataPoint(platId, &platPoint);		
	double lat = platPoint.position.x * UTILS::CU_DEG2RAD;
	double lng = platPoint.position.y * UTILS::CU_DEG2RAD;
	cout << "updatePlatform().  AddDataPoint to platId: " <<platId<<" lat: "<< lat <<" long: "<<lng<<endl;
}

///Advance the engine to a new time value; in simulator mode
void PlatformEngine::advanceToTime(double newTime)
{
	// Determine if a new platform needs to be added
	size_t numPlatforms = platformVec_.size();
	int s_MaxPlatforms = 10; //create 10 platforms

	if ((platformVec_.size() < s_MaxPlatforms) && (newTime - startTime_ >= numPlatforms))
	{
		// New platforms are created once a second		
		createNewPlatform_(newTime);
		numPlatforms++;
	}

	// Send updates to existing data objects
	int k;
	for (k = 0; k < static_cast<int>(numPlatforms); ++k)
		updatePlatform_(newTime, k + 1);
}

///Advance the engine to a new time value; in live mode
void PlatformEngine::advanceToTime(double newTime, TrackData* track){
	size_t numPlatforms = platformVec_.size();
	int k;
	for (k = 0; k < static_cast<int>(numPlatforms); ++k)
		createNewPlatform_(newTime, track);
}


///////////////////////
//	UTILS
//////////////////////
int getRefYear(){
	// Set reference year of the scenario to the current system year	
	time_t t;
	t = time(NULL); ////get current time 
	struct tm* gmt = gmtime(&t);
	return (gmt->tm_year+1900);
}