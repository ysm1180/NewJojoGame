#include "LayoutControl.h"

#include "WindowControl.h"
#include "ImageControl.h"
#include "BaseLib/Color.h"

namespace jojogame {
void CLayoutControl::RegisterFunctions(lua_State* L)
{
    LUA_BEGIN(CLayoutControl, "_Layout");

    LUA_METHOD(SetX);
    LUA_METHOD(SetY);
    LUA_METHOD(SetWidth);
    LUA_METHOD(SetHeight);
    LUA_METHOD(SetRatioX);
    LUA_METHOD(SetRatioY);

    LUA_METHOD(AddImage);
    LUA_METHOD(DeleteImage);
}

CLayoutControl::CLayoutControl()
{
    _dc = CreateCompatibleDC(nullptr);
    _size.cx = 0;
    _size.cy = 0;
    _position.x = 0;
    _position.y = 0;
}

CLayoutControl::~CLayoutControl()
{
    for (auto imageInfo : _images)
    {
        HBITMAP deletedBitmap = SelectBitmap(imageInfo.imageDC, imageInfo.oldBitmap);
        DeleteBitmap(deletedBitmap);
        DeleteDC(imageInfo.imageDC);
    }
}

int CLayoutControl::GetX() const
{
    return _position.x;
}

int CLayoutControl::GetY() const
{
    return _position.y;
}

int CLayoutControl::GetWidth() const
{
    return _size.cx;
}

int CLayoutControl::GetHeight() const
{
    return _size.cy;
}

void CLayoutControl::SetX(int x, bool isRedraw)
{
    if (_position.x != x)
    {
        if (isRedraw)
        {
            if (!_parents.empty())
            {
                for (auto& parent : _parents)
                {
                    int width = _size.cx;
                    int height = _size.cy;
                    if (width == 0)
                    {
                        width = parent->GetWidth();
                    }
                    if (height == 0)
                    {
                        height = parent->GetHeight();
                    }

                    RECT rect;
                    SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX,
                            _position.y + height * _ratioY);
                    InvalidateRect(parent->GetHWnd(), &rect, TRUE);
                }
            }

            _position.x = x;

            if (!_parents.empty())
            {
                for (auto& parent : _parents)
                {
                    int width = _size.cx;
                    int height = _size.cy;
                    if (width == 0)
                    {
                        width = parent->GetWidth();
                    }
                    if (height == 0)
                    {
                        height = parent->GetHeight();
                    }

                    RECT rect;
                    SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX,
                            _position.y + height * _ratioY);
                    InvalidateRect(parent->GetHWnd(), &rect, TRUE);

                    UpdateWindow(parent->GetHWnd());
                }
            }
        }
        else
        {
            _position.x = x;
        }
    }
}

void CLayoutControl::SetY(int y, bool isRedraw)
{
    if (_position.y != y)
    {
        if (isRedraw)
        {
            if (!_parents.empty())
            {
                for (auto& parent : _parents)
                {
                    int width = _size.cx;
                    int height = _size.cy;
                    if (width == 0)
                    {
                        width = parent->GetWidth();
                    }
                    if (height == 0)
                    {
                        height = parent->GetHeight();
                    }

                    RECT rect;
                    SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX,
                            _position.y + height * _ratioY);
                    InvalidateRect(parent->GetHWnd(), &rect, TRUE);
                }
            }

            _position.y = y;

            if (!_parents.empty())
            {
                for (auto& parent : _parents)
                {
                    int width = _size.cx;
                    int height = _size.cy;
                    if (width == 0)
                    {
                        width = parent->GetWidth();
                    }
                    if (height == 0)
                    {
                        height = parent->GetHeight();
                    }

                    RECT rect;
                    SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX,
                            _position.y + height * _ratioY);
                    InvalidateRect(parent->GetHWnd(), &rect, TRUE);

                    UpdateWindow(parent->GetHWnd());
                }
            }
        }
        else
        {
            _position.y = y;
        }
    }
}

