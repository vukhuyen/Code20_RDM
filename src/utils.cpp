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
 * Utilities
 */

#include <sstream>
#include "fstreamc"
#include "iostreamc"
#include <iomanip>
#include <algorithm>

#ifndef UNIX
#include <winsock2.h>
#endif

#include "timec.h"
#include "mathc.h"

#include "fox/fx.h"

#include "CoordConvert/CoordConvert.h"
#include "String/StringFormat.h"
#include "Time/TimeUtils.h"


#include "utils.h"


/**
 * Get the current system time, using timezone value of 0
 * @returns the UTC time
 */
double getSystemTime()
{
  timeb tb;
  ftime(&tb);
  return (double)(tb.time + (double)tb.millitm / 1000.);
}

/**
 * Return Julian Day
 * @returns the julian day
 */
int juldayloc()
{
  time_t t = time(NULL);
  struct tm* loctime = localtime(&t);
  return (loctime->tm_yday + 1);
}

/**
 * Return four-digit year
 * @returns the four digit year
 */
int yearloc()
{
  time_t t = time(NULL);
  struct tm* loctime = localtime(&t);
  return (loctime->tm_year + 1900);
}

/**
 * Return time (secs) since the beginning of the year using localtime and daylight savings
 * @return the current time in secs
 */

double getYearTime()
{
  double secs;
  struct timeval tp;
  struct timezone tz;

  // get the current system time
  gettimeofday(&tp, &tz);

  // put system time into a tm struct
  time_t t(tp.tv_sec);
  struct tm* p_time = gmtime(&t);

  // assemble a UTC "system time"
  unsigned int p_secs = (unsigned int)(p_time->tm_sec )  +
    (((unsigned int)(p_time->tm_min )) * 60) +
    (((unsigned int)(p_time->tm_hour)) * 3600) +
    (((unsigned int)(p_time->tm_yday)) * 86400);

  secs = (p_secs + (tp.tv_usec * 1e-06));
  // fix timezone to be local
  secs -= tz.tz_minuteswest*60;
  // account for daylight savings time
  secs += tz.tz_dsttime ? 60*60 : 0;

  return secs;
}

/**
 * Correct RTD/CRDF-file 2-digit year to 4 digit year
 *   RSDF doesn't seem to contain 2-digit year.
 */
int CorrectYear(int year)
{
  if (40 < year && year <= 99)
    year += 1900;
  else if (0 <= year && year <= 40) // fails at 2041!
    year += 2000;
  else
    year = -1; //flag bad

  return year;
}

/**
 * Return OS specific path
 * @param file base filename
 * @param relPath file path, relative to SIMDIS install location
 * @param simdisDir SIMDIS install location
 * @returns OS specific path to requested file
 */
std::string getFilePath(std::string file,
			std::string relPath,
			const std::string& simdisDir)
{
  if (FXStat::exists(file.c_str()))
  {
    return file;
  }

  std::stringstream fileStr;
  if (FXStat::isDirectory(relPath.c_str()))
  {
    fileStr << relPath << "/"  << file; 
  }
  else
  {
    fileStr << simdisDir << "/" << relPath << "/" << file; 
  }
  
#ifndef UNIX
  // convert front slash to back slash for MS cmd shell
  return UTILS::sub(fileStr.str(), "/", "\\");
#else
  return UTILS::backslashToFrontslash(fileStr.str());
#endif
}

UTILS::ECoordSystemType getCoordSystemFromStr(std::string cs)
{
  if(cs.compare("NED")==0) return UTILS::eCOORD_SYS_NED;
  if(cs.compare("NWU")==0) return UTILS::eCOORD_SYS_NWU;
  if(cs.compare("LLA: DMS")==0) return UTILS::eCOORD_SYS_LLA_DMS;
  if(cs.compare("LLA: DM.D")==0) return UTILS::eCOORD_SYS_LLA_DMD;
  if(cs.compare("LLA: D.D")==0) return UTILS::eCOORD_SYS_LLA;
  if(cs.compare("ECEF")==0) return UTILS::eCOORD_SYS_ECEF;
  if(cs.compare("X-East")==0) return UTILS::eCOORD_SYS_XEAST;
  if(cs.compare("ECI")==0) return UTILS::eCOORD_SYS_ECI;
  if(cs.compare("Generic")==0) return UTILS::eCOORD_SYS_GTP;
  return UTILS::eCOORD_SYS_ENU;
}

