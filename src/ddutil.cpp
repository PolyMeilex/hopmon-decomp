#include "common.h"
#include "ddutil.h"




// 0x0040CC80, 938 bytes.
IDirectDrawPalette* DDLoadPalette(IDirectDraw7* pdd, LPCSTR szBitmap)
{
    PALETTEENTRY ape[256];
    IDirectDrawPalette* ddpal;
    HRSRC h;
    int i;
    int n;
    LPBITMAPINFOHEADER lpbi;
    RGBQUAD* prgb;
    int fh;

    for (i = 0; i < 256; i++)
    {
        ape[i].peRed   = (BYTE)(((i >> 5) & 0x7) * 0xFF / 7);
        ape[i].peGreen = (BYTE)(((i >> 2) & 0x7) * 0xFF / 7);
        ape[i].peBlue  = (BYTE)(((i >> 0) & 0x3) * 0xFF / 3);
        ape[i].peFlags = 0;
    }

    if (szBitmap != NULL && (h = FindResource(NULL, szBitmap, RT_BITMAP)) != NULL)
    {
        lpbi = (LPBITMAPINFOHEADER)LockResource(LoadResource(NULL, h));
        if (lpbi == NULL)
            OutputDebugString("lock resource failed\n");

        prgb = (RGBQUAD*)((BYTE*)lpbi + lpbi->biSize);

        if (lpbi == NULL || lpbi->biSize < sizeof(BITMAPINFOHEADER))
            n = 0;
        else if (lpbi->biBitCount > 8)
            n = 0;
        else
        {
            if (lpbi->biClrUsed == 0)
                n = 1 << lpbi->biBitCount;
            else
                n = lpbi->biClrUsed;
        }

        for (i = 0; i < n; i++)
        {
            ape[i].peRed   = prgb[i].rgbRed;
            ape[i].peGreen = prgb[i].rgbGreen;
            ape[i].peBlue  = prgb[i].rgbBlue;
            ape[i].peFlags = 0;
        }
    }
    else
    {
        if (szBitmap != NULL && (fh = _lopen(szBitmap, OF_READ)) != -1)
        {
            BITMAPFILEHEADER bf;
            BITMAPINFOHEADER bi;
            _lread(fh, &bf, sizeof(bf));
            _lread(fh, &bi, sizeof(bi));
            _lread(fh, ape, sizeof(ape));
            _lclose(fh);

            if (bi.biSize != sizeof(BITMAPINFOHEADER))
                n = 0;
            else if (bi.biBitCount > 8)
                n = 0;
            else
            {
                if (bi.biClrUsed == 0)
                    n = 1 << bi.biBitCount;
                else
                    n = bi.biClrUsed;
            }

            for (i = 0; i < n; i++)
            {
                unsigned char r = ape[i].peRed;
                ape[i].peRed  = ape[i].peBlue;
                ape[i].peBlue = r;
            }
        }
    }

    pdd->CreatePalette(DDPCAPS_8BIT, ape, &ddpal, NULL);

    return ddpal;
}




// 0x0040D030, 324 bytes.
IDirectDrawSurface7* DDLoadBitmap(IDirectDraw7* pdd, LPCSTR szBitmap, int& bmpWidth, int& bmpHeight)
{
    HBITMAP hbm;
    BITMAP bm;
    DDSURFACEDESC2 ddsd;
    IDirectDrawSurface7* pdds;

    bmpWidth = -1;
    bmpHeight = -1;

    hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), szBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if (hbm == NULL)
        hbm = (HBITMAP)LoadImage(NULL, szBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

    if (hbm == NULL)
        return NULL;

    GetObject(hbm, sizeof(bm), &bm);

    ZeroMemory(&ddsd, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    ddsd.dwWidth = bm.bmWidth;
    ddsd.dwHeight = bm.bmHeight;

    if (pdd->CreateSurface(&ddsd, &pdds, NULL) != DD_OK)
        return NULL;

    DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);
    DeleteObject(hbm);
    bmpWidth = bm.bmWidth;
    bmpHeight = bm.bmHeight;

    return pdds;
}




// 0x0040D180, 156 bytes.
HRESULT DDReLoadBitmap(IDirectDrawSurface7* pdds, LPCSTR szBitmap)
{
    HBITMAP hbm;
    HRESULT hr;

    hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), szBitmap, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    if (hbm == NULL)
        hbm = (HBITMAP)LoadImage(NULL, szBitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

    if (hbm == NULL)
    {
        OutputDebugString("handle is null\n");
        return E_FAIL;
    }

    hr = DDCopyBitmap(pdds, hbm, 0, 0, 0, 0);
    if (hr != DD_OK)
        OutputDebugString("ddcopybitmap failed\n");
    DeleteObject(hbm);

    return hr;
}




// 0x0040D220, 376 bytes.
HRESULT DDCopyBitmap(IDirectDrawSurface7* pdds, HBITMAP hbm, int x, int y, int dx, int dy)
{
    HRESULT hr;
    HDC hdcImage;
    HDC hdc;
    BITMAP bm;
    DDSURFACEDESC2 ddsd;

    if (hbm == NULL || pdds == NULL)
        return E_FAIL;

    pdds->Restore();

    hdcImage = CreateCompatibleDC(NULL);
    if (hdcImage == NULL)
        OutputDebugString("createcompatible dc failed\n");

    SelectObject(hdcImage, hbm);
    GetObject(hbm, sizeof(bm), &bm);

    dx = (dx == 0) ? bm.bmWidth : dx;
    dy = (dy == 0) ? bm.bmHeight : dy;

    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
    pdds->GetSurfaceDesc(&ddsd);

    hr = pdds->GetDC(&hdc);
    if (hr == DD_OK)
    {
        StretchBlt(hdc, 0, 0, ddsd.dwWidth, ddsd.dwHeight, hdcImage, x, y, dx, dy, SRCCOPY);
        pdds->ReleaseDC(hdc);
    }

    DeleteDC(hdcImage);

    return hr;
}




// 0x0040D3A0, 342 bytes.
DWORD DDColorMatch(IDirectDrawSurface7* pdds, COLORREF rgb)
{
    COLORREF rgbT;
    DWORD dw;
    HDC hdc;
    DDSURFACEDESC2 ddsd;
    HRESULT hres;

    dw = CLR_INVALID;

    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);
        SetPixel(hdc, 0, 0, rgb);
        pdds->ReleaseDC(hdc);
    }

    ddsd.dwSize = sizeof(ddsd);
    while ((hres = pdds->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
        ;

    if (hres == DD_OK)
    {
        dw = *(DWORD*)ddsd.lpSurface;
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;
        pdds->Unlock(NULL);
    }

    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == DD_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }

    return dw;
}




// 0x0040D500, 55 bytes.
HRESULT DDSetColorKey(IDirectDrawSurface7* pdds, COLORREF rgb)
{
    DDCOLORKEY ddck;

    ddck.dwColorSpaceLowValue = DDColorMatch(pdds, rgb);
    ddck.dwColorSpaceHighValue = ddck.dwColorSpaceLowValue;

    return pdds->SetColorKey(DDCKEY_SRCBLT, &ddck);
}
