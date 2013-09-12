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

#include "PIIncludes.h"

#include "PlatformDialog.h"
#include "PlatformEngine.h"
#include "TrackData.h"

PIIMPLEMENT(PlatformDialog);	// Implement callback functionality

/**
* Dialog for manipulating platforms and points
*/

PlatformDialog::PlatformDialog(std::string title, PIIcon* icon) 
	:PIDialogBox(title, icon)
{
	PIVerticalFrame* vFrame = new PIVerticalFrame (this, PI_LAYOUT_FILL_X|PI_LAYOUT_FILL_Y);
	PIHorizontalFrame* hListBoxFrame = new PIHorizontalFrame (vFrame, PI_LAYOUT_FILL_X|PI_LAYOUT_FILL_Y);

	//Platform Header area list
	platformList_ = new PIList (hListBoxFrame, "Name", 75, PI_LAYOUT_FILL_X|PI_LAYOUT_FILL_Y);
	platformList_->appendHeader ("Id", 30);
	platformList_->mapFunction (CreateCallback (this, &PlatformDialog::onListSelection));

	//Buttons
	PIVerticalFrame* vListButtonFrame = new PIVerticalFrame (hListBoxFrame, PI_LAYOUT_CENTER_Y);
	refreshButton_ = new PIButton (vListButtonFrame, "Refresh", CreateCallback (this, &PlatformDialog::onCmdRefresh));
	loadPlatButton_ = new PIButton (vListButtonFrame, "View", CreateCallback (this, &PlatformDialog::onCmdLoadHeader));
	savePlatButton_ = new PIButton (vListButtonFrame, "Save", CreateCallback (this, &PlatformDialog::onCmdSaveHeader));
	new PIButton (vListButtonFrame, "Create", CreateCallback (this, &PlatformDialog::onCmdCreatePlat));
	mouseClickButton_ = new PIButton (vListButtonFrame, "Grab Mouse", CreateCallback(this, &PlatformDialog::onCmdGrabMouse));

	//Header
	PIGroupBox* headerBox = new PIGroupBox (vFrame, "Header Info", PI_LAYOUT_FILL_X|PI_FRAME_THICK);
	PIHorizontalMatrix* headerMatrix = new PIHorizontalMatrix (headerBox, 2, PI_LAYOUT_FILL_X);
	new PILabel (headerMatrix, "Callsign:");
	nameText_ = new PITextField (headerMatrix);
	new PILabel (headerMatrix, "Icon:");
	iconText_ = new PITextField (headerMatrix);
	new PILabel (headerMatrix, "Orig. ID:");
	originalIdText_ = new PITextField (headerMatrix);

	//Data Points
	PIGroupBox* dataPointBox = new PIGroupBox (vFrame, "Data Points", PI_LAYOUT_FILL_X|PI_FRAME_THICK);
	PIHorizontalMatrix* dataPointMatrix = new PIHorizontalMatrix (dataPointBox, 2, PI_LAYOUT_FILL_X);
	new PILabel (dataPointMatrix, "Time:");
	timeText_ = new PITextField (dataPointMatrix);
	new PILabel (dataPointMatrix, "Lat:");
	latText_ = new PITextField (dataPointMatrix);
	new PILabel (dataPointMatrix, "Lon:");
	lonText_ = new PITextField (dataPointMatrix);
	new PILabel (dataPointMatrix, "Alt:");
	altText_ = new PITextField (dataPointMatrix);
	new PILabel (dataPointMatrix, "Retrieval Method:");
	retrievalMenu_ = new PIOptionMenu (dataPointMatrix, NULL);
	retrievalMenu_->appendOption ("Current");


	//Update 
	PIHorizontalFrame* hDataPointButtons = new PIHorizontalFrame (vFrame, PI_LAYOUT_FILL_X);
	addPointButton_ = new PIButton (hDataPointButtons, "Add/Replace Data", CreateCallback (this, &PlatformDialog::onAddDataPoint), PI_FRAME_THICK|PI_LAYOUT_FILL_X);
	retrieveButton_ = new PIButton (hDataPointButtons, "Get at Time", CreateCallback (this, &PlatformDialog::onGetPoint), PI_FRAME_THICK|PI_LAYOUT_FILL_X);

	
}

