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
 * Utility functions
 */

#ifndef RSDF_UTILS_H
#define RSDF_UTILS_H

#include <string>
#include <vector>
#include "Constants/ConstantsCoordConvert.h"
#include "Constants/ConstantsDatum.h"

/**
 * Correct RTD/CRDF-file 2-digit year to 4 digit year
 *   RSDF doesn't seem to contain 2-digit year.
 */
int CorrectYear(int year);

/**
 * Return time (secs) since the beginning of the year (local time using daylight savings)
 * @return the current time in secs
 */
double getYearTime();

/**
 * Get the current system time, using timezone value of 0
 * @returns the UTC time
 */
double getSystemTime();

/**
 * Return Julian Day
 * @returns the julian day
 */
int juldayloc();

/**
 * Return four-digit year
 * @returns the four digit year
 */
int yearloc();

/**
 * Return OS specific path
 * @param file base filename
 * @param relPath file path, relative to SIMDIS install location
 * @param simdisDir SIMDIS install location
 * @returns OS specific path to requested file
 */
std::string getFilePath(std::string file,
			std::string relPath,
			const std::string& simdisDir);

UTILS::ECoordSystemType getCoordSystemFromStr(std::string cs);

std::string getCoordSystemStr(UTILS::ECoordSystemType cs);

UTILS::MagVarType_t getMagVarFromStr(std::string mv);

std::string getMagVarStr(UTILS::MagVarType_t mv);

UTILS::VerticalDatumType_t getVertDatFromStr(std::string vd);

std::string getVertDatStr(UTILS::VerticalDatumType_t vd);

bool validChecksum(const char *iobuf, size_t iobytes, std::vector<std::string> &svec, bool validateCS, bool verbose);

#endif /* RSDF_UTILS_H */
