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

/*
PlatformLoader.cpp
SimplePlatform
A simple plugin to manage platform in simdis
*/

#include "PlatformLoader.h"
#include <time.h>
#include <sys/timeb.h>
#include "PlatformEngine.h"
#include "PlatformDialog.h"

// Required macro delcaration of the class being used
PI_DECLARE_APPLICATION (SimplePlatform)

static const int FILEMODE_DURATION = 1; // minutes

/**
* Time (secs) since the beginning of the year
* @return the current time in secs
*/
inline double yeartime()
{	
	timeb tb;
	time_t t = time(NULL); 
	
	struct tm* gmt = gmtime(&t); 
	tzset();
	gmt->tm_isdst = gmt->tm_mon = gmt->tm_min = gmt->tm_hour = 0;
	gmt->tm_sec = - timezone;
	gmt->tm_mday = 1;

	ftime(&tb);
	return (((double)(tb.time-mktime(gmt))) + ((double)tb.millitm / 1000.0));	
}

//Public constructor initializes all values to NULL
SimplePlatform::SimplePlatform()
	:PIApplication(),
	platformEngine_(NULL),
	platformDialog_(NULL),
	dialog_(NULL),
	//running_(false),
	startButton_(NULL),
	stopButton_(NULL)
{

}

//Virtual destructor
SimplePlatform::~SimplePlatform()
{
	delete platformDialog_;
}

int SimplePlatform::StartUp(std::vector<std::string>* args){
	// Adds a menu item under Plugins; when clicked, onCmdRunDialog() is called 
	new PIMenuItem(NULL, "Simple Platform...", CreateCallback (this, &SimplePlatform::onCmdRunDialog));

	return 0;
}

/// The function called repeatedly by SIMDIS
int SimplePlatform::Callback()
{
	// Update the data engine's data time.  Assumed LIVE MODE

	if (platformEngine_ != NULL)
	{
		static int skip = 0;
		if (++skip >= 10)
		{
			platformEngine_->advanceToTime (yeartime());
			skip = 0;
		}
	}
	return 0;
}

/// The "destructor" called when simdis is closed
int SimplePlatform::Close()
{	
	stopProcessing_();
	delete dialog_;
	dialog_ = NULL;
	return 0;
}

void SimplePlatform::findNetworkLocation (std::string defaultServer, int defaultPort, PIData::PINetworkProtocol_t defaultProtocol)
{
	// Create the window to show to the user, if it doesn't exist yet
	if (dialog_ == NULL)
		createDialog_();
	// Pop up the window
	dialog_->show();
}


void SimplePlatform::openLocation (std::string location, int port, PIData::PINetworkProtocol_t protocol)
{
	//if (running_)
	//	stopProcessing_();
	// Start a new connection depending on the "location"; string comes from the initializeScenario() in SSEngine
	startProcessing_();
}

/**
* Stops sending data to SIMDIS or Plot-XY on request of the user
*/
void SimplePlatform::closeLocation()
{
	stopProcessing_();
}

long SimplePlatform::onCmdRunDialog (PIObject * menuItem, void* data)
{
	// Create the window to show to the user, if it doesn't exist yet
	if (dialog_ == NULL)
		createDialog_();
	// Pop up the window
	dialog_->show();	
	return 1;
}

long SimplePlatform::onCmdPlatform(PIObject * button, void* data)
{	
	try{
		platformDialog_->show();
	}catch(std::exception e){
		cout<<e.what()<<endl;
	}
	return 1;
}

long SimplePlatform::onCmdPlay (PIObject * button, void* data)
{
	startProcessing_();
	enableAndDisable_();
	return 1;
}
long SimplePlatform::onCmdStop (PIObject * button, void* data)
{
	stopProcessing_();
	return 1;
}

