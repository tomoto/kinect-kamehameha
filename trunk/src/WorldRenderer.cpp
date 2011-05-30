//@COPYRIGHT@//
//
// Copyright (c) 2011, Tomoto S. Washio
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   * Neither the name of the Tomoto S. Washio nor the names of his
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL TOMOTO S. WASHIO BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//@COPYRIGHT@//

#include "WorldRenderer.h"
#include "Configuration.h"
#include "config.h"
#include <GLFrustum.h>
#include "util.h"
#include "math.h"

const float DEFAULT_DEPTH_ADJUSTMENT = 105; // empirical valuel. adjustable by 'q' and 'a' key.

WorldRenderer::WorldRenderer(RenderingContext* rctx, DepthGenerator* depthGen, ImageGenerator* imageGen,
							 HenshinDetector* henshinDetector, KamehamehaStatus* kkhStatus)
: AbstractOpenGLRenderer(rctx)
{
	m_depthGen = depthGen;
	m_imageGen = imageGen;
	m_henshinDetector = henshinDetector;
	m_kkhStatus = kkhStatus;

	DepthMetaData dmd;
	m_depthGen->GetMetaData(dmd);
	m_width = dmd.XRes();
	m_height = dmd.YRes();

	// allocate working buffers
	XnUInt32 numPoints = getNumPoints();
	m_vertexBuf = new M3DVector3f[numPoints];
	m_colorBuf = new M3DVector4f[numPoints];

	// pre-set values on working buffers
	M3DVector3f* vp = m_vertexBuf;
	M3DVector4f* cp = m_colorBuf;
	for (XnUInt32 iy = 0; iy < m_height; iy++) {
		for (XnUInt32 ix = 0; ix < m_width; ix++) {
			(*vp)[0] = normalizeX(float(ix));
			(*vp)[1] = normalizeY(float(iy));
			(*vp)[2] = 0;
			vp++;
			(*cp)[0] = (*cp)[1] = (*cp)[2] = 0;
			(*cp)[3] = 1; // alpha is always 1.0
			cp++;
		}
	}

	m_batch.init(numPoints);

	m_depthAdjustment = DEFAULT_DEPTH_ADJUSTMENT;
}

WorldRenderer::~WorldRenderer()
{
	delete[] m_vertexBuf;
	delete[] m_colorBuf;
}

void WorldRenderer::getHenshinData(XnUserID* pUserID, const XnLabel** ppLabel, XV3* pLightCenter, XV3* pHeadCenter, XV3* pHeadDirection)
{
	UserDetector* userDetector = m_henshinDetector->getUserDetector();

	SceneMetaData smd;
	userDetector->getUserGenerator()->GetUserPixels(0, smd);
	*ppLabel = smd.Data();

	if (*pUserID = userDetector->getTrackedUserID()) {
		XV3 ps[3];
		ps[0].assign(m_kkhStatus->center);
		ps[1].assign(userDetector->getSkeletonJointPosition(XN_SKEL_HEAD));
		ps[2].assign(userDetector->getSkeletonJointPosition(XN_SKEL_NECK));
		m_depthGen->ConvertRealWorldToProjective(3, ps, ps);
		normalizeProjective(&ps[0]);
		normalizeProjective(&ps[1]);
		normalizeProjective(&ps[2]);
		*pLightCenter = ps[0];
		*pHeadCenter = ps[1];
		*pHeadDirection = ps[1] - ps[2];
	}
}

#ifndef USE_MACRO
inline void setGray(M3DVector4f* cp, float g)
{
	(*cp)[0] = (*cp)[1] = (*cp)[2] = g * 0.7f + 0.2f;
}
#else
#define setGray(cp, g) ((*(cp))[0] = (*(cp))[1] = (*(cp))[2] = (g) * 0.7f + 0.2f)
#endif

#ifndef USE_MACRO
inline void setRed(M3DVector4f* cp, float g)
{
	(*cp)[0] = g * 0.8f + 0.2f;
	(*cp)[1] = (*cp)[2] = 0;
}
#else
#define setRed(cp, g) ((*(cp))[0] = (g) * 0.8f + 0.2f, (*(cp))[1] = (*(cp))[2] = 0)
#endif

#ifndef USE_MACRO
inline void setRGB(M3DVector4f* cp, const XnRGB24Pixel& p)
{
	(*cp)[0] = b2fNormalized(p.nRed);
	(*cp)[1] = b2fNormalized(p.nGreen);
	(*cp)[2] = b2fNormalized(p.nBlue);
}
#else
#define setRGB(cp, p) ((*(cp))[0] = b2fNormalized((p).nRed), (*(cp))[1] = b2fNormalized((p).nGreen), (*(cp))[2] = b2fNormalized((p).nBlue))
#endif

void WorldRenderer::draw()
{
	drawBackground();
}

inline float getHairLength(const XV3& v)
{
	float angle = acos(abs(v.X)/v.magnitude());
	const float PI8 = float(M_PI/8);
	switch (int(angle / PI8)) {
		case 0:
		case 1:
			return angle / PI8;
		case 2:
			return 1.0f + (angle-PI8*2)/PI8;
		case 3:
			return 1.0f + (angle-PI8*3)/PI8 * 1.5f;
	}
	return 0.0f;
}

