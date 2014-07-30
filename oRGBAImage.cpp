/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oRGBAImage.cpp
 *
 *  RGBA bitmap class
 *
 *  Bastiaan Olij
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oRGBAImage.h"

// initialise our buffer..
void oRGBAImage::initBuffer(qlong pWidth, qlong pHeight) {
	// free any buffer we still have...
	if (mBuffer!=NULL) {
		MEMfree(mBuffer);
		mWidth		= 0;
		mHeight		= 0;
		mBuffer		= NULL;
	}
	
	if ((pWidth!=0) && (pHeight!=0)) {
		mBuffer = (sPixel *) MEMmalloc(sizeof(sPixel) * pWidth * pHeight);
		if (mBuffer != NULL) {
			mWidth	= pWidth;
			mHeight	= pHeight;
		};
	};
};

// init empty bitmap
oRGBAImage::oRGBAImage() {
	mWidth		= 0;
	mHeight		= 0;
	mBuffer		= NULL;
};

// create empty bitmap of these dimentions
oRGBAImage::oRGBAImage(qlong pWidth, qlong pHeight) {
	mWidth		= 0;
	mHeight		= 0;
	mBuffer		= NULL;
	
	initBuffer(pWidth, pHeight);
	if (mBuffer != NULL) {
		memset(mBuffer, 0xFF, sizeof(sPixel) * mWidth * mHeight);
	};
};

// create empty bitmap of these dimentions with a default color
oRGBAImage::oRGBAImage(qlong pWidth, qlong pHeight, qcol pColor) {
	mWidth		= 0;
	mHeight		= 0;
	mBuffer		= NULL;
	
	initBuffer(pWidth, pHeight);
	if (mBuffer != NULL) {
		sPixel		pixel;
		
		// get real RGB value..
		qlong col = GDIgetRealColor(pColor);
		
		// split into RGBA components
		// need to check if this is the same on windows..
		pixel.mR = col & 0xFF;
		pixel.mG = (col >> 8) & 0xFF;
		pixel.mB = (col >> 16) & 0xFF;
		// pixel.mA = (col >> 24) & 0xFF; 
		pixel.mA = 255; // note, not supplied by omnis generally so we ignore...

		for (qlong index = 0; index < (mWidth*mHeight); index++) {			
			mBuffer[index] = pixel;
		};
	};	
};

// construct as a copy
oRGBAImage::oRGBAImage(const oRGBAImage & pCopy) {
	mWidth		= 0;
	mHeight		= 0;
	mBuffer		= NULL;
	
	copy(pCopy);
};

// construct from an omnis HPIXMAP 
oRGBAImage::oRGBAImage(HPIXMAP pPixMap) {
	mWidth		= 0;
	mHeight		= 0;
	mBuffer		= NULL;
	
	copy(pPixMap);	
};

// destruct and free up memory
oRGBAImage::~oRGBAImage() {
	initBuffer(0, 0);
};

// get a pointer to our image buffer
const sPixel * oRGBAImage::imageBuffer() const {
	return mBuffer;
};

// width of our image
qlong oRGBAImage::width() const {
	return mWidth;
};

// height of our image
qlong oRGBAImage::height() const {
	return mHeight;
};

// copy another image
void oRGBAImage::copy(const oRGBAImage & pCopy) {
	initBuffer(pCopy.width(), pCopy.height());
	if (mBuffer != NULL) {
		memcpy(mBuffer, pCopy.imageBuffer(), sizeof(sPixel) * mWidth * mHeight);
	};	
};

// copy an omnis HPIXMAP into our image
bool oRGBAImage::copy(HPIXMAP pPixMap) {
	// !BAS! need to add soem error handling into this...
	bool			successful = false;
	
	sPixel *		pixels = (sPixel *) GDIlockHPIXMAP(pPixMap);
	if (pixels != NULL) {
		HPIXMAPinfo		pixmapinfo;
		
		// get info
		GDIgetHPIXMAPinfo(pPixMap, &pixmapinfo);
		qlong			width	= pixmapinfo.mWidth;
		qlong			height	= pixmapinfo.mHeight;				
		
		initBuffer(width, height);
		if (mBuffer != NULL) {
			// at this moment HPIXMAP and oRGBAImage share the same memory structure so we can just copy...
			memcpy(mBuffer, pixels, sizeof(sPixel) * mWidth * mHeight);
			
			// we're good
			successful = true;
		};
		GDIunlockHPIXMAP(pPixMap);		
	};
	
	return successful;
};

// return our image as a HPIXMAP (calling method is responsible for freeing up the memory)
HPIXMAP oRGBAImage::asPixMap() {
	HPIXMAP pixmap = 0;
	
	// only if we have a bitmap
	if (mBuffer != NULL) {
		pixmap = GDIcreateHPIXMAP(mWidth, mHeight, 32, false);
		if (pixmap != 0) {
			sPixel *	pixels = (sPixel *) GDIlockHPIXMAP(pixmap);
			if (pixels != NULL) {
				memcpy(pixels, mBuffer, sizeof(sPixel) * mWidth * mHeight);
				
				GDIunlockHPIXMAP(pixmap);
			} else {
				GDIdeleteHPIXMAP(pixmap);
				pixmap = 0;
			};
		};
	};
	
	return pixmap;
};