void CLayoutControl::SetWidth(int cx, bool isRedraw)
{
    if (_size.cx != cx)
    {
        if (isRedraw)
        {
            if (!_parents.empty())
            {
                for (auto& parent : _parents)
                {
                    int width = _size.cx;
                    int height = _size.cy;
                    if (width == 0)
                    {
                        width = parent->GetWidth();
                    }
                    if (height == 0)
                    {
                        height = parent->GetHeight();
                    }

                    RECT rect;
                    SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX,
                            _position.y + height * _ratioY);
                    InvalidateRect(parent->GetHWnd(), &rect, TRUE);
                }
            }

            _size.cx = cx;

            if (!_parents.empty())
            {
                for (auto& parent : _parents)
                {
                    int width = _size.cx;
                    int height = _size.cy;
                    if (width == 0)
                    {
                        width = parent->GetWidth();
                    }
                    if (height == 0)
                    {
                        height = parent->GetHeight();
                    }

                    RECT rect;
                    SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX,
                            _position.y + height * _ratioY);
                    InvalidateRect(parent->GetHWnd(), &rect, TRUE);

                    UpdateWindow(parent->GetHWnd());
                }
            }
        }
        else
        {
            _size.cx = cx;
        }
    }
}

void CLayoutControl::SetHeight(int cy, bool isRedraw)
{
    if (isRedraw)
    {
        if (!_parents.empty())
        {
            for (auto& parent : _parents)
            {
                int width = _size.cx;
                int height = _size.cy;
                if (width == 0)
                {
                    width = parent->GetWidth();
                }
                if (height == 0)
                {
                    height = parent->GetHeight();
                }

                RECT rect;
                SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX, _position.y + height * _ratioY);
                InvalidateRect(parent->GetHWnd(), &rect, TRUE);
            }
        }

        _size.cy = cy;

        if (!_parents.empty())
        {
            for (auto& parent : _parents)
            {
                int width = _size.cx;
                int height = _size.cy;
                if (width == 0)
                {
                    width = parent->GetWidth();
                }
                if (height == 0)
                {
                    height = parent->GetHeight();
                }

                RECT rect;
                SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX, _position.y + height * _ratioY);
                InvalidateRect(parent->GetHWnd(), &rect, TRUE);

                UpdateWindow(parent->GetHWnd());
            }
        }
    }
    else
    {
        _size.cy = cy;
    }
}

void CLayoutControl::SetRatioX(double ratio, bool isRedraw)
{
    if (isRedraw)
    {
        if (!_parents.empty())
        {
            for (auto& parent : _parents)
            {
                int width = _size.cx;
                int height = _size.cy;
                if (width == 0)
                {
                    width = parent->GetWidth();
                }
                if (height == 0)
                {
                    height = parent->GetHeight();
                }

                RECT rect;
                SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX, _position.y + height * _ratioY);
                InvalidateRect(parent->GetHWnd(), &rect, TRUE);
            }
        }

        _ratioX = ratio;

        if (!_parents.empty())
        {
            for (auto& parent : _parents)
            {
                int width = _size.cx;
                int height = _size.cy;
                if (width == 0)
                {
                    width = parent->GetWidth();
                }
                if (height == 0)
                {
                    height = parent->GetHeight();
                }

                RECT rect;
                SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX, _position.y + height * _ratioY);
                InvalidateRect(parent->GetHWnd(), &rect, TRUE);

                UpdateWindow(parent->GetHWnd());
            }
        }
    }
    else
    {
        _ratioX = ratio;
    }
}

void CLayoutControl::SetRatioY(double ratio, bool isRedraw)
{
    if (isRedraw)
    {
        if (!_parents.empty())
        {
            for (auto& parent : _parents)
            {
                int width = _size.cx;
                int height = _size.cy;
                if (width == 0)
                {
                    width = parent->GetWidth();
                }
                if (height == 0)
                {
                    height = parent->GetHeight();
                }

                RECT rect;
                SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX, _position.y + height * _ratioY);
                InvalidateRect(parent->GetHWnd(), &rect, TRUE);
            }
        }

        _ratioY = ratio;

        if (!_parents.empty())
        {
            for (auto& parent : _parents)
            {
                int width = _size.cx;
                int height = _size.cy;
                if (width == 0)
                {
                    width = parent->GetWidth();
                }
                if (height == 0)
                {
                    height = parent->GetHeight();
                }

                RECT rect;
                SetRect(&rect, _position.x, _position.y, _position.x + width * _ratioX, _position.y + height * _ratioY);
                InvalidateRect(parent->GetHWnd(), &rect, TRUE);

                UpdateWindow(parent->GetHWnd());
            }
        }
    }
    else
    {
        _ratioY = ratio;
    }
}

void CLayoutControl::AddParentWindow(CWindowControl* parent)
{
    _parents.push_back(parent);
}

