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

// Include our support library
#define STBI_FAILURE_USERMSG
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
// #define STB_DEFINE
// #include "stb/stb.h"

// initialise our buffer..
void oRGBAImage::initBuffer(qlong pWidth, qlong pHeight) {
	// free any buffer we still have...
	if (mBuffer != NULL) {
		MEMfree(mBuffer);
		mWidth = 0;
		mHeight = 0;
		mBuffer = NULL;
	}

	if ((pWidth != 0) && (pHeight != 0)) {
		mBuffer = (sPixel *)MEMmalloc(sizeof(sPixel) * pWidth * pHeight);
		if (mBuffer != NULL) {
			mWidth = pWidth;
			mHeight = pHeight;
		};
	};
};

// init empty bitmap
oRGBAImage::oRGBAImage() {
	mWidth = 0;
	mHeight = 0;
	mBuffer = NULL;
};

// create empty bitmap of these dimentions
oRGBAImage::oRGBAImage(qlong pWidth, qlong pHeight) {
	mWidth = 0;
	mHeight = 0;
	mBuffer = NULL;

	initBuffer(pWidth, pHeight);
	if (mBuffer != NULL) {
		memset(mBuffer, 0xFF, sizeof(sPixel) * mWidth * mHeight);
	};
};

// create empty bitmap of these dimentions with a default color
oRGBAImage::oRGBAImage(qlong pWidth, qlong pHeight, qcol pColor) {
	mWidth = 0;
	mHeight = 0;
	mBuffer = NULL;

	initBuffer(pWidth, pHeight);
	if (mBuffer != NULL) {
		sPixel pixel;

		// get real RGB value..
		qlong col = GDIgetRealColor(pColor);

		// split into RGBA components
		// need to check if this is the same on windows..
		pixel.mR = col & 0xFF;
		pixel.mG = (col >> 8) & 0xFF;
		pixel.mB = (col >> 16) & 0xFF;
		// pixel.mA = (col >> 24) & 0xFF;
		pixel.mA = 255; // note, not supplied by omnis generally so we ignore...

		for (qlong index = 0; index < (mWidth * mHeight); index++) {
			mBuffer[index] = pixel;
		};
	};
};

// construct as a copy
oRGBAImage::oRGBAImage(const oRGBAImage &pCopy) {
	mWidth = 0;
	mHeight = 0;
	mBuffer = NULL;

	copy(pCopy);
};

// construct from a binary image (see STB library for supported formats)
oRGBAImage::oRGBAImage(qbyte *pBuffer, qlong pSize) {
	mWidth = 0;
	mHeight = 0;
	mBuffer = NULL;

	copy(pBuffer, pSize);
};

// destruct and free up memory
oRGBAImage::~oRGBAImage() {
	initBuffer(0, 0);
};

