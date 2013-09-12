/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: NUWC Keyport
 *               Code 222
 *               610 Dowell Street
 *               Keyport WA, 98367
 *
 *
 * September 30, 2005 - NUWC Keyport
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 *
 */
/**
 * Class functions for defining SIMDIS input for range data.
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <mathc.h>

#include "fox/fx.h"

#include "simCore/Time/Utils.h"
#include "stlstdlib.h"
//#include "String/StringFormat.h"
//#include "CoordConvert/CoordConvert.h"
#include "simCore\String\Format.h"
#include "simCore\Calc\CoordinateConverter.h"

#include "ASIScenarioOutput.h"
#include "ASIPlatformOutput.h"
#include "ASICategoryOutput.h"
#include "ASIGenericOutput.h"

#include "PluginUtils.h"
#include "utils.h"
#include "simdisRecords.h"

using namespace std;

std::string getFullFilePath(const std::string& simdisDir, const std::string& dataFile)
{
  FXString viewFile;
#ifndef UNIX
  viewFile.format ("%s\\config\\NUWC\\%s", simdisDir.c_str(), dataFile.c_str());
#else
  viewFile.format ("%s/config/NUWC/%s", simdisDir.c_str(), dataFile.c_str());
#endif
  return viewFile.text();
}


/**
 * class Scenario
 */
Scenario::Scenario(NUWCPrefs *prefs)
  : sr_(),
    hr_(),
    hs_(),
    pg_(hs_),
    pf_(hs_),
    pp_(hs_),
    pc_(hs_),
    pr_(hs_),
    mk_(hs_),
    cs_(),
    tc_(),
    run_(sr_, hr_),
    curArray_(0),
    resetNewRuns_(false),
    runCount_(0),
    runDesc_(""),
    modeConfigFile_(""),
    lastDataTime_(0.0),
    refYear_(1900),
    recordMode_(eRM_Stopped),
    timingMode_(eTMODE_DATA)
{
  prefs_ = prefs;
  zeroCounts();
}

Scenario::~Scenario()
{
  clearPlatforms();
}

void Scenario::init()
{
  const PICoordReferenceFrame refFrame = prefs_->getRefFrame();
  PIData::setOriginLLA(refFrame.originLLA.x, refFrame.originLLA.y, refFrame.originLLA.z);
  PIData::setTangentPlaneOffsets(refFrame.tpXOffset, refFrame.tpYOffset, refFrame.tpAngle);
  
  if (prefs_->getRunningInSIMDIS())
  {
    if (!prefs_->getViewfile().empty() && PIData::isScenarioInitialized()) 
    {
      PISIMDIS::loadViewFile(getFullFilePath(prefs_->getSimdisDir(), prefs_->getViewfile()));
    }

    if (!prefs_->getRulefile().empty())
    {
      if (newIDs_.size())
      {
	for (size_t ii=0; ii<newIDs_.size(); ii++)
	{
	  PISIMDIS::removePrefRule(newIDs_[ii]);
	}
	newIDs_.clear();
      }
      PISIMDIS::loadPrefRuleFile(getFullFilePath(prefs_->getSimdisDir(), prefs_->getRulefile()), &newIDs_);
    }
  }
  else
  {
    if (PIData::isScenarioInitialized())
    {
      PIPlotXY::loadPMLScript(getFullFilePath(prefs_->getSimdisDir(), prefs_->getPMLFile()));
    }
  }
}

int Scenario::newRun()
{
  std::stringstream ss;
  ss << hs_.year;
  runDesc_ = prefs_->getRangeName() + ", " + ss.str();
  runDesc_ = runDesc_ +  ", MK " + run_.mark + run_.mod + "-" + run_.sequence;
  runDesc_ = runDesc_ + ", Register " + run_.regNum + ", Run plan " + run_.runPlan;

  setDescription(runDesc_);
  setClassif(run_.classif);

  // set once, use value throughout life of scenario to prevent timing errors
  // due to toggling between system and data time modes
  timingMode_ = prefs_->getTrackStatusTimingMode();

  return ++runCount_;
}

