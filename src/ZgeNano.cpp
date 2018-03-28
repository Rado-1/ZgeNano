/*
ZgeNano Library
Copyright (c) 2016-2018 Radovan Cervenka

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
*/

/// The main file used to compile Windows DLL and Android shared library
/// Version: 1.1 (2018 - 01 - 09)


// Definitions

#define DONE 0
#define ERROR -1
//typedef void* ptr;

//#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#undef NULL
#define NULL nullptr
//#else
//#define EXPORT extern "C"
//#endif


// Includes

#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <regex>

//#ifdef _WIN32
#define GLEW_STATIC
#include<GL/glew.h>

//#else // ANDROID
//#include<GLES/gl.h>
//#endif

#define NANOVG_GL2_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"

#define NANOSVG_ALL_COLOR_KEYWORDS
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

#include "tinf.h"

// Types

/* ZgeNano execution context. */
struct ZNVGcontext {

	ZNVGcontext(NVGcontext* vg) :
		vg(vg),
		winWidth(0),
		winHeight(0),
		rasterizer(NULL) {}

	~ZNVGcontext() {
		if (rasterizer != NULL)
			nsvgDeleteRasterizer(rasterizer);

		// delete images
		for (auto iter = images.begin(); iter != images.end(); ++iter) {
			nvgDeleteImage(vg, *iter);
		}

		// delete paints
		for (auto iter = paints.begin(); iter != paints.end(); ++iter) {
			if (*iter != NULL)
				delete(*iter);
		}

		// delete SVG images
		for (auto iter = svgImages.begin(); iter != svgImages.end(); ++iter) {
			delete(*iter);
		}

		nvgDeleteGL2(vg);
	}

	struct NVGcontext* vg;
	int winWidth, winHeight;
	float devicePixelRatio;
	struct NSVGrasterizer* rasterizer;

	// allocated resources deleted on destruction
	std::vector<int> images;
	std::vector<NVGpaint*> paints;
	std::vector<NSVGimage*> svgImages;
};


// Auxiliary

template <typename T> inline void remove(std::vector<T> &vec, T element) {
	auto it = std::find(vec.begin(), vec.end(), element);
	if (it != vec.end()) {
		std::swap(*it, vec.back());
		vec.pop_back();
	}
}

inline NVGcolor COLOR(float r, float g, float b, float a) {
	return nvgRGBAf(r, g, b, a);
}

inline NVGcolor SVGCOLOR(unsigned int c, float r, float g, float b, float a) {
	return nvgRGBA((c & 0xFF) * r, ((c >> 8) & 0xFF) * g, ((c >> 16) & 0xFF) * b, ((c >> 24) & 0xFF) * a);
}

// Globals

ZNVGcontext* currentContext;
std::unordered_map<void*, ZNVGcontext*> contexts;
GLint viewport[4];
bool initializeTinf = true;


// Init

EXPORT void nvg_SetViewport() {
	glGetIntegerv(GL_VIEWPORT, viewport);
	currentContext->winWidth = viewport[2];
	currentContext->winHeight = viewport[3];
	currentContext->devicePixelRatio = viewport[3] != 0 ? (float) viewport[2] / (float) viewport[3] : 0;
}

EXPORT ZNVGcontext* nvg_SetContext(void* contextKey) {
	auto search = contexts.find(contextKey);
	if (search != contexts.end())
		currentContext = search->second;
	return currentContext;
}

EXPORT ZNVGcontext* nvg_Init(int flags, void* contextKey) {

	auto search = contexts.find(contextKey);

	if (search == contexts.end()) {
		// context not found => create new one
		struct NVGcontext* vg;

		if (glewInit() != GLEW_OK) return NULL;
		vg = nvgCreateGL2(flags);
		if (vg == NULL) return NULL;

		currentContext = new ZNVGcontext(vg);

		contexts.insert({ contextKey, currentContext });
		nvg_SetViewport();
	} else {
		// context already exist
		currentContext = search->second;
	}

	return currentContext;
}

EXPORT void nvg_Finish(void* contextKey) {
	auto search = contexts.find(contextKey);

	if (search != contexts.end()) {
		delete search->second;
		contexts.erase(search);
	}
}


// Drawing

EXPORT void nvg_BeginFrame() {
	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	nvgBeginFrame(currentContext->vg , currentContext->winWidth, currentContext->winHeight, currentContext->devicePixelRatio);
}

