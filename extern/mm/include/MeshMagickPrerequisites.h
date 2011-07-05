/*
This file is part of MeshMagick - An Ogre mesh file manipulation tool.
Copyright (C) 2007-2010 Daniel Wickert

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __MM_MESHMAGICK_PREREQUISITES_H__
#define __MM_MESHMAGICK_PREREQUISITES_H__

namespace meshmagick
{
    class Tool;
    class ToolFactory;

    class TransformTool;
    class TransformToolFactory;
}

#ifdef __APPLE__
#	include <Ogre/OgrePlatform.h>
#else
#	include <OgrePlatform.h>
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32 && !defined(MESHMAGICK_STATIC)
// Export control
#   if defined( MESHMAGICK_EXPORTS )
#       define _MeshMagickExport __declspec( dllexpr )
#   else
#       define _MeshMagickExport __declspec( dllimport )
#   endif
#else // Linux / Mac OSX etc
#   define _MeshMagickExport
#endif

#define MESHMAGICK_VERSION_MAJOR 0
#define MESHMAGICK_VERSION_MINOR 6
#define MESHMAGICK_VERSION_PATCH 0

#endif