void Scenario::setRefYear()
{
  setReferenceYear(hs_.year);
}

void Scenario::setClock()
{
  clockSetMode(PI_CLOCKMODE_FREEWHEEL, hs_.yeartime);
}

// Release & clear platform & marker storage
int Scenario::clearPlatforms()
{
  prefs_->removeAllNonUserConfiguredPlatforms();
  markerIDs.clear();
  prefs_->setAllPlatformsInactive();
  return 0;
}

bool Scenario::putMarker(const MKRecord &data)
{
  if (markerIDs.count(data.id) > 0 && prefs_->isModeInPlatMap(markerIDs[data.id])) // already exists
  {
    if(prefs_->getPlatformUpdateVisible(markerIDs[data.id]) && prefs_->getMarkersVisible())
    {
      prefs_->setPlatformVisibility(markerIDs[data.id], prefs_->getMarkersVisible());
      prefs_->setPlatformUpdateVisible(markerIDs[data.id], false);
    }
    // update position
    return addPoint(markerIDs[data.id], data); // call simdis routine
  }
  // create new marker configuration
  uint64_t origID = prefs_->getUniqueMarkerID(); // set new original ID
  markerIDs[data.id] = origID;
  
  PlatformStruct plat = {0, origID, data.name,
			    "Marker",
			    "Neutral",
			    "x.opt",
			    "x.bmp",
			    PIRGBA(0,255,0,255),
			    true,
			    PIRGBA(0,255,0,255),
			    true,
			    -1,
			    prefs_->getMarkersVisible(),
			    PIPoint(0.0,0.0,0.0),    //offset  
			    NULL,    //last point
			    0.0,    //time last point
			    false,  //active
			    false,
			    true}; //user configured

  prefs_->setPlatform(origID, plat);

  PIPlatformPoint pp = makePoint(data);
  uint64_t uniqID = createPlatform(&PIPlatformHeader(plat.name,
						     plat.origID,
						     prefs_->getRunningInSIMDIS() ? plat.icon : plat.iconPxy),
						     &pp);  
  sendCD(uniqID, -1, "Record", data.rectype, true); // must be sent before setting uniq id in platform map
  std::stringstream str;
  str << data.id;
  sendCD(uniqID, -1, "Mode ID", str.str(), true);
  prefs_->setPlatformUniqID(origID, uniqID);

  if (prefs_->getPlatformUniqID(origID)>0)
  {
    prefs_->setPlatformLastPoint(origID, pp);
    prefs_->setPlatformTimeLastPoint(origID, pp.time);
    prefs_->setPlatformActive(origID, true);
  }
 
  recordPoint(origID, uniqID, pp);

  return (prefs_->getPlatformUniqID(origID) != 0);
}

bool Scenario::putPoint(const tspiRecord &data)
{
  if (prefs_->getPlatformUniqID(data.id) > 0) // platform exists in simdis/pxy
  {
    if (prefs_->getPlatformUpdateVisible(data.id) && prefs_->getMobilePositsVisible())
    {
      //reset visibilty so that platform shows up even after a clearscreen
      prefs_->setPlatformVisibility(data.id, prefs_->getMobilePositsVisible());
      prefs_->setPlatformUpdateVisible(data.id, false);
    }
    return addPoint(data.id, data); // send data to simdis
  }
  if (!prefs_->isModeInPlatMap(data.id)) // platform does not exist in map - create
  {
    PlatformStruct plat = {0, data.id, "ID_"+itoa(data.id),
			    "other",
			    "Neutral",
			    "unit_cube.opt",
			    "isotriangle.bmp",
			    PIRGBA(0,255,0,255),
			    true,
			    PIRGBA(0,255,0,255),
			    true,
			    -1,
			    prefs_->getMobilePositsVisible(),
			    PIPoint(0.0,0.0,0.0),    //offset  
			    NULL,    //last point
			    0.0,    //time last point
			    false,  //active
			    false,
			    true}; //user configured
    prefs_->setPlatform(data.id, plat);
  }

  // create plat in simdis/pxy and send prefs to simdis/pxy
  PIPlatformPoint pp = makePoint(data);
  PIData::UniqueID_t uniqID = createPlatform(&PIPlatformHeader(prefs_->getPlatformName(data.id),
							       data.id,
							       prefs_->getRunningInSIMDIS() ? prefs_->getPlatformIcon(data.id) : prefs_->getPlatformIconPxy(data.id)),
                                                               &pp);  
  sendCD(uniqID, -1, "Record", data.rectype, true); // CD must be sent before we set the uniq id in platform map
  std::stringstream str;
  str << data.id;
  sendCD(uniqID, -1, "Mode ID", str.str(), true);
  prefs_->setPlatformUniqID(data.id, uniqID); //set uniqID and update simdis/pxy with prefs
      
  if (prefs_->getPlatformUniqID(data.id)>0)
  {
    prefs_->setPlatformLastPoint(data.id, pp);
    prefs_->setPlatformTimeLastPoint(data.id, pp.time);
    prefs_->setPlatformActive(data.id, true);
  }
  
  recordPoint(data.id, uniqID, pp);

  return (prefs_->getPlatformUniqID(data.id) != 0);
}

