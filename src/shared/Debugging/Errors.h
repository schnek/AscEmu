/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ERRORS_H
#define _ERRORS_H

#include <cstdio>
#include <cstring>

static void arcAssertFailed(const char* fname, int line, const char* expr)
{
    printf("Assertion Failed: (%s)\n", expr);
    printf("Location: %s(%i)\n", fname, line);
}

///\todo handle errors better

#define ANALYSIS_ASSUME(EXPR)

// An assert isn't necessarily fatal, but we want to stop anyways
#define WPAssert(EXPR) if (!(EXPR)) { arcAssertFailed(__FILE__,__LINE__,#EXPR); ((void(*)())0)(); } ANALYSIS_ASSUME(EXPR)

#define WPError(assertion, errmsg) if (!(assertion)) { Log::getSingleton( ).outError("%s:%i ERROR:\n  %s\n", __FILE__, __LINE__, (char*)errmsg); assert(false); }
#define WPWarning(assertion, errmsg) if (!(assertion)) { Log::getSingleton( ).outError("%s:%i WARNING:\n  %s\n", __FILE__, __LINE__, (char*)errmsg); }

// This should always halt everything.  If you ever find yourself wanting to remove the assert( false ), switch to WPWarning or WPError
#define WPFatal(assertion, errmsg) if(!(assertion)) { Log::getSingleton().outError("%s:%i FATAL ERROR:\n  %s\n", __FILE__, __LINE__, (char*)errmsg); assert(#assertion &&0); abort(); }

#define ASSERT WPAssert

#endif      //_ERRORS_H