/**
* Destroys the platform dialog
*/
PlatformDialog::~PlatformDialog()
{
}

///updates the state of the buttons
void PlatformDialog::updateButtonStates_()
{
	if (platformList_->getSelectedItem(0) == -1)
	{
		loadPlatButton_->disable();
		savePlatButton_->disable();
		//removePlatButton_->disable();
		addPointButton_->disable();
		retrieveButton_->disable();
		//recursiveFlushButton_->disable();
		//tspiFlushButton_->disable();
	}
	else
	{
		loadPlatButton_->enable();
		savePlatButton_->enable();
		//removePlatButton_->enable();
		addPointButton_->enable();
		retrieveButton_->enable();
		//recursiveFlushButton_->enable();
		//tspiFlushButton_->enable();
	}
}
///Configures the time state after adding or removing data
void PlatformDialog::checkTimeBounds_()
{
	if (PIData::clockGetMode() != PIData::PI_CLOCKMODE_FREEWHEEL)
	{
		PIData::clockAutoConfigureBeginEnd();
		double begin = PIData::clockGetBegin();
		double end = PIData::clockGetEnd();
		double now = PIData::clockGetTime();
		double newTime = now;
		if (newTime < begin)
			newTime = begin;
		if (newTime > end)
			newTime = end;
		if (now != newTime)
			PIData::clockSetTime (newTime);
	}
}

//*******************************************
///Public functions
//*******************************************

/**
*	Reload the current list of all platforms
*/
long PlatformDialog::onCmdRefresh (PIObject*, void*)
{
	platformList_->clearItems();
	std::vector<PIData::UniqueID_t> platList;
	PIData::getPlatformList (&platList);
	std::vector<PIData::UniqueID_t>::const_iterator iter;
	PIData::PIPlatformHeader header;
	char textString[256];
	for (iter = platList.begin(); iter != platList.end(); ++iter)
	{
		PIData::getHeader (*iter, &header);
		sprintf (textString, "%s\t%d", header.callsign.c_str(), (int)*iter);
		platformList_->appendItem (textString, (void*)((long)*iter));
	}
	updateButtonStates_();
	return 1;
}


/**
//When View button clicked, the plaform information is loaded into the form
*/
long PlatformDialog::onCmdLoadHeader (PIObject*, void*)
{
	int headerItem = platformList_->getSelectedItem(0);
	if (headerItem != -1){ // if there's data
		PIData::PIPlatformHeader header;
		PIData::getHeader((long)(platformList_->getItemData(headerItem)), &header);
		nameText_->setValue(header.callsign);
		iconText_->setValue(header.iconName);
		originalIdText_->setValue((int)header.originalID);
	}

	updateButtonStates_();
	return 1;
}

///Saves the current header info
long PlatformDialog::onCmdSaveHeader (PIObject*, void*)
{
	cout <<"Save button clicked..."<<endl;
	return 1;
}

///Saves a data point to current selection
long PlatformDialog::onAddDataPoint (PIObject*, void*)
{
	int headerItem = platformList_->getSelectedItem(0);
	if (headerItem != -1){
		PIData::UniqueID_t headerId = (long)(platformList_->getItemData(headerItem));
		PIPlatformPoint dataPoint;
		dataPoint.time = timeText_->getValueDouble(); //time which data is valid in seconds
		dataPoint.position.x = latText_->getValueDouble() * UTILS::CU_DEG2RAD; //coordinate on the x plane in radians
		dataPoint.position.y = lonText_->getValueDouble() * UTILS::CU_DEG2RAD;
		dataPoint.position.z = altText_->getValueDouble() * UTILS::CU_DEG2RAD;
		dataPoint.referenceFrame.coordSystem = PI_COORDSYS_LLA;
		PIData::addDataPoint (headerId, &dataPoint);

		double lat = dataPoint.position.x * UTILS::CU_RAD2DEG;
		cout<<"AddDataPoint to headerId: "<<headerId<<". New lat: "<<lat;

		checkTimeBounds_();
	}
	updateButtonStates_();
	return 1;
}

