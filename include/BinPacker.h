/*
 Copyright (c) 2010, The Barbarian Group
 All rights reserved.
 
 Portions of this code (C) Paul Houx
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "cinder/Area.h"

#include <vector>
#include <map>


class BinnedArea : public ci::Area
{
public:
	BinnedArea() : ci::Area(), mBin(-1), mIsRotated(false) {}
	BinnedArea( int32_t bin, bool isRotated=false ) : ci::Area(), mBin(bin), mIsRotated(isRotated) {}

	BinnedArea( const ci::Vec2i &UL, const ci::Vec2i &LR ) : ci::Area(UL, LR), mBin(-1), mIsRotated(false) {}
	BinnedArea( const ci::Vec2i &UL, const ci::Vec2i &LR, int32_t bin, bool isRotated=false ) : ci::Area(UL, LR), mBin(bin), mIsRotated(isRotated) {}

	BinnedArea( int32_t aX1, int32_t aY1, int32_t aX2, int32_t aY2 ) : mBin(-1), mIsRotated(false)
		{ set( aX1, aY1, aX2, aY2 ); }
	BinnedArea( int32_t aX1, int32_t aY1, int32_t aX2, int32_t aY2, int32_t bin, bool isRotated=false ) : mBin(bin), mIsRotated(isRotated)
		{ set( aX1, aY1, aX2, aY2 ); }

	explicit BinnedArea( const ci::RectT<float> &r ) : ci::Area(r), mBin(-1), mIsRotated(false) {}
	explicit BinnedArea( const ci::RectT<float> &r, int32_t bin, bool isRotated=false ) : ci::Area(r), mBin(bin), mIsRotated(isRotated) {}
	
	explicit BinnedArea( const ci::Area &area ) : ci::Area(area), mBin(-1), mIsRotated(false) {}
	explicit BinnedArea( const ci::Area &area, int32_t bin, bool isRotated=false ) : ci::Area(area), mBin(bin), mIsRotated(isRotated) {}

	//
	int32_t getBin() const { return mBin; }
	//
	bool isRotated() const { return mIsRotated; }

private:
	int32_t		mBin;
	bool		mIsRotated;
};

class BinPackerBase
{
public:
	BinPackerBase() : mBinWidth(512), mBinHeight(512) { clear(); }
	BinPackerBase( int width, int height ) :
		mBinWidth(width), mBinHeight(height) { clear(); }

	virtual ~BinPackerBase() {}	

	//!
	virtual BinPackerBase&	setSize( unsigned width, unsigned height ) = 0;
	virtual BinPackerBase&	setSize( const ci::Vec2i &size ) = 0;

	//
	virtual std::vector<BinnedArea>	pack( const std::vector<ci::Area> &rects, bool allowRotation = false ) = 0;
	
	//!
	ci::Vec2i	getSize() const { return ci::Vec2i( mBinWidth, mBinHeight ); }
	//!
	int			getWidth() const { return mBinWidth; }
	//!
	int			getHeight() const { return mBinHeight; }

protected:
    struct Rect
    {
        int  x;
        int  y;
        int  w;
        int  h;
        int  order;
        int  children[2];
        bool rotated;
        bool packed;

        Rect(int w, int h, int order = -1)
            : x(0), y(0), w(w), h(h), order(order), rotated(false), packed(false)
        {
            children[0] = -1;
            children[1] = -1;
        }

        Rect(int x, int y, int w, int h, int order = -1)
            : x(x), y(y), w(w), h(h), order(order), rotated(false), packed(false)
        {
            children[0] = -1;
            children[1] = -1;
        }
        
        int getArea() const {
            return w * h;
        }
        
        void rotate() {
            std::swap(w, h);
            rotated = !rotated;
        }
        
        bool operator<(const Rect& rhs) const {
            return getArea() < rhs.getArea();
        }
    };

protected:
	int					mBinWidth;
	int					mBinHeight;
    
    int					mNumPacked;
    std::vector<Rect>	mRects;
    std::vector<Rect>	mPacks;

    virtual void clear();

    void fill(int pack, bool allowRotation);
    void split(int pack, int rect);
    bool fits(Rect& rect1, const Rect& rect2, bool allowRotation) const;
    
    bool rectIsValid(int i) const;
    bool packIsValid(int i) const;
};

class BinPacker /*final*/ : public BinPackerBase 
{
public:
	BinPacker() : BinPackerBase(512, 512) {}
	BinPacker( int width, int height ) : BinPackerBase(width, height) {}
	BinPacker( const ci::Vec2i &size ) : BinPackerBase(size.x, size.y) {}

	~BinPacker() {}

	//!
	BinPacker&	setSize( unsigned width, unsigned height ) { mBinWidth = width; mBinHeight = height; return *this; }
	BinPacker&	setSize( const ci::Vec2i &size ) { mBinWidth = size.x; mBinHeight = size.y; return *this; }

	//!
    std::vector<BinnedArea>	pack( const std::vector<ci::Area> &rects, bool allowRotation = false );
};

class MultiBinPacker /*final*/ : public BinPackerBase
{
public:
	MultiBinPacker() : BinPackerBase(512, 512) {}
	MultiBinPacker( int width, int height ) : BinPackerBase(width, height) {}
	MultiBinPacker( const ci::Vec2i &size ) : BinPackerBase(size.x, size.y) {}

	~MultiBinPacker() {}

	//!
	MultiBinPacker&	setSize( unsigned width, unsigned height ) { mBinWidth = width; mBinHeight = height; return *this; }
	MultiBinPacker&	setSize( const ci::Vec2i &size ) { mBinWidth = size.x; mBinHeight = size.y; return *this; }

	//!
    std::vector<BinnedArea>	pack( const std::vector<ci::Area> &rects, bool allowRotation = false );
private:
    void clear();

	std::vector<unsigned>	mBins;
};

class BinPackerTooSmallExc : public std::exception {
 public:
	virtual const char* what() const throw() { return "Bin size is too small to fit all rectangles."; } 
};