EXPORT void nvg_CancelFrame() {
	nvgCancelFrame(currentContext->vg);
	glPopAttrib();
}

EXPORT void nvg_EndFrame() {
	nvgEndFrame(currentContext->vg);
	glPopAttrib();
}


// Global composite operation

EXPORT void nvg_GlobalCompositeOperation(int op) {
	nvgGlobalCompositeOperation(currentContext->vg,op);
}

EXPORT void nvg_GlobalCompositeBlendFunc(int sfactor, int dfactor) {
	nvgGlobalCompositeBlendFunc(currentContext->vg, sfactor, dfactor);
}

EXPORT void nvg_GlobalCompositeBlendFuncSeparate(int srcRGB, int dstRGB, int srcAlpha, int dstAlpha) {
	nvgGlobalCompositeBlendFuncSeparate(currentContext->vg, srcRGB, dstRGB, srcAlpha, dstAlpha);
}


// State handling

EXPORT void nvg_Save() {
	nvgSave(currentContext->vg);
}

EXPORT void nvg_Restore() {
	nvgRestore(currentContext->vg);
}

EXPORT void nvg_Reset() {
	nvgReset(currentContext->vg);
}


// Render styles

EXPORT void nvg_StrokeColor(float r, float g, float b, float a) {
	nvgStrokeColor(currentContext->vg, COLOR(r, g, b, a));
}

EXPORT void nvg_StrokePaint(NVGpaint paint) {
	nvgStrokePaint(currentContext->vg, paint);
}

EXPORT void nvg_FillColor(float r, float g, float b, float a) {
	nvgFillColor(currentContext->vg, COLOR(r, g, b, a));
}

EXPORT void nvg_FillPaint(NVGpaint* paint) {
	nvgFillPaint(currentContext->vg, *paint);
}

EXPORT void nvg_MiterLimit(float limit) {
	nvgMiterLimit(currentContext->vg, limit);
}

EXPORT void nvg_StrokeWidth(float size) {
	nvgStrokeWidth(currentContext->vg, size);
}

EXPORT void nvg_LineCap(int cap) {
	nvgLineCap(currentContext->vg, cap);
}

EXPORT void nvg_LineJoin(int join) {
	nvgLineJoin(currentContext->vg, join);
}

EXPORT void nvg_GlobalAlpha(float alpha) {
	nvgGlobalAlpha(currentContext->vg, alpha);
}


// Transformations

EXPORT void nvg_ResetTransform() {
	nvgResetTransform(currentContext->vg);
}

EXPORT void nvg_Transform(float a, float b, float c, float d, float e, float f) {
	nvgTransform(currentContext->vg, a, b, c, d, e, f);
}

EXPORT void nvg_Translate(float x, float y) {
	nvgTranslate(currentContext->vg, x, y);
}

EXPORT void nvg_Rotate(float angle) {
	nvgRotate(currentContext->vg, angle);
}

EXPORT void nvg_SkewX(float angle) {
	nvgSkewX(currentContext->vg, angle);
}

EXPORT void nvg_SkewY(float angle) {
	nvgSkewY(currentContext->vg, angle);
}

EXPORT void nvg_Scale(float x, float y) {
	nvgScale(currentContext->vg, x, y);
}

// Note: Functions for working with float[6] matrices are not included (yet).


// Images

EXPORT int nvg_CreateImage(const char* filename, int imageFlags) {
	int i = nvgCreateImage(currentContext->vg, filename, imageFlags);
	currentContext->images.push_back(i);
	return i;
}

EXPORT int nvg_CreateImageMem(int imageFlags, unsigned char* data, int ndata) {
	int i = nvgCreateImageMem(currentContext->vg, imageFlags, data, ndata);
	currentContext->images.push_back(i);
	return i;
}

EXPORT int nvg_CreateImageRGBA(int w, int h, int imageFlags, const unsigned char* data) {
	int i = nvgCreateImageRGBA(currentContext->vg, w, h, imageFlags, data);
	currentContext->images.push_back(i);
	return i;
}

EXPORT void nvg_UpdateImage(int image, const unsigned char* data) {
	nvgUpdateImage(currentContext->vg, image, data);
}

EXPORT void nvg_ImageSize(int image, int* w, int* h) {
	nvgImageSize(currentContext->vg, image, w, h);
}

