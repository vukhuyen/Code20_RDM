// -*- mode: c++ -*-
/****************************************************************************
*****                                                                  *****
*****                   Classification: UNCLASSIFIED                   *****
*****                    Classified By:                                *****
*****                    Declassify On:                                *****
*****                                                                  *****
****************************************************************************
*
* Developed by: NUWC Keyport 
*               Code 234
*               610 Dowell Street
*               Keyport WA, 98367
* @Khue Vu
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*/

/*
PlatformCategory.cpp
*/

#include "PlatformCategory.h"
#include <iostream>

using namespace std;

PlatformCategory::PlatformCategory()
{
}
PlatformCategory::~PlatformCategory()
{
}

char *getPlatformTypeStr(PlatformCategory::PlatformType type)
{
	//return (char *[]){"Unlisted Value", "Aircraft", "Torpedo"}[type];
	char *enumString[] = {"Unlisted Value", "Aircraft", "Torpedo"};
	return enumString[type];
}

int main()
{
	PlatformCategory cat;
	cout << cat.getPlatformTypeStr(PlatformCategory::PlatformType::AIRCRAFT);
	return 0;
}