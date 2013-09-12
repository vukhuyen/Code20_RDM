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

#ifndef PLATFORMDIALOG_H
#define PLATFORMDIALOG_H

#include "PIIncludes.h"
#include <vector>
#include <string>

class TrackData;

class PlatformDialog : public PIDialogBox
{
protected:

	TrackData* track;
	
	PIList* platformList_;			///List of all platforms

	PITextField* nameText_;			///Textfield for the name	
	PITextField* iconText_;			///Textfield for icon	
	PITextField* originalIdText_;	///Textfield for the original ID
	PITextField* latText_;			///Textfield for latitude	
	PITextField* lonText_;			///Textfield for longitude	
	PITextField* altText_;			///Textfield for altitude	
	PITextField* timeText_;			///Textfield for time	
	
	PIOptionMenu* retrievalMenu_;	///Option for retrieving time
	
	PIButton* refreshButton_;		///Button to refresh the platform list	
	PIButton* loadPlatButton_;		///Button to load selected platform	
	PIButton* savePlatButton_;		///Button to update the selected plat header
	PIButton* removePlatButton_;	///Button for removing platforms
	PIButton* addPointButton_;		///Button for adding new point
	PIButton* retrieveButton_;		///Button for retrieving points
	PIButton* mouseClickButton_;	///Button for grabbing the cursor lat lng
	//PIButton* createPlatButton_;	///Button for creating new platform (not necessary, created in .cpp)

	///Button to flush out platform recursively
	//PIButton* recursiveFlushButton_;
	///Button to flush out TSPI data
	//PIButton* tspiFlushButton_;


	
	///updates the state of the buttons
	void updateButtonStates_();
	///Configures the time state after adding or removing data
	void checkTimeBounds_();
	///Detects selected platform and flushes based on rule provided
	//void doFlush_(PIData::FlushType flushType);

public:
	///Creates a platform dialog
	PlatformDialog (std::string title, PIIcon* icon=NULL);
	///Destroys a platform dialog
	virtual ~PlatformDialog();

	///Override show
	virtual void show() {updateButtonStates_(); PIDialogBox::show();}

	///Refreshes the platform list
	long onCmdRefresh (PIObject*, void*);
	///Loads the selected header info
	long onCmdLoadHeader (PIObject*, void*);
	///Saves the current header info
	long onCmdSaveHeader (PIObject*, void*);
	///Removes a platform
	//long onCmdRemoveHeader (PIObject*, void*);
	///Saves a data point to current selection
	long onAddDataPoint (PIObject*, void*);
	///Retrieves a data point from the current header
	long onGetPoint (PIObject*, void*);
	///Called when list item is selected
	long onListSelection (PIObject*, void*) {updateButtonStates_(); return 1;}
	///Creates a new platform
	long onCmdCreatePlat (PIObject*, void*);
	///Flushes the selected platform's data recursively including children
	//long onFlushRecursive(PIObject*, void*);
	///Flushes the selected platform's TSPI data
	//long onFlushTspi(PIObject*, void*);
	    
    long onCmdGrabMouse (PIObject*, void*);		///Will grab the mouse

	    ///Process event
    int handleMouseEvent (PIGUI::PIMouseEventType_t eventType, const PIGUI::PIMouseEvent& eventData);

	///////////////////////
	void sendTrackToSimdis(TrackData* td);


};

#endif