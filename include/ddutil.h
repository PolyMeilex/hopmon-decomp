//-----------------------------------------------------------------------------
// File: ddutil.h
//
// Desc: Routines for loading bitmaps and palettes from resources (DirectDraw
//       sample code), the DirectDraw 7 versions Hopmon uses.
//
// Declarations from Hopmon.pdb; this header isn't in dx7-framework/.
//-----------------------------------------------------------------------------
#ifndef DDUTIL_H
#define DDUTIL_H
#include <ddraw.h>

#ifdef __cplusplus
extern "C" {
#endif

IDirectDrawPalette*  DDLoadPalette( IDirectDraw7* pdd, LPCSTR szBitmap );
IDirectDrawSurface7* DDLoadBitmap( IDirectDraw7* pdd, LPCSTR szBitmap,
                                   int& bmpWidth, int& bmpHeight );
HRESULT              DDReLoadBitmap( IDirectDrawSurface7* pdds, LPCSTR szBitmap );
HRESULT              DDCopyBitmap( IDirectDrawSurface7* pdds, HBITMAP hbm,
                                   int x, int y, int dx, int dy );
DWORD                DDColorMatch( IDirectDrawSurface7* pdds, COLORREF rgb );
HRESULT              DDSetColorKey( IDirectDrawSurface7* pdds, COLORREF rgb );

#ifdef __cplusplus
}
#endif

#endif // DDUTIL_H