bool Scenario::addPoint(uint64_t origID, const tspiRecord &data)
{
  PIPlatformPoint point = makePoint(data);
  PIData::UniqueID_t id = prefs_->getPlatformUniqID(origID);
  PIData::addDataPoint(id, &point); // send data to simdis
  recordPoint(origID, id, point);
  prefs_->setPlatformLastPoint(origID, point); // store last data point - in LLA
  prefs_->setPlatformTimeLastPoint(origID, point.time);
  lastDataTime_ = point.time;
  if (!prefs_->getPlatformActive(origID))
  {
    // has not been updated yet 
    prefs_->getRunningInSIMDIS() ? prefs_->updateSimdisWithPlatformPrefs(origID) : prefs_->updatePlotXYWithPlatformPrefs(origID);
    prefs_->setPlatformActive(origID, true);
  }
  return true;
}

PIPlatformPoint Scenario::makePoint(const tspiRecord &data) const
{
  // Return the data point needed by SIMDIS
  double time = (timingMode_ == eTMODE_DATA || !prefs_->getLiveMode()) ? data.yeartime : data.toa;
  double pos[3] = {data.x * UTILS::CU_FT2M,
		   data.y * UTILS::CU_FT2M,
		   0};

  // depth/alt reported by telemetry; not referenced to GTP
  double tmZ = data.z * UTILS::CU_FT2M;

  // convert RSDF mixed coordinate system into a common system
  CoordValues lla;
  prefs_->getCoordConvert()->convert(CoordValues(simCore::COORD_SYS_GTP, pos),
					   lla, simCore::COORD_SYS_LLA);
  
  // course relative to True North
  double crs = data.course * UTILS::CU_DEG2RAD;
  double ori[3] = {crs, 0, 0};

  double spd = data.speed * UTILS::CU_KTS2MS;
  double vel[3] = {spd * sin(crs),
		   spd * cos(crs),
		   0};
  
  // input reference frame to SIMDIS/PXY is now LLA
  PICoordReferenceFrame refFrame;
  refFrame = prefs_->getRefFrame();
  refFrame.coordSystem = PIData::PI_COORDSYS_LLA;

  // RSDF TSPI records only contain position, course and speed
  PIPlatformPoint point(
    time,					// time, seconds
    PIPoint(lla.x(), lla.y(), tmZ),	        // lat & lon radians, replace TM z value
    PIPoint(crs, 0, 0),				// heading, yaw/pit/rol, radians
    PIPoint(vel[0], vel[1], 0),			// velocity, m/s
    PIPoint(),					// acceleration, m/s^2
    refFrame					// entire coord reference
    );  
  return point;
}