void CLayoutControl::RemoveParentWIndow(CWindowControl * parent)
{
    _parents.erase(std::remove(_parents.begin(), _parents.end(), parent), _parents.end());
}


int CLayoutControl::AddImage(CImageControl* image, int x, int y, bool isUpdate)
{
    HDC imageDC = CreateCompatibleDC(_dc);
    HDC newDC = CreateCompatibleDC(nullptr);
    HDC dc = GetWindowDC(GetDesktopWindow());
    HBITMAP newBitmap = CreateCompatibleBitmap(dc, image->GetWidth(), image->GetHeight());
    ImageInformation imageInfo;
    int index = _GetNewIndex();

    imageInfo.oldBitmap = SelectBitmap(newDC, newBitmap);

    HBITMAP oldBitmap = SelectBitmap(imageDC, image->GetImageHandle());
    BitBlt(newDC, 0, 0, image->GetWidth(), image->GetHeight(), imageDC, 0, 0, SRCCOPY);
    SelectBitmap(imageDC, oldBitmap);
    DeleteDC(imageDC);

    imageInfo.imageDC = newDC;
    imageInfo.index = index;
    imageInfo.image = image;
    imageInfo.position.x = x;
    imageInfo.position.y = y;

    _images.push_back(imageInfo);

    if (!_parents.empty())
    {
        for (auto& parent : _parents)
        {
            int imageX = (x * _ratioX) + _position.x;
            int imageY = (y * _ratioY) + _position.y;

            RECT rect;
            SetRect(&rect, imageX, imageY, imageX + image->GetWidth() * _ratioX, imageY + image->GetHeight() * _ratioY);
            InvalidateRect(parent->GetHWnd(), &rect, TRUE);

            if (isUpdate)
            {
                UpdateWindow(parent->GetHWnd());
            }
        }
    }

    return index;
}

void CLayoutControl::DeleteImage(CImageControl* image, bool isUpdate)
{
    auto iter = std::begin(_images);

    while (iter != std::end(_images))
    {
        if (iter->image == image)
        {
            auto position = iter->position;

            HBITMAP deletedBitmap = SelectBitmap(iter->imageDC, iter->oldBitmap);
            DeleteBitmap(deletedBitmap);
            DeleteDC(iter->imageDC);
            _reusingImageIndex.push(iter->index);
            iter = _images.erase(iter);

            if (!_parents.empty())
            {
                for (auto& parent : _parents)
                {
                    int imageX = (position.x * _ratioX) + _position.x;
                    int imageY = (position.y * _ratioY) + _position.y;
                    RECT rect;

                    SetRect(&rect, imageX, imageY, imageX + image->GetWidth() * _ratioX,
                            imageY + image->GetHeight() * _ratioY);
                    InvalidateRect(parent->GetHWnd(), &rect, TRUE);

                    if (isUpdate)
                    {
                        UpdateWindow(parent->GetHWnd());
                    }
                }
            }

            break;
        }
        else
        {
            ++iter;
        }
    }
}

void CLayoutControl::Draw(HDC destDC)
{
    for (ImageInformation image : _images)
    {
        int imageX = (image.position.x * _ratioX) + _position.x;
        int imageY = (image.position.y * +_ratioY) + _position.y;
        int imageWidth = image.image->GetWidth() * _ratioX;
        int imageHeight = image.image->GetHeight() * _ratioY;

        if (_ratioX == 1.0 && _ratioY == 1.0)
        {
            if (imageX + imageWidth > _size.cx)
            {
                imageWidth = _size.cx - imageX;
            }
            if (imageY + imageHeight > _size.cy)
            {
                imageHeight = _size.cy - imageY;
            }

            BitBlt(destDC, imageX, imageY, imageWidth, imageHeight, image.imageDC, 0, 0, SRCCOPY);
        }
        else
        {
            auto memDc = CreateCompatibleDC(destDC);
            auto memBitmap = CreateCompatibleBitmap(destDC, imageWidth, imageHeight);
            auto oldBitmap = SelectBitmap(memDc, memBitmap);

            SetStretchBltMode(memDc, COLORONCOLOR);
            StretchBlt(memDc, 0, 0, imageWidth, imageHeight, image.imageDC, 0, 0, image.image->GetWidth(),
                       image.image->GetHeight(), SRCCOPY);

            if (imageX + imageWidth > _size.cx)
            {
                imageWidth = _size.cx - imageX;
            }
            if (imageY + imageHeight > _size.cy)
            {
                imageHeight = _size.cy - imageY;
            }

            BitBlt(destDC, imageX, imageY, imageWidth, imageHeight, memDc, 0, 0, SRCCOPY);

            SelectBitmap(memDc, oldBitmap);
            DeleteBitmap(memBitmap);
            DeleteDC(memDc);
        }
    }
}