////////////////////
void SimplePlatform::createDialog_()
{
	if (dialog_!=NULL)
		return;

	dialog_ = new PIDialogBox("Simple Platform Plugin", NULL, PI_ALL_NORMAL, 0, 200, 15, 15, 5, 5);
	PIVerticalFrame* vFrame = new PIVerticalFrame(dialog_, PI_LAYOUT_CENTER_X, 300);
	PIHorizontalFrame* hFrame = new PIHorizontalFrame(dialog_, PI_LAYOUT_CENTER_X, 300, 50, 0, 0, 5, 0, 45);	

	platformButton_ = new PIButton (hFrame, "View Platform", CreateCallback (this, &SimplePlatform::onCmdPlatform), PI_LAYOUT_FILL_X|PI_LAYOUT_BOTTOM);
	startButton_ = new PIButton(hFrame, "Start", CreateCallback(this, &SimplePlatform::onCmdPlay), PI_LAYOUT_CENTER_X, 60, 20);
	stopButton_ = new PIButton(hFrame, "Stop", CreateCallback(this, &SimplePlatform::onCmdStop), PI_LAYOUT_CENTER_X, 60, 20);

	try{
		platformDialog_ = new PlatformDialog("Platform Editor");
	}catch(std::exception e){
		cout <<e.what()<<endl;
	}

	enableAndDisable_();
}

void SimplePlatform::enableAndDisable_()
{
	if (dialog_==NULL)
		return;

	// Start button is enabled when SIMDIS has a scenario and the clock is in
	// freewheel or simulation mode; SimpleServer cannot interrupt another
	// DCS or plug-in live scenario, but it can 'overwrite' a file.
	if (PIData::isScenarioInitialized() &&
		(PIData::clockGetMode() == PI_CLOCKMODE_FREEWHEEL ||
		PIData::clockGetMode() == PI_CLOCKMODE_SIMULATION ))
		startButton_->disable();
	else
		startButton_->enable();
}

void SimplePlatform::startProcessing_()
{
	PICommon::beginWaitCursor();

	if (PIData::isScenarioInitialized()){
		cout << "A scenario is already initialized (by another data source) --> close it down" <<endl;
		PIData::deleteScenario();
	}

	if (platformEngine_ == NULL)
		platformEngine_ = new PlatformEngine;


	double currentTime = yeartime();
	platformEngine_->initialize(currentTime);
	cout <<"Initialize at the SIMDIS time: "<< currentTime <<endl;

	/*
	// For File mode, we simulate reading a data file that is five minutes long at
	// ten hertz update rate.  To do this, we initialize the clock to an arbitrary
	// time (say, now) and immediately and repeatedly call the advanceToTime().
	// Additionally, we do not set the "running" flag because we don't want data
	// continuously updating like we do in live mode.

	// 5 minutes, 60 seconds per minute, 10 hertz
	int numUpdates = FILEMODE_DURATION * 60 * 10;
	while (--numUpdates > 0)
	{
	currentTime += 0.1;
	platformEngine_->advanceToTime (currentTime);
	cout << "\nnumUpdates: " <<numUpdates;
	}


	// Now that all the updates have been sent, we close out
	PIData::clockAutoConfigureBeginEnd();
	PIData::clockSetTime (PIData::clockGetBegin());
	delete platformEngine_;
	platformEngine_ = NULL;
	*/

	// Stop the waiting cursor
	PICommon::endWaitCursor();	
}

/**
* Stops any processing that might have been occurring
*/
void SimplePlatform::stopProcessing_()
{
	// Setting the clock mode to STEP time after it was already in a
	// freewheel state will change the scenario from a live mode to a
	// post-process file mode where the user can adjust time and inspect data
	//if (running_)
	PIData::clockSetMode (PI_CLOCKMODE_STEP, PIData::clockGetBegin());
	PIData::clockStop();
	// Delete the data engine if it was instantiated
	delete platformEngine_;
	platformEngine_ = NULL;
	enableAndDisable_();
	//running_ = false;

	cout <<"Not running..."<<endl;
}

    ///Processes mouse events from the host
int SimplePlatform::handleMouseEvent (PIGUI::PIMouseEventType_t eventType, const PIGUI::PIMouseEvent& eventData){
	return platformDialog_->handleMouseEvent(eventType, eventData);
}