void Scenario::setClassif(std::string runClass) const
{
  if (prefs_->getOverrideDataClassification())
  {
    setClassification(prefs_->getDisplayClassification());
    setClassificationColor(prefs_->getDisplayClassificationColor());
    return;
  }

  // Code rewritten so that classification retains highest value set - bja

  enum classifOrder { LOWEST, UNKNOWN, UNC, CON, SEC, TS };

  PIColor_t color[6]; //sizeof(enum classifOrder) / sizeof(LOWEST)]; (sizeof fails)
  color[UNKNOWN] = 0x00ff0080;
  color[UNC]     = 0x00ff0080;
  color[CON]     = 0x0000ff80;
  color[SEC]     = 0xff000080;
  color[TS]      = 0xff000080;

  std::map<std::string, int> cmap;
  std::map<std::string, int>::const_iterator cmap_ii;
  cmap["unc"] = UNC;
  cmap["con"] = CON;
  cmap["sec"] = SEC;
  cmap["top"] = TS;

  int newClass;
  cmap_ii = cmap.find(simCore::lowerCase(runClass.substr(0,3)));

  if (cmap_ii == cmap.end()) // don't recognize
  {
    newClass = UNKNOWN;
  }
  else
  {
    newClass = cmap_ii->second;
  }

  int curClass;
  std::string curClassStr = getClassification();
  cmap_ii = cmap.find(simCore::lowerCase(curClassStr.substr(0,3)));

  if (cmap_ii == cmap.end()) // don't recognize
  {
    curClass = LOWEST; // replace it
  }
  else
  {
    curClass = cmap_ii->second;
  }

  if (newClass > curClass)
  {
    setClassification(runClass);
    setClassificationColor(color[newClass]);
  }
}

bool Scenario::startRecording(const std::string &filename)
{
  if (PIData::isScenarioInitialized() && !filename.empty())
  {
    recordFile_.open(filename.c_str(), ios::out);
    if (!recordFile_ || !recordFile_.good())
    {
      return false;
    }
    recordMap_.clear();
    saveASIScenarioRequired(recordFile_, prefs_->getRefLat(),
			    prefs_->getRefLon(),
			    prefs_->getRefAlt(),
			    UTILS::eCOORD_SYS_LLA, refYear_, 0,
			    prefs_->getGTPXOffset(),
			    prefs_->getGTPYOffset(),
			    prefs_->getGTPRotation());
    saveASIClassification(recordFile_,
                          PIData::getClassification(),
                          PIRGBATOABGR(PIData::getClassificationColor()));
    saveASIScenarioInfo(recordFile_, runDesc_);
    saveASIDegreeAngles(recordFile_, true);
    // TODO:  Provide support for relative file paths, if desired by NUWC
    saveASIExternalFiles(recordFile_, getFullFilePath(prefs_->getSimdisDir(), prefs_->getViewfile()), eVIEW);
    saveASIExternalFiles(recordFile_, prefs_->getTerrainConfiguration(prefs_->getImageryResolution()), eITCONFIG);
    saveASIExternalFiles(recordFile_, getFullFilePath(prefs_->getSimdisDir(), prefs_->getRulefile()), eRULE);
    saveASIExternalFiles(recordFile_, prefs_->getArrayFile(), eGOG);
    saveASIExternalFiles(recordFile_, prefs_->getArrayNumbers(), eGOG);
    saveASIExternalFiles(recordFile_, prefs_->getRangeBoundary(), eGOG);
    saveASIExternalFiles(recordFile_, prefs_->getRangeContour(), eGOG);
    recordMode_ = eRM_Recording;
    return true;
  }
  return false;
}

void Scenario::pauseRecording()
{
  if (recordMode_ == eRM_Recording)
  {
    recordMode_ = eRM_Paused;
  }
  else if (recordMode_ == eRM_Paused)
  {
    recordMode_ = eRM_Recording;
  }
}

void Scenario::stopRecording()
{ 
  if (recordFile_.good())
  {
    recordFile_.close();
  }
  recordMode_ = eRM_Stopped;
}

