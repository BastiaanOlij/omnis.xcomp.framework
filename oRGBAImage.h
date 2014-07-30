/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oRGBAImage.h
 *
 *  RGBA bitmap class
 *  The Omnis SDK has a really powerful set of device dependent bitmap functions. It also has powerful functions around HPIXMAP for device independent functions.
 *  But the documentation is poor and I've had alot of stability issues with them.
 *
 *  This class is a simple and raw implementation for bitmaps. In the future we may change this object to simply encapsulate an HPIXMAP object.
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */


#include "xCompStandardIncludes.h"

#ifndef orgbaimageh
#define orgbaimageh

// pixel structure as its used in a 32bit HPIXMAP, we'll use this here as well. 
// tested on Mac, may need to change on windows..
typedef struct sPixel {
	qbyte	mA;
	qbyte	mR;
	qbyte	mG;
	qbyte	mB;
} sPixel;

class oRGBAImage {
private:
	qlong		mWidth;
	qlong		mHeight;
	sPixel *	mBuffer;
	
	void		initBuffer(qlong pWidth, qlong pHeight);	// initialise our buffer..
	
protected:
	
public:
	oRGBAImage();											// init empty bitmap
	oRGBAImage(qlong pWidth, qlong pHeight);				// create empty bitmap of these dimentions
	oRGBAImage(qlong pWidth, qlong pHeight, qcol pColor);	// create empty bitmap of these dimentions with a default color
	oRGBAImage(const oRGBAImage & pCopy);					// construct as a copy
	oRGBAImage(HPIXMAP pPixMap);							// construct from an omnis HPIXMAP 
	~oRGBAImage();											// destruct and free up memory
	
	const sPixel *	imageBuffer() const;					// get a pointer to our image buffer
	qlong			width() const;							// width of our image
	qlong			height() const;							// height of our image
	
	void			copy(const oRGBAImage & pCopy);			// copy another image
	bool			copy(HPIXMAP pPixMap);					// copy an omnis HPIXMAP into our image
	
	HPIXMAP			asPixMap();								// return our image as a HPIXMAP (calling method is responsible for freeing up the memory)
	
	sPixel			getPixel(float pX, float pY) const;		// returns interpolated pixel

	// operators
	sPixel &		operator()(qlong pX, qlong pY);			// (X, Y) operator (direct access to pixel)
	
	// handy static functions
	static sPixel	mixPixel(sPixel pBase, sPixel pAdd, qbyte pAlpha);
	static sPixel	interpolatePixel(sPixel pA, sPixel pB, float pFract);
	
};

#endif