///Retrieves a data point from the current header
long PlatformDialog::onGetPoint (PIObject*, void*)
{
	int headerItem = platformList_->getSelectedItem (0);
	if (headerItem != -1)
	{
		PIData::UniqueID_t headerId = (long)(platformList_->getItemData(headerItem));
		PIPlatformPoint dataPoint;
		PIData::getDataPoint (headerId, timeText_->getValueDouble(), &dataPoint, (PIDataRetrievalMethod_t)(retrievalMenu_->getValueInt()));
		timeText_->setValue (dataPoint.time);
		latText_->setValue (dataPoint.position.x * UTILS::CU_RAD2DEG);
		lonText_->setValue (dataPoint.position.y * UTILS::CU_RAD2DEG);
		altText_->setValue (dataPoint.position.z * UTILS::CU_RAD2DEG);
	}
	updateButtonStates_();
	return 1;
}

/**
* Creates a new platform
*/
long PlatformDialog::onCmdCreatePlat (PIObject*, void*)
{
	PIData::PIPlatformHeader header;
	header.callsign = nameText_->getValueText();
	header.iconName = iconText_->getValueText();
	header.originalID = (PIData::UniqueID_t)(originalIdText_->getValueDouble());

	PIPlatformPoint dataPoint;
	dataPoint.time = timeText_->getValueDouble();
	dataPoint.position.x = latText_->getValueDouble() * UTILS::CU_DEG2RAD;
	dataPoint.position.y = lonText_->getValueDouble() * UTILS::CU_DEG2RAD;
	dataPoint.position.z = altText_->getValueDouble() * UTILS::CU_DEG2RAD;
	dataPoint.referenceFrame.coordSystem = PI_COORDSYS_LLA;

	// Create a new track, process the subtree, and send it to SIMDIS
	TrackData track(nameText_->getValueText());
	sendTrackToSimdis(&track);

	//PIData::createPlatform (&header, &dataPoint);

	checkTimeBounds_();
	updateButtonStates_();

	PIGUI::ungrabMouse();
	return 1;
}

long PlatformDialog::onCmdGrabMouse(PIObject*, void*)
{
	double lat = 0;
	double lon = 0;

	PIGUI::grabMouse();
	mouseClickButton_->disable();

	return 1;
}

///Process event
int PlatformDialog::handleMouseEvent (PIGUI::PIMouseEventType_t eventType, const PIGUI::PIMouseEvent& eventData)
{
	double lat = 0;
	double lon = 0;

	if (PISIMDIS::getMouseCursorLLA(&lat, &lon, NULL)==0){
		latText_->setValue(lat*UTILS::CU_RAD2DEG); //convert to degree
		lonText_->setValue(lon*UTILS::CU_RAD2DEG);
	}
	else
	{
		latText_->setValue (0.0);
		lonText_->setValue (0.0);
	}


	if (eventType==PI_MOUSEEVENT_LEFTPRESS)
	{		
		PIGUI::ungrabMouse();		
		mouseClickButton_->enable();
	}


	return 1;
}


void PlatformDialog::sendTrackToSimdis(TrackData* trackData)
{
	trackData->setCallsign(nameText_->getValueText());
	double timeVal = timeText_->getValueDouble();
	double lla[3];
	lla[0] = latText_->getValueDouble() * UTILS::CU_DEG2RAD;
	lla[1] = lonText_->getValueDouble() * UTILS::CU_DEG2RAD;
	lla[2] = altText_->getValueDouble() * UTILS::CU_DEG2RAD;
	int refYear = 2013;

	// Add the converted data to the trackData structure for later processing and transmission to SIMDIS
	trackData->addData (timeVal, refYear, lla[0], lla[1], lla[2]);
	// send track to simdis
	trackData->sendToSIMDIS();
}