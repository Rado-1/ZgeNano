/*
ZgeNano Library
Copyright (c) 2016 Radovan Cervenka

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


// Globals

struct NVGcontext* vg;
int winWidth, winHeight;
float devicePixelRatio;
struct NSVGrasterizer* rasterizer = NULL;


// Auxiliary

inline NVGcolor COLOR(float r, float g, float b, float a) {
	return nvgRGBAf(r, g, b, a);
}

inline NVGcolor SVGCOLOR(unsigned int c, float opacity) {
	return nvgRGBA(c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF, ((c >> 24) & 0xFF) * opacity);
}



// Init

EXPORT int nvg_Init(int flags) {
	if (glewInit() != GLEW_OK) return ERROR;

	vg = nvgCreateGL2(flags);
	if (vg == NULL) return ERROR;

	GLint m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);
	winWidth = m_viewport[2];
	winHeight = m_viewport[3];
	devicePixelRatio = static_cast<float> (winWidth / winHeight);

	return DONE;
}

EXPORT void nvg_Finish() {
	if (rasterizer != NULL)
		nsvgDeleteRasterizer(rasterizer);
	nvgDeleteGL2(vg);
}


// Drawing

EXPORT void nvg_BeginFrame() {
	glPushAttrib(GL_DEPTH_BUFFER_BIT);
	nvgBeginFrame(vg, winWidth, winHeight, devicePixelRatio);
}

EXPORT void nvg_CancelFrame() {
	nvgCancelFrame(vg);
	glPopAttrib();
}

EXPORT void nvg_EndFrame() {
	nvgEndFrame(vg);
	glPopAttrib();
}


// Global composite operation

EXPORT void nvg_GlobalCompositeOperation(int op) {
	nvgGlobalCompositeOperation(vg ,op);
}

EXPORT void nvg_GlobalCompositeBlendFunc(int sfactor, int dfactor) {
	nvgGlobalCompositeBlendFunc(vg, sfactor, dfactor);
}

EXPORT void nvg_GlobalCompositeBlendFuncSeparate(int srcRGB, int dstRGB, int srcAlpha, int dstAlpha) {
	nvgGlobalCompositeBlendFuncSeparate(vg, srcRGB, dstRGB, srcAlpha, dstAlpha);
}


// State handling

EXPORT void nvg_Save() {
	nvgSave(vg);
}

EXPORT void nvg_Restore() {
	nvgRestore(vg);
}

EXPORT void nvg_Reset() {
	nvgReset(vg);
}


// Render styles

EXPORT void nvg_StrokeColor(float r, float g, float b, float a) {
	nvgStrokeColor(vg, COLOR(r, g, b, a));
}

EXPORT void nvg_StrokePaint(NVGpaint paint) {
	nvgStrokePaint(vg, paint);
}

EXPORT void nvg_FillColor(float r, float g, float b, float a) {
	nvgFillColor(vg, COLOR(r, g, b, a));
}

EXPORT void nvg_FillPaint(NVGpaint* paint) {
	nvgFillPaint(vg, *paint);
}

EXPORT void nvg_MiterLimit(float limit) {
	nvgMiterLimit(vg, limit);
}

EXPORT void nvg_StrokeWidth(float size) {
	nvgStrokeWidth(vg, size);
}

EXPORT void nvg_LineCap(int cap) {
	nvgLineCap(vg, cap);
}

EXPORT void nvg_LineJoin(int join) {
	nvgLineJoin(vg, join);
}

EXPORT void nvg_GlobalAlpha(float alpha) {
	nvgGlobalAlpha(vg, alpha);
}


// Transformations

EXPORT void nvg_ResetTransform() {
	nvgResetTransform(vg);
}

EXPORT void nvg_Transform(float a, float b, float c, float d, float e, float f) {
	nvgTransform(vg, a, b, c, d, e, f);
}

EXPORT void nvg_Translate(float x, float y) {
	nvgTranslate(vg, x, y);
}

EXPORT void nvg_Rotate(float angle) {
	nvgRotate(vg, angle);
}

EXPORT void nvg_SkewX(float angle) {
	nvgSkewX(vg, angle);
}

EXPORT void nvg_SkewY(float angle) {
	nvgSkewY(vg, angle);
}

EXPORT void nvg_Scale(float x, float y) {
	nvgScale(vg, x, y);
}

// Note: Functions for working with float[6] matrices are not included (yet).


// Images

EXPORT int nvg_CreateImage(const char* filename, int imageFlags) {
	return nvgCreateImage(vg, filename, imageFlags);
}

EXPORT int nvg_CreateImageMem(int imageFlags, unsigned char* data, int ndata) {
	return nvgCreateImageMem(vg, imageFlags, data, ndata);
}

EXPORT int nvg_CreateImageRGBA(int w, int h, int imageFlags, const unsigned char* data) {
	return nvgCreateImageRGBA(vg, w, h, imageFlags, data);
}

EXPORT void nvg_UpdateImage(int image, const unsigned char* data) {
	nvgUpdateImage(vg, image, data);
}

EXPORT void nvg_ImageSize(int image, int* w, int* h) {
	nvgImageSize(vg, image, w, h);
}

EXPORT void nvg_DeleteImage(int image) {
	nvgDeleteImage(vg, image);
}


// Paints

EXPORT NVGpaint* nvg_LinearGradient(float sx, float sy, float ex, float ey,
	float ir, float ig, float ib, float ia,
	float or, float og, float ob, float oa) {

	NVGpaint* ret = new NVGpaint(nvgLinearGradient(vg, sx, sy, ex, ey, COLOR(ir, ig, ib, ia), COLOR(or , og, ob, oa)));
	return ret;
}

EXPORT NVGpaint* nvg_BoxGradient(float x, float y, float w, float h, float r, float f,
	float ir, float ig, float ib, float ia,
	float or, float og, float ob, float oa) {

	NVGpaint* ret = new NVGpaint(nvgBoxGradient(vg, x, y, w, h, r, f, COLOR(ir, ig, ib, ia), COLOR(or , og, ob, oa)));
	return ret;
}

EXPORT NVGpaint* nvg_RadialGradient(float cx, float cy, float inr, float outr,
	float ir, float ig, float ib, float ia,
	float or, float og, float ob, float oa) {

	NVGpaint* ret = new NVGpaint(nvgRadialGradient(vg, cx, cy, inr, outr, COLOR(ir, ig, ib, ia), COLOR(or , og, ob, oa)));
	return ret;
}

EXPORT NVGpaint* nvg_ImagePattern(float ox, float oy, float ex, float ey,
	float angle, int image, float alpha) {

	NVGpaint* ret = new NVGpaint(nvgImagePattern(vg, ox, oy, ex, ey, angle, image, alpha));
	return ret;
}

EXPORT void nvg_FreePaint(NVGpaint* paint) {
	delete paint;
}


// Scissoring

EXPORT void nvg_Scissor(float x, float y, float w, float h) {
	nvgScissor(vg, x, y, w, h);
}

EXPORT void nvg_IntersectScissor(float x, float y, float w, float h) {
	nvgIntersectScissor(vg, x, y, w, h);
}

EXPORT void nvg_ResetScissor() {
	nvgResetScissor(vg);
}


// Paths

EXPORT void nvg_BeginPath() {
	nvgBeginPath(vg);
}

EXPORT void nvg_MoveTo(float x, float y) {
	nvgMoveTo(vg, x, y);
}

EXPORT void nvg_LineTo(float x, float y) {
	nvgLineTo(vg, x, y);
}

EXPORT void nvg_BezierTo(float c1x, float c1y, float c2x, float c2y, float x, float y) {
	nvgBezierTo(vg, c1x, c1y, c2x, c2y, x, y);
}

EXPORT void nvg_QuadTo(float cx, float cy, float x, float y) {
	nvgQuadTo(vg, cx, cy, x, y);
}

EXPORT void nvg_ArcTo(float x1, float y1, float x2, float y2, float radius) {
	nvgArcTo(vg, x1, y1, x2, y2, radius);
}

EXPORT void nvg_ClosePath() {
	nvgClosePath(vg);
}

EXPORT void nvg_PathWinding(int dir) {
	nvgPathWinding(vg, dir);
}

EXPORT void nvg_Arc(float cx, float cy, float r, float a0, float a1, int dir) {
	nvgArc(vg, cx, cy, r, a0, a1, dir);
}

EXPORT void nvg_Rect(float x, float y, float w, float h) {
	nvgRect(vg, x, y, w, h);
}

EXPORT void nvg_RoundedRect(float x, float y, float w, float h, float r) {
	nvgRoundedRect(vg, x, y, w, h, r);
}

EXPORT void nvg_RoundedRectVarying(float x, float y, float w, float h,
	float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft) {
	
	nvgRoundedRectVarying(vg, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
}

EXPORT void nvg_Ellipse(float cx, float cy, float rx, float ry) {
	nvgEllipse(vg, cx, cy, rx, ry);
}

EXPORT void nvg_Circle(float cx, float cy, float r) {
	nvgCircle(vg, cx, cy, r);
}

EXPORT void nvg_Fill() {
	nvgFill(vg);
}

EXPORT void nvg_Stroke() {
	nvgStroke(vg);
}


// Text

EXPORT int nvg_CreateFont(const char* name, const char* filename) {
	return nvgCreateFont(vg, name, filename);
}

EXPORT int nvg_CreateFontMem(const char* name, unsigned char* data, int ndata, int freeData) {
	return nvgCreateFontMem(vg, name, data, ndata, freeData);
}

EXPORT int nvg_FindFont(const char* name) {
	return nvgFindFont(vg, name);
}

EXPORT int nvg_AddFallbackFontId(int baseFont, int fallbackFont) {
	return nvgAddFallbackFontId(vg, baseFont, fallbackFont);
}

EXPORT int nvg_AddFallbackFont(const char* baseFont, const char* fallbackFont) {
	return nvgAddFallbackFont(vg, baseFont, fallbackFont);
}

EXPORT void nvg_FontSize(float size) {
	nvgFontSize(vg, size);
}

EXPORT void nvg_FontBlur(float blur) {
	nvgFontBlur(vg, blur);
}

EXPORT void nvg_TextLetterSpacing(float spacing) {
	nvgTextLetterSpacing(vg, spacing);
}

EXPORT void nvg_TextLineHeight(float lineHeight) {
	nvgTextLineHeight(vg, lineHeight);
}

EXPORT void nvg_TextAlign(int align) {
	nvgTextAlign(vg, align);
}

EXPORT void nvg_FontFaceId(int font) {
	nvgFontFaceId(vg, font);
}

EXPORT void nvg_FontFace(const char* font) {
	nvgFontFace(vg, font);
}

EXPORT void nvg_Text(float x, float y, const char* str, const char* end) {
	nvgText(vg, x, y, str, end);
}

EXPORT void nvg_TextBox(float x, float y, float breakRowWidth, const char* str, const char* end) {
	nvgTextBox(vg, x, y, breakRowWidth, str, end);
}

EXPORT float nvg_TextBounds(float x, float y, const char* str, const char* end, float* bounds) {
	return nvgTextBounds(vg, x, y, str, end, bounds);
}

EXPORT void nvg_TextBoxBounds(float x, float y, float breakRowWidth, const char* str, const char* end, float* bounds) {
	nvgTextBoxBounds(vg, x, y, breakRowWidth, str, end, bounds);
}

EXPORT void nvg_TextMetrics(float* ascender, float* descender, float* lineh) {
	nvgTextMetrics(vg, ascender, descender, lineh);
}

// Note: nvgTextGlyphPositions and nvgTextBreakLines are not supported due to relatively
// complicated return value processing.


// SVG support

EXPORT NSVGimage* nsvg_ParseFromFile(const char* filename, const char* units, float dpi) {
	return nsvgParseFromFile(filename, units, dpi);
}

EXPORT NSVGimage* nsvg_ParseMem(unsigned char* data, int ndata, const char* units, float dpi) {
	char* buf = static_cast<char*>(malloc(ndata + 1));
	memcpy(buf, data, ndata);
	buf[ndata] = '\0';
	NSVGimage* ret = nsvgParse(buf, units, dpi);
	free(buf);
	return ret;
}

NVGpaint createLinearGradient(NSVGgradient* gradient, float alpha) {
	float inverse[6];
	float sx, sy, ex, ey;

	nvgTransformInverse(inverse, gradient->xform);
	nvgTransformPoint(&sx, &sy, inverse, 0, 0);
	nvgTransformPoint(&ex, &ey, inverse, 0, 1);

	return nvgLinearGradient(vg, sx, sy, ex, ey,
		SVGCOLOR(gradient->stops[0].color, alpha),
		SVGCOLOR(gradient->stops[gradient->nstops - 1].color, alpha));
}

NVGpaint createRadialGradient(NSVGgradient* gradient, float alpha) {
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

	NVGpaint paint = nvgRadialGradient(vg, cx, cy, inr, outr,
		SVGCOLOR(gradient->stops[0].color, alpha),
		SVGCOLOR(gradient->stops[gradient->nstops - 1].color, alpha));
	
	return  paint;
}

EXPORT void nsvg_ImageSize(NSVGimage* image, float &width, float &height) {
	width = image->width;
	height = image->height;
}

EXPORT void nsvg_Draw(NSVGimage* image, char* shapeIdPrefix, float alpha) {
	int cap;

	// iterate shapes
	for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {

		// skip invisible shape
		if (!(shape->flags & NSVG_FLAGS_VISIBLE)) continue;

		// skip shape with prefix not started with shapeIdPrefix, if specified
		if (shapeIdPrefix != NULL &&
			strncmp(shapeIdPrefix, shape->id, strlen(shapeIdPrefix))) continue;

		nvgBeginPath(vg);
		bool pathHole = false;

		// draw paths
		for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {

			nvgMoveTo(vg, path->pts[0], path->pts[1]);

			for (int i = 0; i < path->npts - 1; i += 3) {
				float* p = &path->pts[i * 2];
				nvgBezierTo(vg, p[2], p[3], p[4], p[5], p[6], p[7]);
			}

			if (pathHole)
				nvgPathWinding(vg, NVG_HOLE);
			else
				pathHole = true;

			if (path->closed)
				nvgLineTo(vg, path->pts[0], path->pts[1]);
		}

		// set fill
		switch (shape->fill.type) {
		case NSVG_PAINT_COLOR:
			nvgFillColor(vg, SVGCOLOR(shape->fill.color, shape->opacity * alpha));
			nvgFill(vg);
			break;
		case NSVG_PAINT_LINEAR_GRADIENT:
			nvgFillPaint(vg, createLinearGradient(shape->fill.gradient, alpha));
			nvgFill(vg);
			break;
		case NSVG_PAINT_RADIAL_GRADIENT:
			nvgFillPaint(vg, createRadialGradient(shape->fill.gradient, alpha));
			nvgFill(vg);
			break;
		}

		// set stroke/line
		nvgStrokeWidth(vg, shape->strokeWidth);
		switch (shape->stroke.type) {
			case NSVG_PAINT_COLOR:
				nvgStrokeColor(vg, SVGCOLOR(shape->stroke.color, shape->opacity * alpha));
				nvgStroke(vg);
				break;
			case NSVG_PAINT_LINEAR_GRADIENT:
				nvgStrokePaint(vg, createLinearGradient(shape->stroke.gradient, alpha));
				nvgStroke(vg);
				break;
			case NSVG_PAINT_RADIAL_GRADIENT:
				nvgStrokePaint(vg, createRadialGradient(shape->stroke.gradient, alpha));
				nvgStroke(vg);
				break;
		}

		switch (shape->strokeLineCap) {
			case NSVG_JOIN_MITER: cap = NVG_MITER; break;
			case NSVG_JOIN_ROUND: cap = NVG_ROUND; break;
			case NSVG_JOIN_BEVEL: cap = NVG_BEVEL; break;
			default: cap = NVG_MITER;
		}
		nvgLineCap(vg, cap);
		nvgLineJoin(vg, shape->strokeLineJoin);
	}
}

EXPORT void nsvg_Rasterize(NSVGimage* image, float tx, float ty, float scale, unsigned char* dst, int w, int h) {
	if(rasterizer == NULL)
		rasterizer = nsvgCreateRasterizer();

	nsvgRasterize(rasterizer, image, tx, ty, scale, dst, w, h, w*4);
}

EXPORT void nsvg_Delete(NSVGimage* image) {
	nsvgDelete(image);
}