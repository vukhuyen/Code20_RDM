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

// keep local constants and functions out of the global namespace
namespace
{
	// Define a few constants

	///Maximum number of platforms to create
	const size_t s_MaxPlatforms = 10;

	///Toggle duration for beams/gates on/off and generic data changes
	const double s_ToggleDuration = 4.0;

	///Define PI locally
	const double s_PI = 3.141592654;

	///Define DEG2RAD locally
	const double s_DEG2RAD = s_PI / 180.0;

	///Reference latitude position in radians
	const double s_ReferenceLat = (22.0*(s_DEG2RAD));
	///Reference longitude position in radians
	const double s_ReferenceLon = (-159.0*(s_DEG2RAD));

	///Death star coordinates in XEast meters
	const double s_DeathStarXY[] = {1000.0, -200.0};

	///Elevation angle in radians for beams and gates
	const double s_ElevationAngle = 15.0 * (s_DEG2RAD);

	///Colors for the beams to rotate through: RRGGBBAA components
	const PIColor_t bcolors[10] = {
		PIRGBA(255, 0, 0, 160), PIRGBA(255, 255, 255, 160), PIRGBA(0, 0, 255, 160), PIRGBA(0, 0, 0, 160),
		PIRGBA(255, 0, 0, 160), PIRGBA(0, 255, 0, 160), PIRGBA(0, 0, 255, 160), PIRGBA(255, 255, 0, 160),
		PIRGBA(255, 0, 255, 160), PIRGBA(0, 255, 255, 160)
	};
	///Colors for the gates to rotate through: RRGGBBAA components
	const PIColor_t gcolors[10] = {
		PIRGBA(0, 255, 255, 160), PIRGBA(255, 0, 255, 160), PIRGBA(255, 255, 0, 160), PIRGBA(0, 0, 255, 160),
		PIRGBA(0, 255, 0, 160), PIRGBA(255, 0, 0, 160), PIRGBA(0, 0, 0, 160), PIRGBA(255, 0, 0, 160),
		PIRGBA(255, 255, 255, 160), PIRGBA(0, 0, 255, 160)
	};

	/**
	* Fixes angle between {0 < x < 2*PI} (rads)
	* @param x angle in degs
	* @return angle in rads > 0 and < 2*PI
	*/
	inline double ANGFIX(double x)
	{
		return ((x = fmod(x,(s_PI*2.0)))<0.0)?(x+(s_PI*2.0)):x;
	}

	/// Define a callback function that will be used with PIData::regCBPreAuthorizePlatformEngine(); non-zero return ignores data point
	int authPlatDataCallback(PIData::UniqueID_t id, PIData::PIPlatformPoint* point, void* userData)
	{
		return static_cast<PlatformEngine*>(userData)->screenDataFromPlatform2(id, point);
	}

	// create a callback to demonstrate listening to new data
	void demoCallback(size_t id, PIData::UniqueID_t hostPlat, PlatformEngine *self)
	{
		// just for platform 2
		if (id == 2){
			PIData::regCBPreAuthorizePlatformData(hostPlat, authPlatDataCallback, self);
			cout <<"demoCallback for id 2 called"<<endl;
		}
	}
}

PlatformEngine::PlatformEngine()
	: startTime_(0)
{	
}

PlatformEngine::~PlatformEngine()
{
}

///Initialize the engine with a starting time
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

	clockSetMode(PI_CLOCKMODE_FREEWHEEL, startTime_); //live mode

	/* File mode
	clockSetMode(PI_CLOCKMODE_STEP, 0);
	clockSetBegin(startTime_);
	clockSetEnd(startTime_);
	clockSetTime(startTime_);*/

	// Go ahead and process the initial time
	advanceToTime(startTime_); //ORIG
}


int getRefYear(){
	// Set reference year of the scenario to the current system year	
	time_t t;
	t = time(NULL); ////get current time 
	struct tm* gmt = gmtime(&t);
	return (gmt->tm_year+1900);
}
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
		//PIData::setOriginLLA (originLLA[0], originLLA[1], originLLA[2]);
	}

	processPlatformVec(startTime, track);

	clockSetMode(PI_CLOCKMODE_FREEWHEEL, startTime_); //live mode
}


