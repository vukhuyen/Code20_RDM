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
#ifndef PLATFORMLOADER_H
#define PLATFORMLOADER_H

#include "PIIncludes.h"
#include <vector>;

class PlatformEngine;
class PlatformDialog;



class SimplePlatform : public PIApplication //All SIMDIS plug-in applications may derive from the PIApplication class
{
protected:

	//Engine used to generate data: read data from a file or network
	PlatformEngine* platformEngine_;

	//GUI Dialog
	PlatformDialog* platformDialog_;


	PIDialogBox *dialog_;

	///Textfield for the name
	PITextField* nameText_;
	///Textfield for icon
	PITextField* iconText_;
	///Textfield for the original ID
	PITextField* originalIdText_;
	///Textfield for latitude
	PITextField* latText_;
	///Textfield for longitude
	PITextField* lonText_;
	///Button for adding new point
	PIButton* addPointButton_;

	PIButton *platformButton_;
	PIButton *startButton_;	///< Start Button
	PIButton *stopButton_;	///< Stop Button
	bool live_;

	std::vector<PIData::UniqueID_t>& trackList_;

	void createDialog_();		/// Helper function for creating the dialog
	
	void enableAndDisable_();	///Handles enabling and disabling various controls
	
	void startProcessing_ ();	///Initializes the scenario into a file or live mode
	
	void stopProcessing_();		///Stops any processing that might have been occurring

	void startProcessing_(std::vector<PIData::UniqueID_t>& trackList);

public:
	//constructor
	SimplePlatform();

	//destructor
	virtual ~SimplePlatform();

	/// The "constructor" called when the plugin is first loaded
	virtual int StartUp(std::vector<std::string>* args);
	/// The function called repeatedly by SIMDIS
	virtual int Callback();
	/// The "destructor" called when simdis is closed
	virtual int Close();
	/// A description for the plugin
	virtual std::string Name() {return "RDM_SimplePlatformPlugin";}

	///Open a given filename or server...
	virtual void openLocation (std::string location, int port=0, PIData::PINetworkProtocol_t protocol=PI_NETWORK_PROTOCOL_MULTICAST);
	
	///Unloads a file from memory
	virtual void closeLocation();
	
	///Display our "find server" window when user selects menu item in Plot-XY
	virtual void findNetworkLocation (std::string defaultServer, int defaultPort=0, PIData::PINetworkProtocol_t defaultProtocol=PI_NETWORK_PROTOCOL_MULTICAST);


	// The submenu items for the plugin
	/// Called when selected from the Plugins Menu
	long onCmdRunDialog (PIObject * menuItem, void* data);

	// Callback functions for dialog box inputs
	/// Correctly toggles the radial buttons and normal buttons
	//long onCmdRadioLiveMode (PIObject * menuItem, void* data);
	/// Correctly toggles the radial buttons and normal buttons
	//long onCmdRadioFileMode (PIObject * menuItem, void* data);

	
	long onCmdPlay (PIObject * button, void* data);/// Starts the selected mode
	
	long onCmdStop (PIObject * button, void* data);/// Stops live mode only (disabled for file mode)
	
	long onCmdPlatform(PIObject * button, void* data);/// Open Platform Dialog 
	///Creates a new platform
	//long onCmdCreatePlat (PIObject*, void*);

	
	void startLiveMode();/// Called when the live mode is to be started
	
	void startFileMode();/// Called when the file mode is to be started

	    ///Processes mouse events from the host
    int handleMouseEvent (PIGUI::PIMouseEventType_t eventType, const PIGUI::PIMouseEvent& eventData);


};

#endif