// returns interpolated pixel
sPixel oRGBAImage::getPixel(float pX, float pY) const {
	sPixel pixel;
	
	if (mBuffer != NULL) {
		if (pX < 0.0) pX = 0.0; // ???
		if (pY < 0.0) pY = 0.0; // ???	
		
		qlong	intX		= floor(pX);
		qlong	intY		= floor(pY);
		bool	interpolate	= true;
		
		// make sure we stay within our buffer...
		if (intX >= (mWidth - 1)) {
			intX			= mWidth - 1;
			interpolate		= false;
		};
		
		if (intY >= (mHeight - 1)) {
			intY			= mHeight - 1;
			interpolate		= false;
		};
		
		// calculate our top/left offset
		qlong	offset		= intY;
		offset *= mWidth;
		offset += intX;
		
		// check if we need to interpolate
		if (interpolate) {
			float	fractX		= pX - intX;
			float	fractY		= pY - intY;
			
			if ((fractX > 0.0) || (fractY > 0.0)) {
				sPixel	topcol		= interpolatePixel(mBuffer[offset], mBuffer[offset+1], fractX);
				offset += mWidth;
				sPixel	bottomcol	= interpolatePixel(mBuffer[offset], mBuffer[offset+1], fractX);
				
				pixel = interpolatePixel(topcol, bottomcol, fractY);
			} else {
				pixel = mBuffer[offset];	
			};
		} else {
			pixel = mBuffer[offset];		
		};		
	} else {
		pixel.mR = 0;
		pixel.mG = 0;
		pixel.mB = 0;
		pixel.mA = 0;
		
	};
	
	return pixel;	
};

////////////////////////////////////////////////////////////////////////////////////
// operators
////////////////////////////////////////////////////////////////////////////////////

// (X,Y) operator (direct access to pixel)
sPixel & oRGBAImage::operator()(qlong pX, qlong pY) {
	static sPixel pixel;
	
	if (mBuffer != NULL) {
		if ((pX>=0) && (pX<mWidth) && (pY>=0) && (pY<mHeight)) {
			return mBuffer[pX + (pY * mWidth)];
		};
	};
	
	pixel.mR = 0;
	pixel.mG = 0;
	pixel.mB = 0;
	pixel.mA = 0;
	
	return pixel;
};

////////////////////////////////////////////////////////////////////////////////////
// static functions
////////////////////////////////////////////////////////////////////////////////////

sPixel	oRGBAImage::mixPixel(sPixel pBase, sPixel pAdd, qbyte pAlpha) {
	if ((pAlpha == 255) && (pAdd.mA == 255)) {
		// opaque pixel...
		return pAdd;
	} else if ((pAlpha == 0) || (pAdd.mA == 0)) {		
		// fully transparent pixel
		return pBase;
	} else {
		sPixel	result;
		qlong	value, alpha = pAlpha;
		
		alpha		*= pAdd.mA;
		result.mA	= 255;
		
		// Red
		value		= pBase.mR;
		value		*= (255 * 255) - alpha;
		result.mR	= value >> 16;
		
		value		= pAdd.mR;
		value		*= alpha;
		result.mR	+= (value >> 16);
		
		// Green
		value		= pBase.mG;
		value		*= (255 * 255) - alpha;
		result.mG	= value >> 16;
		
		value		= pAdd.mG;
		value		*= alpha;
		result.mG	+= (value >> 16);
		
		// blue
		value		= pBase.mB;
		value		*= (255 * 255) - alpha;
		result.mB	= value >> 16;
		
		value		= pAdd.mB;
		value		*= alpha;
		result.mB	+= (value >> 16);
		
		return result;
	};
};

// interpolate pixel
sPixel	oRGBAImage::interpolatePixel(sPixel pA, sPixel pB, float pFract) {
	sPixel	result;
	float	delta;
	
	if (pA.mR == pB.mR) {
		result.mR = pA.mR;
	} else {
		delta		= pB.mR - pA.mR;
		delta		*= pFract;
		result.mR	= pA.mR + delta;		
	};
	
	if (pA.mG == pB.mG) {
		result.mG = pA.mG;
	} else {
		delta		= pB.mG - pA.mG;
		delta		*= pFract;
		result.mG	= pA.mG + delta;
	};
	
	if (pA.mB == pB.mB) {
		result.mB = pA.mB;
	} else {
		delta		= pB.mB - pA.mB;
		delta		*= pFract;
		result.mB	= pA.mB + delta;
	};
	
	if (pA.mA == pB.mA) {
		result.mA = pA.mA;
	} else {
		delta		= pB.mA - pA.mA;
		delta		*= pFract;
		result.mA	= pA.mA + delta;
	};
	
	return result;
};