// get a pointer to our image buffer
const sPixel *oRGBAImage::imageBuffer() const {
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
void oRGBAImage::copy(const oRGBAImage &pCopy) {
	initBuffer(pCopy.width(), pCopy.height());
	if (mBuffer != NULL) {
		memcpy(mBuffer, pCopy.imageBuffer(), sizeof(sPixel) * mWidth * mHeight);
	};
};

// cpoy a binary image into our image (see STB library for supported formats)
bool oRGBAImage::copy(qbyte *pBuffer, qlong pSize) {
	bool successful = false;
	int X = 0, Y = 0, Comp = 0;

	// this will decompress our image and
	unsigned char *pixeldata = stbi_load_from_memory(pBuffer, pSize, &X, &Y, &Comp, 4);
	if (pixeldata == NULL) {
		// Need to implement stbi_failure_reason() to get more info...
		oBaseComponent::addToTraceLog("copy: can't decode image data - %s", stbi_failure_reason());
	} else {
		if (Comp == 3) {
			initBuffer(X, Y);
			if (mBuffer != NULL) {
				qbyte *data = pixeldata;
				for (qlong i = 0; i < X * Y; i++) {
					mBuffer[i].mR = *data;
					data++;
					mBuffer[i].mG = *data;
					data++;
					mBuffer[i].mB = *data;
					data++;
					mBuffer[i].mA = 255;
				};

				successful = true;
			};
		} else if (Comp == 4) {
			//			oBaseComponent::addToTraceLog("copy: loaded image %i, %i - %i", X, Y, Comp);

			// in theory we should be able to use our pixel data directly but we'll take the safe approach and copy it into our own buffer
			initBuffer(X, Y);
			if (mBuffer != NULL) {
				memcpy(mBuffer, pixeldata, 4 * X * Y);

				successful = true;
			};
		} else {
			oBaseComponent::addToTraceLog("copy: couldn't convert image to RGBA");
		};

		// and free up...
		stbi_image_free(pixeldata);
	};

	return successful;
};

// returns our image as a PNG (calling method is responsible for freeing up the memory using free)
qbyte *oRGBAImage::asPNG(int &pLen) {
	// note that according to the STB documentation to compression used by this PNG logic isn't very good
	// its implemented for simplicity but it does what we need it to do...
	// you can always have Omnis recompress it as Omnis has a full PNG implementation
	int len;
	unsigned char *png = stbi_write_png_to_mem((unsigned char *)mBuffer, 0, mWidth, mHeight, 4, &len);
	if (png == NULL) {
		pLen = 0;
		oBaseComponent::addToTraceLog("asPNG: Couldn't convert to PNG");
	} else {
		pLen = len;
	};
	return png;
};

// returns interpolated pixel
sPixel oRGBAImage::getPixel(float pX, float pY) const {
	sPixel pixel;

	if (mBuffer != NULL) {
		if (pX < 0.0) pX = 0.0; // ???
		if (pY < 0.0) pY = 0.0; // ???

		qlong intX = (qlong)floor(pX);
		qlong intY = (qlong)floor(pY);
		bool interpolate = true;

		// make sure we stay within our buffer...
		if (intX >= (mWidth - 1)) {
			intX = mWidth - 1;
			interpolate = false;
		};

		if (intY >= (mHeight - 1)) {
			intY = mHeight - 1;
			interpolate = false;
		};

		// calculate our top/left offset
		qlong offset = intY;
		offset *= mWidth;
		offset += intX;

		// check if we need to interpolate
		if (interpolate) {
			float fractX = pX - intX;
			float fractY = pY - intY;

			if ((fractX > 0.0) || (fractY > 0.0)) {
				sPixel topcol = interpolatePixel(mBuffer[offset], mBuffer[offset + 1], fractX);
				offset += mWidth;
				sPixel bottomcol = interpolatePixel(mBuffer[offset], mBuffer[offset + 1], fractX);

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
sPixel &oRGBAImage::operator()(qlong pX, qlong pY) {
	static sPixel pixel;

	if (mBuffer != NULL) {
		if ((pX >= 0) && (pX < mWidth) && (pY >= 0) && (pY < mHeight)) {
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

sPixel oRGBAImage::mixPixel(sPixel pBase, sPixel pAdd, qbyte pAlpha) {
	if ((pAlpha == 255) && (pAdd.mA == 255)) {
		// opaque pixel...
		return pAdd;
	} else if ((pAlpha == 0) || (pAdd.mA == 0)) {
		// fully transparent pixel
		return pBase;
	} else {
		sPixel result;
		qlong value, alpha = pAlpha;

		alpha *= pAdd.mA;
		result.mA = 255;

		// Red
		value = pBase.mR;
		value *= (255 * 255) - alpha;
		result.mR = value >> 16;

		value = pAdd.mR;
		value *= alpha;
		result.mR += (value >> 16);

		// Green
		value = pBase.mG;
		value *= (255 * 255) - alpha;
		result.mG = value >> 16;

		value = pAdd.mG;
		value *= alpha;
		result.mG += (value >> 16);

		// blue
		value = pBase.mB;
		value *= (255 * 255) - alpha;
		result.mB = value >> 16;

		value = pAdd.mB;
		value *= alpha;
		result.mB += (value >> 16);

		return result;
	};
};

// interpolate pixel
sPixel oRGBAImage::interpolatePixel(sPixel pA, sPixel pB, float pFract) {
	sPixel result;
	float delta;

	if (pA.mR == pB.mR) {
		result.mR = pA.mR;
	} else {
		delta = (float)(pB.mR - pA.mR);
		delta *= pFract;
		result.mR = (qbyte)(pA.mR + delta);
	};

	if (pA.mG == pB.mG) {
		result.mG = pA.mG;
	} else {
		delta = (float)(pB.mG - pA.mG);
		delta *= pFract;
		result.mG = (qbyte)(pA.mG + delta);
	};

	if (pA.mB == pB.mB) {
		result.mB = pA.mB;
	} else {
		delta = (float)(pB.mB - pA.mB);
		delta *= pFract;
		result.mB = (qbyte)(pA.mB + delta);
	};

	if (pA.mA == pB.mA) {
		result.mA = pA.mA;
	} else {
		delta = (float)(pB.mA - pA.mA);
		delta *= pFract;
		result.mA = (qbyte)(pA.mA + delta);
	};

	return result;
};