void Scenario::recordPoint(uint64_t id, uint64_t uniqID, PIPlatformPoint &pp)
{
  if (recordMode_ != eRM_Recording || !prefs_->getLiveMode())
  {
    // not in record mode
    return;
  }
  if (recordMap_.count(uniqID) == 0)
  {
    PIPlatformHeader platHeader;
    if (PIData::getHeader(uniqID, &platHeader) != 0)
    {
      // error occurred
      std::cerr << "Error occurred requesting PIPlatformHeader for recording..." << std::endl;
      return;
    }
      
    recordMap_[uniqID] = uniqID;
    // use SIMDIS icon for ASI recording
    saveASIPlatformRequired(recordFile_,
			    uniqID,
			    platHeader.callsign,
			    prefs_->getPlatformIcon(id));
    saveASIPlatformOriginalID(recordFile_,
			      uniqID,
			      id);

    std::vector< PICategoryDataPoint > catDataPnts;
    if (PIData::retrieveAllCategoryData(uniqID, &catDataPnts) != 0)
    {
      size_t i;
      for (i=0; i<catDataPnts.size(); ++i)
      {
	saveASICategoryData(recordFile_,
			    uniqID,
			    catDataPnts[i].category,
			    catDataPnts[i].value,
			    catDataPnts[i].time,
			    simCore::TIMEFORMAT_ORDINAL,
			    refYear_);
      }
    }
    std::vector< PIGenericDataPoint > gdVec;
    if (PIData::getDataPoints(uniqID, &gdVec) == 0)
    {
      size_t i;
      for (i=0; i<gdVec.size(); ++i)
      {
	saveASIGenericData(recordFile_,
			   uniqID,
			   gdVec[i].tag,
			   gdVec[i].data,
			   gdVec[i].time,
			   gdVec[i].expireTime,
			   simCore::TIMEFORMAT_ORDINAL,
			   refYear_);
      }
    }
  }
  double pos[3]={pp.position.x, pp.position.y, pp.position.z};
  double ori[3]={pp.orientation.x, pp.orientation.y, pp.orientation.z};
  double vel[3]={pp.velocity.x, pp.velocity.y, pp.velocity.z};
  saveASIPlatformData(recordFile_,
		      uniqID,
		      pp.time,
		      pos, ori, vel, 0,
		      true, true,
		      simCore::TIMEFORMAT_ORDINAL,
		      refYear_);
}

int Scenario::sendGD(uint64_t origID,
		     double pointTime,
		     double pointExpireTime,
		     const std::string &tagString,
		     const std::string &dataString,
		     bool record)
{
  uint64_t uniqID = prefs_->getPlatformUniqID(origID);
  PIData::PIGenericDataPoint gdp(pointTime, pointExpireTime, tagString, dataString);
  int rv = PIData::createGenericData(uniqID, &gdp);
  if (recordMode_ == eRM_Recording && record && rv == 0 && prefs_->getLiveMode())
  {
    saveASIGenericData(recordFile_,
		       uniqID,
		       tagString,
		       dataString,
		       pointTime,
		       pointExpireTime,
		       simCore::TIMEFORMAT_ORDINAL,
		       refYear_);
  }
  return rv;
}

int Scenario::sendCD(uint64_t uniqID,
		     double timeVal,
		     const std::string &categoryString,
		     const std::string &valueString,
		     bool record)
{
  PIData::PICategoryDataPoint cdp(timeVal, categoryString, valueString);
  int rv = PIData::addCategoryData(uniqID, &cdp);
  if (recordMode_ == eRM_Recording && record && rv == 0 && prefs_->getLiveMode())
  {
    saveASICategoryData(recordFile_,
			uniqID,
			categoryString,
			valueString,
			-1,
			simCore::TIMEFORMAT_ORDINAL,
			refYear_);
  }
  return rv;
}

void Scenario::zeroCounts()
{
  lineCount_ = 0;
  srCount_ = hrCount_ = hsCount_ = csCount_ = tcCount_ = 0;
}

// Reset the data counters and scenario (clear platforms, etc.)
void Scenario::reset(bool rnr, bool reinit)
{
  resetNewRuns_ = rnr;
  zeroCounts();
  clearPlatforms();
  if(reinit)
  {
    init();
  }
}