void WorldRenderer::drawBackground()
{
	m_rctx->orthoMatrix.PushMatrix();
	{
		//
		// TODO: clean up
		//

		float currentIntensity = m_kkhStatus->getCurrentIntensity();

		m_rctx->orthoMatrix.Translate(
			float(m_rng.gaussian(0.6)) * currentIntensity * 0.01f,
			float(m_rng.gaussian(0.6)) * currentIntensity * 0.01f,
			0);

		// setup shader
		m_rctx->shaderMan->UseStockShader(GLT_SHADER_SHADED, m_rctx->orthoMatrix.GetMatrix());

		// get depth buffer
		DepthMetaData dmd;
		m_depthGen->GetMetaData(dmd);
		const XnDepthPixel* dp = dmd.Data();

		// get image buffer
		ImageMetaData imd;
		m_imageGen->GetMetaData(imd);
		const XnRGB24Pixel* ip = imd.RGB24Data();

		// get working buffers
		M3DVector3f* vp = m_vertexBuf;
		M3DVector4f* cp = m_colorBuf;
		XnUInt32 numPoints = getNumPoints();

		// setup henshin-related information
		const float Z_SCALE = 10.0f;
		XnUserID userID = 0;
		const XnLabel* lp = NULL;
		XV3 lightCenter, headCenter, headDirection;
		getHenshinData(&userID, &lp, &lightCenter, &headCenter, &headDirection);
		lightCenter.Z *= Z_SCALE;
		float lightRadius =
			(m_kkhStatus->getGrowth() + currentIntensity * 0.5f) *
			(0.97f + float(m_rng.gaussian(0.6)) * 0.06f);

		float halationFactor = 100.0f * (1.0f + sqrt(currentIntensity) * 2.0f) / square(Z_SCALE);

		float lightCoreRadius = lightRadius * 0.25f + currentIntensity * 0.02f;
		float lightCoreRadius2 = square(lightCoreRadius);

		bool isCalibration = (m_henshinDetector->getStage() == HenshinDetector::STAGE_CALIBRATION);
		float calibrationGlowIntensity = sinf(m_henshinDetector->getCalibrationProgress() * float(M3D_PI)) * 0.5f;

		bool isTracked = userID && lp;
		bool isLightened = isTracked && lightRadius > 0.0f;

		float hairScale = m_henshinDetector->getHenshinProgress() * 0.6f;
		float auraScale = m_henshinDetector->getHenshinProgress();
		XV3 hairDirection = headDirection;
		if (isLightened) {
			hairDirection += XV3(headCenter.X - lightCenter.X, headCenter.Y - lightCenter.Y, 0) * lightRadius * 0.5f;
		}
		GLFrame hairFrame;
		hairFrame.SetForwardVector(0, 0, 1);
		hairFrame.SetUpVector(XV3toM3D(hairDirection));
		hairFrame.Normalize();

		XnUInt32 ix = 0, iy = 0;
		float nearZ = PERSPECTIVE_Z_MIN + m_depthAdjustment;
		for (XnUInt32 i = 0; i < numPoints; i++, dp++, ip++, vp++, cp++, lp++, ix++) {

			if (ix == m_width) {
				ix = 0;
				iy++;
			}

			// (*vp)[0] (x) is already set
			// (*vp)[1] (y) is already set
			(*vp)[2] = (*dp) ? getNormalizedDepth(*dp, nearZ, PERSPECTIVE_Z_MAX) : Z_INFINITE;

			setRGB(cp, *ip);

			if (isTracked) {
				// aura and hair
				if (iy > 0 && *lp  == userID && (*(lp-m_width) != userID || *(lp-1) != userID || *(lp+1) != userID)) {
					M3DVector4f* auraSourcePtr = cp;

					// hair
					// TODO: clean up!
					XV3 p(*vp), flatCoords(*vp);
					flatCoords.Z = headCenter.Z;
					if (headCenter.distance2(flatCoords) < 0.06 && abs(headCenter.Z / p.Z - 1.0) < 0.01f && flatCoords.Y > headCenter.Y) {
						XV3 v(flatCoords - headCenter);
						v *= getHairLength(v) * hairScale;

						M3DVector3f tmp;
						hairFrame.TransformPoint(XV3toM3D(v), tmp);
						v.assign(tmp);

						glDisable(GL_CULL_FACE);
						glBegin(GL_TRIANGLES);
						glVertexAttrib4f(GLT_ATTRIBUTE_COLOR, (*cp)[0], (*cp)[1], (*cp)[2], 0.45f); // root
						glVertex3f(p.X-v.Y*0.07f, p.Y+v.X*0.07f, p.Z);
						glVertex3f(p.X+v.Y*0.07f, p.Y-v.X*0.07f, p.Z);
						glVertexAttrib4f(GLT_ATTRIBUTE_COLOR, 0.9f, 1.0f, 0.4f, 0.3f); // gold
						glVertex3f(p.X+v.X, p.Y+v.Y, p.Z+v.Z);
						glEnd();

						auraSourcePtr += ptrdiff_t(-v.X*Y_RES*0.5f) + ptrdiff_t(v.Y*Y_RES*0.5f) * m_width;
					}

					M3DVector4f* ap = auraSourcePtr;
					for (int i = int(20.0f * auraScale); i > 0; i--, ap -= m_width) {
						if (ap < m_colorBuf) {
							break;
						}

						float a = (i / 20.0f) * 0.25f;
						(*ap)[0] = interpolate((*ap)[0], 0.9f, a);
						(*ap)[1] = interpolate((*ap)[1], 1.0f, a);
						(*ap)[2] = interpolate((*ap)[2], 0.4f, a);
					}

				}
			}
			
			if (isCalibration) {
				// glow for calibration
				if (*lp > 0) { // TODO: check user ID if it is under calibration
					(*cp)[0] = interpolate((*cp)[0], 0.9f, calibrationGlowIntensity);
					(*cp)[1] = interpolate((*cp)[1], 1.0f, calibrationGlowIntensity);
					(*cp)[2] = interpolate((*cp)[2], 0.4f, calibrationGlowIntensity);
				}
			}

			if (isLightened) {
				{
					XV3 v(*vp);
					v.Z *= Z_SCALE;
					float d = lightRadius * (halationFactor / lightCenter.distance2(v) -  1.5f);
					float intensity = ((*lp == userID) ? 0.1f : 0.2f);
					d *= intensity;
					float a = 1.0f + d;

					(*cp)[0] = (*cp)[0] * a + 0.01f * d;
					(*cp)[1] = (*cp)[1] * a + 0.02f * d;
					(*cp)[2] = (*cp)[2] * a + 0.02f * d;
				}
				{
					// TODO: Should we use 3D object?
					XV3 flatCoords(*vp);
					flatCoords.Z = lightCenter.Z;
					float flatDistance2 = lightCenter.distance2(flatCoords);
					if (flatDistance2 < lightCoreRadius2) {
						float r = (1.0f - sqrt(flatDistance2) / lightCoreRadius) * (1.0f + 0.8f * lightRadius);
						float r2 = r * r;
						float a = (r <= 1.0f) ? (2 * r2 - r2 * r2) : 1.0f;
						float intensity = (*lp == userID) ? 0.8f : 1.0f;
						a *= intensity;
						(*cp)[0] = interpolate((*cp)[0], 0.9f, a);
						(*cp)[1] = interpolate((*cp)[1], 1.0f, a);
						(*cp)[2] = interpolate((*cp)[2], 1.0f, a);
					}
				}
			}
		}

		glEnable(GL_POINT_SIZE);
		glPointSize(getPointSize());
		m_batch.draw(m_vertexBuf, m_colorBuf);
	}
	m_rctx->orthoMatrix.PopMatrix();

	drawModeText();
}


