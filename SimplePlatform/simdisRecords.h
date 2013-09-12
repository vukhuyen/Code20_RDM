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
 * Classes for defining SIMDIS input for range data.
 */

#ifndef SIMDISRECORDS_H
#define SIMDISRECORDS_H

#include <string>
#include <map>
#include "iostreamc"
#include "fstreamc"

//#include "CoordConvert/CoordConvert.h"
//#include "simCore\Calc\CoordinateConverter.h"
#include "NUWCPrefs.h"

#include "PIIncludes.h"
#include "rsdfRecords.h"
#include "color.h"

enum eRecordMode
{
  eRM_Stopped=0,
  eRM_Paused,
  eRM_Recording
};

typedef std::map<uint64_t, uint64_t> RecordMap;

class Scenario
{
    friend class Platform;

  private:
    size_t lineCount_; // range data (rtd files) can have > 80 K lines!
                    // long_max is +- 2147483647.  Should do it.
                    // use negative returns to flag errors below.
    size_t srCount_; // count of SR records
    size_t hrCount_; // count of HR records
    size_t hsCount_; // count of HS records
    size_t csCount_; // count of CS records
    size_t tcCount_; // count of TC records

    //TSRecord ts; // Tape security    (1 per "tape"), not in rsdf?
    //TRRecord tr; // Tape real ID     (1 per "tape"), not in rsdf?
    SRRecord sr_; // Run security     (1 per run)
    HRRecord hr_; // Run header       (1 per run)
    HSRecord hs_; // Run statistics   (1 per point count)
    PGRecord pg_; // Processed GPS    (n per point count)
    PPRecord pc_; // Processed CINESEXTANT (n per point count)
    PPRecord pp_; // Processed PSK    (n per point count)
    PFRecord pf_; // Processed SFSK   (n per point count)
    PRRecord pr_; // Processed radar  (n per point count)
    MKRecord mk_; // Marker   (n per point count)
    CSRecord cs_; // Check sum        (1 per point count), end of each HS block
    TCRecord tc_; // Trailer comments (1 per run), end of run, not in rsdf?

    void zeroCounts();
    bool resetNewRuns_;
    int curArray_;
    Run run_;        		// May be more than 1 per file

    int refYear_;		// reference year of data

    double lastDataTime_;	// last time that data was sent to simdis

    int runCount_;		// # of runs in input stream
    std::string runDesc_;	// description of current run
    PISIMDIS::vPrefRuleID newIDs_;

    NUWCPrefs *prefs_;
    eTimingMode timingMode_;	// time mode at scenario initialization

    // Mode configuration file
    std::string modeConfigFile_;
    
     // Create SIMDIS data point class
    bool initPlat(const tspiRecord &data);
    // Add the point to the SIMDIS display
    bool addPoint(uint64_t origID, const tspiRecord &data);
    PIPlatformPoint makePoint(const tspiRecord &data) const; // want inline

    // recording file
    fstream recordFile_;
    eRecordMode recordMode_;
    RecordMap recordMap_;

  public:
    std::map<uint64_t, uint64_t> markerIDs;		// map of original network based marker IDs to their new original IDs

  public:
    Scenario(NUWCPrefs *prefs);
    ~Scenario();
    void init();
    int newRun();
    void setRefYear();
    void setClock();
    bool putPoint(const tspiRecord &data);
    bool putMarker(const MKRecord &data);
    int clearPlatforms();

    void setClassif(std::string classif) const; // want inline

    double getLastDataTime() { return lastDataTime_;}
    void setLastDataTime(double t) { lastDataTime_ = t; }

    bool startRecording(const std::string &filename);
    void pauseRecording();
    void stopRecording();
    void recordPoint(uint64_t uniqID,
		     uint64_t pluginID,
		     PIPlatformPoint &point);
    

    void reset(bool rstnwrns, bool reinit=true);
    size_t processLine(const std::string &str, const std::string &firsttwo);
    size_t processFile(const std::string &filename);
    int getArray();
    PPRecord getPP();

    int sendGD(uint64_t origID,
	       double pointTime,
	       double pointExpireTime,
	       const std::string &tagString,
	       const std::string &dataString,
	       bool record);

    int sendCD(uint64_t uniqID,
	       double timeVal,
	       const std::string &categoryString,
	       const std::string &valueString,
	       bool record);
};

#endif /* SIMDISRECORDS_H */
