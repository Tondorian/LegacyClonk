/*
 * LegacyClonk
 *
 * Copyright (c) RedWolf Design
 * Copyright (c) 2003, Sven2
 * Copyright (c) 2017-2019, The LegacyClonk Team and contributors
 *
 * Distributed under the terms of the ISC license; see accompanying file
 * "COPYING" for details.
 *
 * "Clonk" is a registered trademark of Matthes Bender, used with permission.
 * See accompanying file "TRADEMARK" for details.
 *
 * To redistribute this file separately, substitute the full license texts
 * for the above references.
 */

// startup screen

#ifndef INC_C4LoaderScreen
#define INC_C4LoaderScreen

#include <C4FacetEx.h>

class C4LoaderScreen
	{
	public:
		CStdFont &TitleFont; // font used for title output
		CStdFont &LogFont;   // font used for logging
		C4FacetExSurface fctBackground; // background image
		char *szInfo;      // info text to be drawn on loader screen
		bool fBlackScreen; // if set, a black screen is drawn instead of a loader

	public:
		C4LoaderScreen(); // ctor
		~C4LoaderScreen(); // dtor

		bool Init(const char *szLoaderSpec); // inits and loads from global C4Game-class
		void SetBlackScreen(bool fIsBlack);  // enabled/disables drawing of loader screen
		int SeekLoaderScreens(C4Group &rFromGrp, const char *szWildcard, int iLoaderCount, char *szDstName, C4Group **ppDestGrp);

		void Draw(C4Facet &cgo, int iProgress=0, class C4LogBuffer *pLog=NULL, int Process=0); // draw loader screen (does not page flip!)
	};

#endif //INC_C4LoaderScreen