EXPORT void nvg_DeleteImage(int image) {
	remove(currentContext->images, image);
	nvgDeleteImage(currentContext->vg, image);
}


// Paints

EXPORT NVGpaint* nvg_LinearGradient(float sx, float sy, float ex, float ey,
	float ir, float ig, float ib, float ia,
	float or, float og, float ob, float oa) {

	NVGpaint* ret = new NVGpaint(nvgLinearGradient(currentContext->vg, sx, sy, ex, ey, COLOR(ir, ig, ib, ia), COLOR(or , og, ob, oa)));
	currentContext->paints.push_back(ret);
	return ret;
}

EXPORT NVGpaint* nvg_BoxGradient(float x, float y, float w, float h, float r, float f,
	float ir, float ig, float ib, float ia,
	float or, float og, float ob, float oa) {

	NVGpaint* ret = new NVGpaint(nvgBoxGradient(currentContext->vg, x, y, w, h, r, f, COLOR(ir, ig, ib, ia), COLOR(or , og, ob, oa)));
	currentContext->paints.push_back(ret);
	return ret;
}

EXPORT NVGpaint* nvg_RadialGradient(float cx, float cy, float inr, float outr,
	float ir, float ig, float ib, float ia,
	float or, float og, float ob, float oa) {

	NVGpaint* ret = new NVGpaint(nvgRadialGradient(currentContext->vg, cx, cy, inr, outr, COLOR(ir, ig, ib, ia), COLOR(or , og, ob, oa)));
	currentContext->paints.push_back(ret);
	return ret;
}

EXPORT NVGpaint* nvg_ImagePattern(float ox, float oy, float ex, float ey,
	float angle, int image, float alpha) {

	NVGpaint* ret = new NVGpaint(nvgImagePattern(currentContext->vg, ox, oy, ex, ey, angle, image, alpha));
	currentContext->paints.push_back(ret);
	return ret;
}

EXPORT void nvg_FreePaint(NVGpaint* paint) {
	if (paint != NULL) {
		remove(currentContext->paints, paint);
		delete paint;
	}
}


// Scissoring

EXPORT void nvg_Scissor(float x, float y, float w, float h) {
	nvgScissor(currentContext->vg, x, y, w, h);
}

EXPORT void nvg_IntersectScissor(float x, float y, float w, float h) {
	nvgIntersectScissor(currentContext->vg, x, y, w, h);
}

EXPORT void nvg_ResetScissor() {
	nvgResetScissor(currentContext->vg);
}


// Paths

EXPORT void nvg_BeginPath() {
	nvgBeginPath(currentContext->vg);
}

EXPORT void nvg_MoveTo(float x, float y) {
	nvgMoveTo(currentContext->vg, x, y);
}

EXPORT void nvg_LineTo(float x, float y) {
	nvgLineTo(currentContext->vg, x, y);
}

EXPORT void nvg_BezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y) {
	nvgBezierTo(currentContext->vg, c1x, c1y, c2x, c2y, x, y);
}

EXPORT void nvg_QuadTo(float cx, float cy, float x, float y) {
	nvgQuadTo(currentContext->vg, cx, cy, x, y);
}

EXPORT void nvg_ArcTo(float x1, float y1, float x2, float y2, float radius) {
	nvgArcTo(currentContext->vg, x1, y1, x2, y2, radius);
}

EXPORT void nvg_ClosePath() {
	nvgClosePath(currentContext->vg);
}

EXPORT void nvg_PathWinding(int dir) {
	nvgPathWinding(currentContext->vg, dir);
}

EXPORT void nvg_Arc(float cx, float cy, float r, float a0, float a1, int dir) {
	nvgArc(currentContext->vg, cx, cy, r, a0, a1, dir);
}

EXPORT void nvg_Rect(float x, float y, float w, float h) {
	nvgRect(currentContext->vg, x, y, w, h);
}

EXPORT void nvg_RoundedRect(float x, float y, float w, float h, float r) {
	nvgRoundedRect(currentContext->vg, x, y, w, h, r);
}

