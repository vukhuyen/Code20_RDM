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
#include "TrackMessage.h"


/**
* Time (secs) since the beginning of the year
* @return the current time in secs
*/
/*
inline double yeartime()
{
	timeb tb;
	time_t t = time(NULL);
	struct tm* gmt = gmtime(&t);
	tzset();
	gmt->tm_isdst = gmt->tm_mon = gmt->tm_min = gmt->tm_hour = 0;
	gmt->tm_sec = -timezone;
	gmt->tm_mday = 1;

	ftime(&tb);
	return (((double)(tb.time-mktime(gmt))) + ((double)tb.millitm / 1000.0));
}*/

/*
TrackMessage::TrackMessage()
{
}
TrackMessage::~TrackMessage()
{
}*/

void TrackMessage::getData(std::string s)
{
	std::istringstream stream;
	stream>>callsign_>>platIcon_>>id_>>refLat_>>refLon_>>tarLat_>>tarLon_;	
	std::cout<<"Read stream: "<<callsign_;
}

std::string TrackMessage::getCallsign(){
	return callsign_;
}

std::string TrackMessage::getIcon(){
	return this->platIcon_;
}

uint64_t TrackMessage::getId(){
	return this->id_;
}

double TrackMessage::getRefLat(){
	return this->refLat_;
}
double TrackMessage::getRefLon(){
	return this->refLon_;
}