// Process the data file
size_t Scenario::processFile(const std::string &filename)
{
  std::ifstream infile(filename.c_str(), ios::binary);
  if (!infile.is_open())
  {
    cout << "Failed to open file: " << filename << endl;
    return 1;
  }

  // get length of file:
  infile.seekg(0, ios::end);
  std::streamoff fileLength = infile.tellg();
  infile.seekg(0, ios::beg);

  if (fileLength <=0)
  {
    infile.close();
    cout << "Failed to load file: " << filename << endl;
    return 1;
 }

  // allocate memory:
  char *fileBuffer = new char[fileLength];

  // read data as a block:
  infile.read(fileBuffer, fileLength);
  infile.close();

  // tokenize incoming data records
  std::vector<std::string> rsdfVec;
  std::string rsdfStr;
  int i;
  bool beginRecord = false;
  for (i=0; i<fileLength; i++)
  {
    if (fileBuffer[i] == '<' && beginRecord == false)
    {
      rsdfStr = fileBuffer[i];
      beginRecord = true;
    }
    else if (fileBuffer[i] == '<' && beginRecord == true)
    {
      rsdfVec.push_back(rsdfStr);
      rsdfStr = fileBuffer[i];
    }
    else if (beginRecord)
    {
      rsdfStr += fileBuffer[i];
    }
  }

  // remove allocated memory
  if (fileBuffer) delete fileBuffer;

  // process individual records
  size_t ic = 0;
  for (ic=0; ic<rsdfVec.size(); ic++)
  {
    if (rsdfVec[ic].size() > 13)
    {
      std::string rsdfLine = rsdfVec[ic];
      std::vector<std::string> vecStr;
      if (validChecksum(rsdfLine.c_str(),
                        rsdfLine.size(),
                        vecStr,
                        prefs_->getValidateCS(),
                        prefs_->getVerboseMsgs()))
      {
        // Process each record
        size_t ii = 0;
        for (ii=0; ii<vecStr.size(); ii++)
        {
          if (vecStr[ii].size() > 2)
          {
            std::string firsttwo = vecStr[ii].substr(0,2);
            // prevent processing of CS record and rsdf header
            if (firsttwo != "CS" && !strstr(vecStr[ii].c_str(), "<rsdf"))
            {
              size_t rtn = processLine(vecStr[ii], firsttwo);
              if (rtn != 0)
              {
                cout << "The following error occurred processing file: " << rtn << endl;
              }
            }
          }
        }
      }
    }
  }
  return 0;
}

int Scenario::getArray()
{
  return curArray_;
}

PPRecord Scenario::getPP()
{
  return pp_;
}