/**
* Creates a platform object with an initial data point
* @param newTime double 
*/
void PlatformEngine::createNewPlatform_(double newTime)
{
	// The next platform ID is going to be 1 higher than the existing number of platforms
	size_t id = platformVec_.size() + 1;

	// Figure out the callsign based on the ID
	std::stringstream tmpString;
	tmpString << id << " friendly_helo";

	//Create a platform header object to represent the platform
	PIPlatformHeader pHeader(tmpString.str(),		// Name/callsign of the platform displayed in SIMDIS
		id,											// User-defined original ID; here, the vector position
		"C:\\SIMDIS\\data\\models\\ntds\\large\\friendly_helo.gif");							// Icon to use in simdis

	//Request a new data point for the platform
	PIPlatformPoint platPoint = calcPlatformPoint_(newTime, static_cast<int>(id));

	//Creat the platform by passing over its header and first data point
	PIData::UniqueID_t hostPlat = PIData::createPlatform(&pHeader, &platPoint);	

	//Save the UniqueID for later use
	platformVec_.push_back(hostPlat);

	//demonstrate callback listening to new data
	demoCallback(id, hostPlat, this);

	//Add category data for affinity and plaftorm type
	//std::string type = platformCategory_.getAffinityStr(PlatformCategory::PlatformType.AIRCRAFT);
	PICategoryDataPoint platTypeCategory(-1, "Platform Type", "Aircraft");
	PICategoryDataPoint allegienceCategory(-1, "Affinity", "Friendly");
	PIData::addCategoryData(hostPlat, &platTypeCategory);
	PIData::addCategoryData(hostPlat, &allegienceCategory);

	//add track quality generic data, a value between 0 and 7; start it at 7
	PIData::PIGenericDataPoint tq(newTime, -1, "Track Quality", "7");
	PIData::createGenericData(hostPlat, &tq);

	//cout <<"\nNew platform created.  Category: " << &platTypeCategory.value;
	cout <<"\nNew Platform id: "<< id << "lat: " << (platPoint.position.x*UTILS::CU_RAD2DEG)<<"\nlon: " << (platPoint.position.y*UTILS::CU_RAD2DEG);
}

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
	double lat = platPoint.position.x * s_DEG2RAD;
	double lng = platPoint.position.y * s_DEG2RAD;
	cout << "updatePlatform().  AddDataPoint to platId: " <<platId<<" lat: "<< lat <<" long: "<<lng<<endl;
}

///Advance the engine to a new time value; call this function when idle

void PlatformEngine::advanceToTime(double newTime)
{
	// Determine if a new platform needs to be added
	size_t numPlatforms = platformVec_.size();//ORIG


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

void PlatformEngine::processPlatformVec(double newTime, TrackData* track){
	size_t numPlatforms = platformVec_.size();
	int k;
	for (k = 0; k < static_cast<int>(numPlatforms); ++k)
		createNewPlatform_(newTime, track);
}


int PlatformEngine::getPlatformList()
{
	std::vector<PIData::UniqueID_t> platList;
	PIData::getPlatformList (&platList);
	std::vector<PIData::UniqueID_t>::const_iterator iter;
	PIData::PIPlatformHeader header;
	char textString[256];
	for (iter = platList.begin(); iter != platList.end(); ++iter)
	{
		PIData::getHeader (*iter, &header);
		sprintf (textString, "%s\t%d", header.callsign.c_str(), (int)*iter);
	}
	cout <<"Platform list size: "<<platList.size()<<endl;
	return platList.size();
}

/**
* This function will get an announcement every time fuel data is added to SIMDIS.
* It will reject every 8th point, and apply a small velocity value to every other point
* @param id ID of the platform we get the callback for
* @param point Editable data point
* @return non-zero return ignores data point
*/
int PlatformEngine::screenDataFromPlatform2(PIData::UniqueID_t id, PIData::PIPlatformPoint* point) const
{
	static int number = 0;
	if (++number == 8) {
		number = 0;
		// Reject every 8th point
		return 1;
	}
	// Update every other point to contain a velocity based on the orientation
	double vel[3] = {25 * sin(point->orientation.x), 25 * cos(point->orientation.x), 0.0};
	point->velocity.set(vel[0], vel[1], vel[2]);
	cout <<"New velocity values set. x:"<<point->velocity.x<<endl;
	return 0;
}