EXPORT void nvg_RoundedRectVarying(float x, float y, float w, float h,
	float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) {
	
	nvgRoundedRectVarying(currentContext->vg, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
}

EXPORT void nvg_Ellipse(float cx, float cy, float rx, float ry) {
	nvgEllipse(currentContext->vg, cx, cy, rx, ry);
}

EXPORT void nvg_Circle(float cx, float cy, float r) {
	nvgCircle(currentContext->vg, cx, cy, r);
}

EXPORT void nvg_Fill() {
	nvgFill(currentContext->vg);
}

EXPORT void nvg_Stroke() {
	nvgStroke(currentContext->vg);
}

EXPORT void nvg_StrokeNoScale() {
	nvgStrokeNoScale(currentContext->vg);
}


// Text

EXPORT int nvg_CreateFont(const char* name, const char* filename) {
	return nvgCreateFont(currentContext->vg, name, filename);
}

EXPORT int nvg_CreateFontMem(const char* name, unsigned char* data, int ndata, int freeData) {
	return nvgCreateFontMem(currentContext->vg, name, data, ndata, freeData);
}

EXPORT int nvg_FindFont(const char* name) {
	return nvgFindFont(currentContext->vg, name);
}

EXPORT int nvg_AddFallbackFontId(int baseFont, int fallbackFont) {
	return nvgAddFallbackFontId(currentContext->vg, baseFont, fallbackFont);
}

EXPORT int nvg_AddFallbackFont(const char* baseFont, const char* fallbackFont) {
	return nvgAddFallbackFont(currentContext->vg, baseFont, fallbackFont);
}

EXPORT void nvg_FontSize(float size) {
	nvgFontSize(currentContext->vg, size);
}

EXPORT void nvg_FontBlur(float blur) {
	nvgFontBlur(currentContext->vg, blur);
}

EXPORT void nvg_TextLetterSpacing(float spacing) {
	nvgTextLetterSpacing(currentContext->vg, spacing);
}

EXPORT void nvg_TextLineHeight(float lineHeight) {
	nvgTextLineHeight(currentContext->vg, lineHeight);
}

EXPORT void nvg_TextAlign(int align) {
	nvgTextAlign(currentContext->vg, align);
}

EXPORT void nvg_FontFaceId(int font) {
	nvgFontFaceId(currentContext->vg, font);
}

EXPORT void nvg_FontFace(const char* font) {
	nvgFontFace(currentContext->vg, font);
}

EXPORT void nvg_Text(float x, float y, const char* str, const char* end) {
	nvgText(currentContext->vg, x, y, str, end);
}

EXPORT void nvg_TextBox(float x, float y, float breakRowWidth, const char* str, const char* end) {
	nvgTextBox(currentContext->vg, x, y, breakRowWidth, str, end);
}

EXPORT float nvg_TextBounds(float x, float y, const char* str, const char* end, float* bounds) {
	return nvgTextBounds(currentContext->vg, x, y, str, end, bounds);
}

EXPORT void nvg_TextBoxBounds(float x, float y, float breakRowWidth, const char* str, const char* end, float* bounds) {
	nvgTextBoxBounds(currentContext->vg, x, y, breakRowWidth, str, end, bounds);
}

EXPORT void nvg_TextMetrics(float* ascender, float* descender, float* lineh) {
	nvgTextMetrics(currentContext->vg, ascender, descender, lineh);
}

// Note: nvgTextGlyphPositions and nvgTextBreakLines are not supported due to relatively
// complicated return value processing.


// SVG support

EXPORT NSVGimage* nsvg_ParseFromFile(const char* filename, const char* units, float dpi) {
	
	if((strstr(filename, ".svgz") - filename) != (strlen(filename) - 5))
		// parse SVG file
		return nsvgParseFromFile(filename, units, dpi);
	else {
		// parse SVGZ file (compressed SVG)

		// lazy initialization of tinf
		if (initializeTinf) {
			tinf_init();
			initializeTinf = false;
		}

		FILE* fp = NULL;
		size_t size;
		unsigned char* data = NULL;
		NSVGimage* image = NULL;
		unsigned char* svgData = NULL;
		unsigned int svgSize;

		// read file
		fp = fopen(filename, "rb");
		if (!fp) goto error;
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		data = (unsigned char*)malloc(size);
		if (data == NULL) goto error;
		if (fread(data, 1, size, fp) != size) goto error;
		fclose(fp);

		// get decompressed length
		svgSize =				  data[size - 1];
		svgSize = 256 * svgSize + data[size - 2];
		svgSize = 256 * svgSize + data[size - 3];
		svgSize = 256 * svgSize + data[size - 4];

		svgData = (unsigned char*)malloc(svgSize + 1);
		if (svgData == NULL) goto error;

		// uncompress
		if (tinf_gzip_uncompress(svgData, &svgSize, data, size) != TINF_OK) goto error;
		free(data);

		// SVG must be null terminated
		svgData[svgSize] = '\0';

		image = nsvgParse((char*)svgData, units, dpi);
		free(svgData);

		return image;

	error:
		if (fp) fclose(fp);
		if (data) free(data);
		if (svgData) free(svgData);
		if (image) nsvgDelete(image);
		return NULL;
	}
}

EXPORT NSVGimage* nsvg_ParseMem(unsigned char* data, int ndata, const char* units, float dpi) {
	char* buf = static_cast<char*>(malloc(ndata + 1));
	memcpy(buf, data, ndata);
	buf[ndata] = '\0';
	NSVGimage* ret = nsvgParse(buf, units, dpi);
	free(buf);
	return ret;
}

NVGpaint createLinearGradient(NSVGgradient* gradient, float r, float g, float b, float a) {
	float inverse[6];
	float sx, sy, ex, ey;

	nvgTransformInverse(inverse, gradient->xform);
	nvgTransformPoint(&sx, &sy, inverse, 0, 0);
	nvgTransformPoint(&ex, &ey, inverse, 0, 1);

	return nvgLinearGradient(currentContext->vg, sx, sy, ex, ey,
		SVGCOLOR(gradient->stops[0].color, r, g, b, a),
		SVGCOLOR(gradient->stops[gradient->nstops - 1].color, r, g, b, a));
}

NVGpaint createRadialGradient(NSVGgradient* gradient, float r, float g, float b, float a) {
	float inverse[6];
	float cx, cy, r1, r2, inr, outr;

	nvgTransformInverse(inverse, gradient->xform);
	nvgTransformPoint(&cx, &cy, inverse, 0, 0);
	nvgTransformPoint(&r1, &r2, inverse, 0, 1);
	outr = r2 - cy;
	if (gradient->nstops == 3)
		inr = gradient->stops[1].offset * outr;
	else
		inr = 0;

	NVGpaint paint = nvgRadialGradient(currentContext->vg, cx, cy, inr, outr,
		SVGCOLOR(gradient->stops[0].color, r, g, b, a),
		SVGCOLOR(gradient->stops[gradient->nstops - 1].color, r, g, g, a));
	
	return  paint;
}

EXPORT void nsvg_ImageSize(NSVGimage* image, float &width, float &height) {
	width = image->width;
	height = image->height;
}

EXPORT int nsvg_ImageShapeCount(NSVGimage* image, char* shapeIdPattern) {
	int i = 0;

	for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {

		std::regex pattern (shapeIdPattern != NULL ? shapeIdPattern : "");

		// skip invisible shape
		if (!(shape->flags & NSVG_FLAGS_VISIBLE)) continue;

		// skip shape with not matching ID pattern
		if (shapeIdPattern != NULL &&
			!std::regex_match(shape->id, pattern)) continue;
	
		i++;
	}

	return i;
}

EXPORT void nsvg_Draw(NSVGimage* image, char* shapeIdPattern, int strokeWidthScaling,
	float strokeWidthFactor, float buildUpFactorFrom, float buildUpFactorTo, float* color) {

	// skip drawing if builtup from is greter than (equal to) builtup to
	if (buildUpFactorTo >= 0 && buildUpFactorFrom >= buildUpFactorTo) return;

	int join, cap;
	float i, r, g, b, a;
	float buildUpFromAlpha = 1.0, buildUpToAlpha = 0.0;
	int buildUpFromCount = -1, buildUpToCount = -1;
	std::regex pattern(shapeIdPattern != NULL ? shapeIdPattern : "");

	// prepare build-up properties
	if (buildUpFactorFrom >= 0.0) {
		buildUpFromAlpha = 1.0 - modf(buildUpFactorFrom, &i);
		buildUpFromCount = floor(i) + 1;
	}

	if (buildUpFactorTo >= 0.0) {
		buildUpToAlpha = modf(buildUpFactorTo, &i);
		buildUpToCount = floor(i) + 1;
	}

	// custom color
	if (color == NULL) {
		r = g = b = a = 1.0;
	} else {
		r = color[0];
		g = color[1];
		b = color[2];
		a = color[3];
	}

	// iterate shapes
	for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {

		// skip invisible shape
		if (!(shape->flags & NSVG_FLAGS_VISIBLE)) continue;

		// skip shape with not matching ID pattern
		if (shapeIdPattern != NULL &&
			!std::regex_match(shape->id, pattern)) continue;

		// decrease build-up counter or finish drawing if 0
		if (--buildUpToCount == -1) break;

		// skip shape under builtup from
		if (--buildUpFromCount > 0) continue;

		// set build-up alpha for the first visible shape
		if (!buildUpFromCount)
			a *= buildUpFromAlpha;
		else if (buildUpFromCount == -1)
			a = color[3];

		// set build-up alpha for the last shape
		if (!buildUpToCount)
			a *= buildUpToAlpha;
		
		nvgBeginPath(currentContext->vg);
		bool pathHole = false;

		// draw paths
		for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
			
			if (pathHole)
				nvgPathWinding(currentContext->vg, NVG_HOLE);
			else
				pathHole = true;

			nvgMoveTo(currentContext->vg, path->pts[0], path->pts[1]);

			for (int i = 0; i < path->npts - 1; i += 3) {
				float* p = &path->pts[i * 2];
				nvgBezierTo(currentContext->vg, p[2], p[3], p[4], p[5], p[6], p[7]);
			}

			if (path->closed)
				nvgLineTo(currentContext->vg, path->pts[0], path->pts[1]);
		}

		// fill
		switch (shape->fill.type) {
		case NSVG_PAINT_COLOR:
			nvgFillColor(currentContext->vg, SVGCOLOR(shape->fill.color, r, g, b, shape->opacity * a));
			nvgFill(currentContext->vg);
			break;
		case NSVG_PAINT_LINEAR_GRADIENT:
			nvgFillPaint(currentContext->vg, createLinearGradient(shape->fill.gradient, r, g, b, shape->opacity * a));
			nvgFill(currentContext->vg);
			break;
		case NSVG_PAINT_RADIAL_GRADIENT:
			nvgFillPaint(currentContext->vg, createRadialGradient(shape->fill.gradient, r, g, b, shape->opacity * a));
			nvgFill(currentContext->vg);
			break;
		}

		// set stroke/line
		switch (shape->strokeLineJoin) {
			case NSVG_JOIN_ROUND: join = NVG_ROUND; break;
			case NSVG_JOIN_BEVEL: join = NVG_BEVEL; break;
			case NSVG_JOIN_MITER:
			default: join = NVG_MITER;
		}
		nvgLineJoin(currentContext->vg, join);
		nvgLineCap(currentContext->vg, shape->strokeLineCap); // NanoSVG has the same line cap constants values as NanoVG 
		nvgStrokeWidth(currentContext->vg, shape->strokeWidth * strokeWidthFactor);

		// draw line
		switch (shape->stroke.type) {
			case NSVG_PAINT_COLOR:
				nvgStrokeColor(currentContext->vg, SVGCOLOR(shape->stroke.color, r, g, b, shape->opacity * a));
				if (strokeWidthScaling != 0)
					nvgStroke(currentContext->vg);
				else
					nvgStrokeNoScale(currentContext->vg);
				break;
			case NSVG_PAINT_LINEAR_GRADIENT:
				nvgStrokePaint(currentContext->vg, createLinearGradient(shape->stroke.gradient, r, g, b, shape->opacity * a));
				if (strokeWidthScaling != 0)
					nvgStroke(currentContext->vg);
				else
					nvgStrokeNoScale(currentContext->vg);
				break;
			case NSVG_PAINT_RADIAL_GRADIENT:
				nvgStrokePaint(currentContext->vg, createRadialGradient(shape->stroke.gradient, r, g, b, shape->opacity * a));
				if (strokeWidthScaling != 0)
					nvgStroke(currentContext->vg);
				else
					nvgStrokeNoScale(currentContext->vg);
				break;
		}
	}
}

EXPORT void nsvg_Rasterize(NSVGimage* image, float tx, float ty, float scale, unsigned char* dst, int w, int h) {
	if(currentContext->rasterizer == NULL)
		currentContext->rasterizer = nsvgCreateRasterizer();

	nsvgRasterize(currentContext->rasterizer, image, tx, ty, scale, dst, w, h, w*4);
}

EXPORT void nsvg_Delete(NSVGimage* image) {
	remove(currentContext->svgImages, image);
	nsvgDelete(image);
}