void WorldRenderer::drawModeText()
{
	if (Configuration::getInstance()->getPartyMode() == Configuration::PARTY_MODE_ON) {
		const char* text = "Party Mode";

		XV3 p(0.62f, -0.95f, 0.0f), s(0.0005f, 0.001f, 1.0f);
		float color[4] = { 1.0f, 0.0f, 0.0f, 0.7f };
		renderStrokeText("Party Mode", p, s, 2.0f, color);
	}
}

WorldRenderer::Batch::Batch()
{
	m_numPoints = 0;
	m_vertexArrayID = 0;
	m_vertexBufID = 0;
	m_colorBufID = 0;
}

WorldRenderer::Batch::~Batch()
{
	if (m_vertexArrayID) glDeleteVertexArrays(1, &m_vertexArrayID);
	if (m_vertexBufID) glDeleteBuffers(1, &m_vertexBufID);
	if (m_colorBufID) glDeleteBuffers(1, &m_colorBufID);
}

void WorldRenderer::Batch::init(GLuint numPoints)
{
	m_numPoints = numPoints;

	glGenBuffers(1, &m_vertexBufID);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(M3DVector3f) * numPoints, NULL, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &m_colorBufID);
	glBindBuffer(GL_ARRAY_BUFFER, m_colorBufID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(M3DVector4f) * numPoints, NULL, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);
	{
		glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufID);
		glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(GLT_ATTRIBUTE_COLOR);
		glBindBuffer(GL_ARRAY_BUFFER, m_colorBufID);
		glVertexAttribPointer(GLT_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}
	glBindVertexArray(0);
}

void WorldRenderer::Batch::draw(M3DVector3f* vertexBuf, M3DVector4f* colorBuf)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(M3DVector3f) * m_numPoints, vertexBuf);

	glBindBuffer(GL_ARRAY_BUFFER, m_colorBufID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(M3DVector4f) * m_numPoints, colorBuf);

	glBindVertexArray(m_vertexArrayID);
	glDrawArrays(GL_POINTS, 0, m_numPoints);
	glBindVertexArray(0);
}