void CLayoutControl::Draw(HDC destDC, RECT& clipingRect)
{
    for (ImageInformation image : _images)
    {
        RECT realClipingRect;
        RECT layoutRect;
        SetRect(&layoutRect, _position.x, _position.y, _position.x + _size.cx, _position.y + _size.cy);
        if (!IntersectRect(&realClipingRect, &layoutRect, &clipingRect))
        {
            continue;
        }

        int imageX = (image.position.x * _ratioX) + _position.x;
        int imageY = (image.position.y * +_ratioY) + _position.y;
        int imageWidth = image.image->GetWidth() * _ratioX;
        int imageHeight = image.image->GetHeight() * _ratioY;

        if (_ratioX == 1.0 && _ratioY == 1.0)
        {
            if (imageX + imageWidth > _size.cx)
            {
                imageWidth = _size.cx - imageX;
            }
            if (imageY + imageHeight > _size.cy)
            {
                imageHeight = _size.cy - imageY;
            }

            RECT imageRect;
            RECT realDrawRect;
            SetRect(&imageRect, imageX, imageY, imageX + imageWidth, imageY + imageHeight);
            if (!IntersectRect(&realDrawRect, &imageRect, &realClipingRect))
            {
                continue;
            }

            BitBlt(destDC, realDrawRect.left, realDrawRect.top, realDrawRect.right - realDrawRect.left,
                   realDrawRect.bottom - realDrawRect.top, image.imageDC, realDrawRect.left - imageRect.left,
                   realDrawRect.top - imageRect.top, SRCCOPY);
        }
        else
        {
            auto memDc = CreateCompatibleDC(destDC);
            auto memBitmap = CreateCompatibleBitmap(destDC, imageWidth, imageHeight);
            auto oldBitmap = SelectBitmap(memDc, memBitmap);

            SetStretchBltMode(memDc, COLORONCOLOR);
            StretchBlt(memDc, 0, 0, imageWidth, imageHeight, image.imageDC, 0, 0, image.image->GetWidth(),
                       image.image->GetHeight(), SRCCOPY);

            if (imageX + imageWidth > _size.cx)
            {
                imageWidth = _size.cx - imageX;
            }
            if (imageY + imageHeight > _size.cy)
            {
                imageHeight = _size.cy - imageY;
            }

            RECT imageRect;
            RECT realDrawRect;
            SetRect(&imageRect, imageX, imageY, imageX + imageWidth, imageY + imageHeight);
            if (!IntersectRect(&realDrawRect, &imageRect, &realClipingRect))
            {
                continue;
            }

            BitBlt(destDC, realDrawRect.left, realDrawRect.top, realDrawRect.right - realDrawRect.left,
                   realDrawRect.bottom - realDrawRect.top, memDc, realDrawRect.left - imageRect.left,
                   realDrawRect.top - imageRect.top, SRCCOPY);

            SelectBitmap(memDc, oldBitmap);
            DeleteBitmap(memBitmap);
            DeleteDC(memDc);
        }
    }
}