size_t Scenario::processLine(const std::string &str, const std::string &firsttwo)
{
  lineCount_++;
  curArray_ = -1;

  if (firsttwo == "SR")
  {
    // should be 1 per run
    hsCount_ = 0; // reset to flag new run
    ++srCount_;
    sr_.get(str);
  }
  else if (firsttwo == "HR")
  {
    // should be 1 per run, after SR
    // reset to flag new run, 2nd time to make sure
    hsCount_ = 0;
    ++hrCount_;
    hr_.get(str);
    run_.New();
  }
  else if (firsttwo == "HS")
  {
    // one per point count, 1st record
    curArray_ = -2;

    ++hsCount_;
    hs_.get(str);

    refYear_ = (timingMode_ == eTMODE_DATA) ? hs_.year : simCore::currentYear();

    // Set simdis year to next year when it changes
    // (Will probably never be used, since is Jan 1)
    if (hs_.day == 1 && hs_.month == 1 && hs_.hour == 0 && hs_.mins < 1)
    {
      setRefYear();
    }

    if ( hsCount_ == 1 )
    {
      newRun(); // do here, since if live mode, won't see HR.

      if (srCount_ == 0) ++srCount_; // Will be zero in live mode, so bump
      if (hrCount_ == 0) ++hrCount_; // to make consistent with file mode.

      // timing set based on incoming data
      if (prefs_->getTrackStatusTimingMode() == eTMODE_DATA)
      {
 	setRefYear();
        setClock();
      }
      
      if (hrCount_ == 1 || resetNewRuns_)
      { // 1st run, or reset each run
        // In live mode, an hr record is recieved when
        // the range starts recording.  When that happens,
        // a new map will be created, which will duplicate
        // already existing platforms.
        clearPlatforms(); 
      }
    } // if hsCount == 1
    else
    {
      // new HS record received
      setClassif(hs_.classif);
    }
  }
  else if (firsttwo == "PG")
  {// n per point count
    pg_.get(str);

    if ( ! putPoint((tspiRecord) pg_) )
    {
      cout << "Failed to process PG record. Line # " << lineCount_ << endl;
      return -3;
    }
  }
  else if (firsttwo == "PC")
  {// n per point count
    pc_.get(str);

    if ( ! putPoint((tspiRecord) pc_) )
    {
      cout << "Failed to process PC record. Line # " << lineCount_ << endl;
      return -3;
    }
  }
  else if (firsttwo == "PF")
  {// n per point count
    pf_.get(str);

    if ( ! putPoint((tspiRecord) pf_) )
    {
      cout << "Failed to process PF record. Line # " << lineCount_ << endl;
      return -3;
    }
    double time = (timingMode_ == eTMODE_DATA || !prefs_->getLiveMode()) ? pf_.yeartime : pf_.toa;
    std::stringstream tmBits;
    tmBits << hex << pf_.tmWord;
    sendGD(pf_.id, time, -1, "Telemetry word", tmBits.str(), true);
    std::stringstream sIDs;
    sIDs << pf_.sensorID1 << ", " 
         << pf_.sensorID2 << ", "
         << pf_.sensorID3 << ", "
         << pf_.sensorID4 << ", "
         << pf_.sensorID5;
    sendGD(pf_.id, time, -1, "Sensor IDs (1-5)", sIDs.str(), true);
  }
  else if (firsttwo == "PP")
  {// n per point count
    curArray_ = pp_.get(str);	// get() was modified to return the array number

    if ( !putPoint((tspiRecord) pp_) )
    {
      cout << "Failed to process PP record. Line # " << lineCount_ << endl;
      return -4;
    }
    double time = (timingMode_ == eTMODE_DATA || !prefs_->getLiveMode()) ? pp_.yeartime : pp_.toa;
    std::stringstream sRng;
    sRng << pp_.slantRng;
    sendGD(pp_.id, time, -1, "Slant range to array center (ft)", sRng.str(), true);
    std::stringstream fBits;
    fBits << pp_.framingBits;
    sendGD(pp_.id, time, -1, "Framing bits", fBits.str(), true);
    std::stringstream tmBits;
    tmBits << hex << pp_.tmBits;
    sendGD(pp_.id, time, -1, "Telemetry bits", tmBits.str(), true);
  }
  else if (firsttwo == "PR")
  {// n per point count
    pr_.get(str);

    if ( ! putPoint((tspiRecord) pr_) )
    {
      cout << "Failed to process PR record. Line # " << lineCount_ << endl;
      return -5;
    }
  }
  else if (firsttwo == "MK")
  {
    mk_.get(str);

    if ( !putMarker(mk_) )
    {
      cout << "Failed to process MK record. Line # " << lineCount_ << endl;
      return -3;
    }
  }
  else if (firsttwo == "TC")
  {
    if (tc_.get(str))
    {
      cout << "Failed to process TC record. Line # " << lineCount_ << endl;
      return -6;
    }
    
    if (!tc_.comments.empty())
    {
      sendGD(0, PIData::clockGetTime(), -1, "Comments", tc_.comments, true);
    }
  }
  else
  {
    // No recognized string
    if (!strstr(str.c_str(), "<rsdf"))
    {
      cout << "Unknown RSDF Message: " << str << endl;
    }
  }

  return 0; //success
}
