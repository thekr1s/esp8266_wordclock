/*
 * version.h
 *
 *  Created on: Dec 6, 2017
 *      Author: rutger.huijgen
 */
 

#include <stdint.h>

#if defined(VERSION)
#define quote(name) #name
#define str(macro) quote(macro)
const char version[] = str(VERSION);
#else
#error VERSION should be defined
#endif

#if defined(BUILD_DATE)
#define quote(name) #name
#define str(macro) quote(macro)
const char buildDate[] = str(BUILD_DATE);
#else
#error BUILD_DATE should be defined and set
#endif