void CLayoutControl::Draw(HDC destDC, RECT &clipingRect, COLORREF mixedColor)
{
    for (ImageInformation image : _images)
    {
        RECT realClipingRect;
        RECT layoutRect;
        SetRect(&layoutRect, _position.x, _position.y, _position.x + _size.cx, _position.y + _size.cy);
        if (!IntersectRect(&realClipingRect, &layoutRect, &clipingRect))
        {
            continue;
        }

        int imageX = (image.position.x * _ratioX) + _position.x;
        int imageY = (image.position.y * +_ratioY) + _position.y;
        auto width = image.image->GetWidth();
        int height = image.image->GetHeight();
        int imageWidth = width * _ratioX;
        int imageHeight = height * _ratioY;

        if (_ratioX == 1.0 && _ratioY == 1.0)
        {
            if (imageX + imageWidth > _size.cx)
            {
                imageWidth = _size.cx - imageX;
            }
            if (imageY + imageHeight > _size.cy)
            {
                imageHeight = _size.cy - imageY;
            }

            RECT imageRect;
            RECT realDrawRect;
            SetRect(&imageRect, imageX, imageY, imageX + imageWidth, imageY + imageHeight);
            if (!IntersectRect(&realDrawRect, &imageRect, &realClipingRect))
            {
                continue;
            }

            auto bitmapInfo = image.image->GetBitmapInfo();
            GetDIBits(image.imageDC, image.image->GetImageHandle(), 0, 0, nullptr, &bitmapInfo, DIB_RGB_COLORS);

            int bits = bitmapInfo.bmiHeader.biBitCount / 8;
            auto pixels = new BYTE[bitmapInfo.bmiHeader.biSizeImage];
            GetDIBits(image.imageDC, image.image->GetImageHandle(), 0, height, pixels, &bitmapInfo, DIB_RGB_COLORS);

            auto cx = realDrawRect.right - realDrawRect.left;
            auto cy = realDrawRect.bottom - realDrawRect.top;
            auto drawingImageX = realDrawRect.left - imageRect.left;
            auto drawingImageY = realDrawRect.top - imageRect.top;

            for (int y = height - drawingImageY - 1; y >  height - drawingImageY - cy - 1; --y)
            {
                for (int x = drawingImageX; x <  drawingImageX + cx; ++x)
                {
                    double h, s, v, h2, s2, v2;
                    BYTE b = pixels[(x + y * width) * bits];
                    BYTE g = pixels[(x + y * width) * bits + 1];
                    BYTE r = pixels[(x + y * width) * bits + 2];
                    RgbToHsv(r, g, b, h, s, v);
                    RgbToHsv(GetRValue(mixedColor), GetGValue(mixedColor), GetBValue(mixedColor), h2, s2, v2);
                    h = h2;
                    s = s2;
                    HsvToRgb(h, s, v, r, g, b);
                    pixels[(x + y * width) * bits] = b;
                    pixels[(x + y * width) * bits + 1] = g;
                    pixels[(x + y * width) * bits + 2] = r;
                }
            }
            BITMAPINFOHEADER *bitmapInfoHeader = (BITMAPINFOHEADER *)&bitmapInfo;
            HBITMAP newBitmap = CreateDIBitmap(image.imageDC, bitmapInfoHeader, CBM_INIT, pixels, &bitmapInfo, DIB_RGB_COLORS);
            auto oldBitmap = SelectBitmap(image.imageDC, newBitmap);

            
            BitBlt(destDC, realDrawRect.left, realDrawRect.top, cx,
                   cy, image.imageDC, drawingImageX,
                   drawingImageY, SRCCOPY);

            SelectObject(image.imageDC, oldBitmap);
            DeleteBitmap(newBitmap);
            delete[] pixels;
        }
        else
        {
            auto memDc = CreateCompatibleDC(destDC);
            auto memBitmap = CreateCompatibleBitmap(destDC, imageWidth, imageHeight);
            auto oldBitmap = SelectBitmap(memDc, memBitmap);

            SetStretchBltMode(memDc, COLORONCOLOR);
            StretchBlt(memDc, 0, 0, imageWidth, imageHeight, image.imageDC, 0, 0, width,
                       height, SRCCOPY);

            if (imageX + imageWidth > _size.cx)
            {
                imageWidth = _size.cx - imageX;
            }
            if (imageY + imageHeight > _size.cy)
            {
                imageHeight = _size.cy - imageY;
            }

            RECT imageRect;
            RECT realDrawRect;
            SetRect(&imageRect, imageX, imageY, imageX + imageWidth, imageY + imageHeight);
            if (!IntersectRect(&realDrawRect, &imageRect, &realClipingRect))
            {
                continue;
            }

            BitBlt(destDC, imageX, imageY, imageWidth, imageHeight, memDc, 0, 0, SRCCOPY);

            SelectBitmap(memDc, oldBitmap);
            DeleteBitmap(memBitmap);
            DeleteDC(memDc);
        }
    }
}

void CLayoutControl::Erase()
{
}

int CLayoutControl::_GetNewIndex()
{
    int index;

    if (_reusingImageIndex.empty())
    {
        index = _images.size();
    }
    else
    {
        index = _reusingImageIndex.front();
        _reusingImageIndex.pop();
    }

    return index;
}
}