std::string getCoordSystemStr(UTILS::ECoordSystemType cs)
{
  switch(cs)
  {
    case UTILS::eCOORD_SYS_NED:  return "NED";
    case UTILS::eCOORD_SYS_NWU:  return "NWU";
    case UTILS::eCOORD_SYS_LLA_DMS:  return "LLA: DMS";
    case UTILS::eCOORD_SYS_LLA_DMD:  return "LLA: DM.D";
    case UTILS::eCOORD_SYS_LLA:  return "LLA: D.D";
    case UTILS::eCOORD_SYS_ECEF:  return "ECEF";
    case UTILS::eCOORD_SYS_XEAST:  return "X-East";
    case UTILS::eCOORD_SYS_ECI:  return "ECI";
    case UTILS::eCOORD_SYS_GTP:  return "Generic";
    default: return "ENU";
  }
}

UTILS::MagVarType_t getMagVarFromStr(std::string mv)
{
  if(mv.compare("TRUE")==0) return UTILS::eMAGDATUM_TRUE;
  if(mv.compare("WMM")==0) return UTILS::eMAGDATUM_WMM;
  return UTILS::eMAGDATUM_USER;
}

std::string getMagVarStr(UTILS::MagVarType_t mv)
{
  switch(mv)
  {
    case UTILS::eMAGDATUM_TRUE: return "TRUE";
    case UTILS::eMAGDATUM_WMM: return "WMM";
    default: return "USER";
  }
}

UTILS::VerticalDatumType_t getVertDatFromStr(std::string vd)
{
  if(vd.compare("HAE")==0) return UTILS::eVERTDATUM_WGS84;
  if(vd.compare("MSL")==0) return UTILS::eVERTDATUM_MSL;
  return UTILS::eVERTDATUM_USER;
}

std::string getVertDatStr(UTILS::VerticalDatumType_t vd)
{
  switch(vd)
  {
  case UTILS::eVERTDATUM_WGS84: return "HAE";
  case UTILS::eVERTDATUM_MSL: return "MSL";
  default: return "USER";
  }
}

bool validChecksum(const char *iobuf, size_t iobytes, 
                   std::vector<std::string> &svec, 
                   bool validateCS, bool verbose)
{
  // tokenize incoming data records
  svec = UTILS::split(iobuf, "\r\n", -1);
  size_t ii;
  int csIoBlockSize = 0;
  int csValue = 0;
  for (ii=0; ii<svec.size(); ii++)
  {
    svec[ii] = UTILS::trimWhiteSpace(svec[ii]);
    // find checksum record
    if (svec[ii].size() > 2)
    {
      std::string firsttwo = svec[ii].substr(0,2);
      if (firsttwo == "CS")
      {
        std::vector<std::string> csVec = UTILS::split(svec[ii], " \t", -1);
        if (csVec.size() > 2)
        {
          csIoBlockSize = atoi(csVec[1].c_str());
          csValue = atoi(csVec[2].c_str());
        }
      }
    }
  }

  // compute checksum and IO block size
  int ioBlockSize = 0;
  int checkSum = 0;
  for (ii=0; ii<iobytes; ii++)
  {
    if (iobuf[ii] == 'C')
    {
      if (ii<iobytes && iobuf[ii+1] == 'S')
      {
        break;
      }
    }
    ioBlockSize++;
    checkSum += (int)iobuf[ii];
  }

  bool rv = validateCS ? (csIoBlockSize == ioBlockSize && csValue == checkSum) : true;

  if (rv == false && verbose == true)
  {
    std::cerr << "!!!!!!!!!!!!!" << endl;
    std::cerr << "The following record failed CS test:" << endl;
    std::cerr << "CS record size: " << csIoBlockSize << " cs: " << csValue << endl;
    std::cerr << "CS actual size: " << ioBlockSize << " cs: " << checkSum << endl;
    std::cerr << "=============" << endl;
    for (ii=0; ii<iobytes; ii++)
    {
      std::cerr << iobuf[ii];
    }
    std::cerr << endl;
    std::cerr << "=============" << endl;
  }

  return rv;
}
