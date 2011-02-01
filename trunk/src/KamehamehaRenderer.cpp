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
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//@COPYRIGHT@//

#include "KamehamehaRenderer.h"
#include "TrigonometricTable.h"
#include "util.h"
#include "config.h"

static const char *srcVP =
	"uniform mat4 mvMatrix;"
	"uniform mat4 pMatrix;"
	"varying vec4 vFragColor;"
	"attribute vec4 vVertex;"
	"attribute vec3 vNormal;"
	"uniform vec4 vColor;"
	"void main(void) { "
	" mat3 mNormalMatrix;"
	" mNormalMatrix[0] = mvMatrix[0].xyz;"
	" mNormalMatrix[1] = mvMatrix[1].xyz;"
	" mNormalMatrix[2] = mvMatrix[2].xyz;"
	" vec4 mvvVertex = mvMatrix * vVertex;"
	" vec3 vLightDir = -normalize(mvvVertex.xyz / mvvVertex.w);"
	//" vec3 vLightDir = vec3(0.0, 0.0, 1.0); "
	" vec3 vNorm = normalize(mNormalMatrix * vNormal);"
	" float fDot = dot(vNorm, vLightDir); "
	" vFragColor.rgb = vColor.rgb;"
	" vFragColor.a = vColor.a * fDot * fDot;"
	" gl_Position = pMatrix * mvMatrix * vVertex;"
	"}";

static const char *srcFP =	
	"varying vec4 vFragColor; "
	"void main(void) { "
	" gl_FragColor = vFragColor; "
	"}";

static GLuint s_shaderID;

KamehamehaRenderer::KamehamehaRenderer(RenderingContext* rctx, KamehamehaStatus* kkhStatus) : AbstractOpenGLRenderer(rctx)
{
	m_kkhStatus = kkhStatus;
	if (!s_shaderID) {
		s_shaderID = m_rctx->shaderMan->LoadShaderPairSrcWithAttributes("test", srcVP, srcFP, 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");
	}
	m_active = false;
}

KamehamehaRenderer::~KamehamehaRenderer()
{
}

const float LIFE_TIME_MAX = 6.0f;
const float LIFE_TIME_MIN = 3.0f;
const float RADIUS_BASE = 250.0f;

float KamehamehaRenderer::getCurrentIntensity()
{
	if (m_active) {
		float t = m_ticker.tick();
		return ((t < 0.5f) ? square(t) * (m_lifeTime - 0.5f) : (m_lifeTime - t) ) * square(m_intensity);
	} else {
		return 0;
	}
}

float KamehamehaRenderer::getCurrentRadius()
{
	return getCurrentIntensity() * RADIUS_BASE;
}

float KamehamehaRenderer::getCurrentLength()
{
	if (m_active) {
		float t = m_ticker.tick();
		if (t < m_lifeTime - 1.0f) {
			return getCurrentRadius() * (t * t / square(m_intensity)) * 2.0f;
		} else {
			return RADIUS_BASE * square(m_lifeTime - 1.0f) * 2.0f;
		}
	} else {
		return 0;
	}
}

static const TrigonometricTable TRIGON(128);

inline float fluct(cv::RNG& rng) {
	return float(rng.gaussian(0.5)) * 0.1f;
}

void KamehamehaRenderer::drawBeam(float radius, float length)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	for (int i = 0; i < TRIGON.slices() / 2; i++) {
		float z0(TRIGON.sin(i)), z1(TRIGON.sin(i+1));
		float r0(TRIGON.cos(i)), r1(TRIGON.cos(i+1));
		float zz0(z0*length), zz1(z1*length);
		float rr0(r0*radius), rr1(r1*radius);

		glBegin(GL_QUAD_STRIP);
		//glBegin(GL_LINE_STRIP); // for testing
		for (int j = 0; j <= TRIGON.slices(); j += 4) {
			float c(TRIGON.cos(j)), s(TRIGON.sin(j));
			glNormal3f(c*r0 + fluct(m_rng), s*r0 + fluct(m_rng), z0*radius/length + fluct(m_rng));
            glVertex3d(c*rr0, s*rr0, zz0);
			glNormal3f(c*r1 + fluct(m_rng), s*r1 + fluct(m_rng), z1*radius/length + fluct(m_rng));
            glVertex3d(c*rr1, s*rr1, zz1);
		}
		glEnd();
	}
}


void KamehamehaRenderer::draw()
{
	m_kkhStatus->currentIntensity = getCurrentIntensity();

	if (!m_active) {
		return;
	}

	float t = m_ticker.tick();

	m_rctx->modelViewMatrix.PushMatrix();
	{
		float length = getCurrentLength();
		float radius = getCurrentRadius();

		GLFrame f;
		f.SetOrigin(XV3toM3D(m_origin + m_forward * length));
		f.SetForwardVector(XV3toM3D(-m_forward));
		m_rctx->modelViewMatrix.MultMatrix(f);

		float planeColor[] = { 0.95f, 1.0f, 1.0f, 1.5f };

		glUseProgram(s_shaderID);
		glUniformMatrix4fv(glGetUniformLocation(s_shaderID, "mvMatrix"), 1, GL_FALSE, m_rctx->transform.GetModelViewMatrix());
		glUniformMatrix4fv(glGetUniformLocation(s_shaderID, "pMatrix"), 1, GL_FALSE, m_rctx->transform.GetProjectionMatrix());
		glUniform4fv(glGetUniformLocation(s_shaderID, "vColor"), 1, planeColor);

		drawBeam(radius, length);
		//glutSolidSphere(radius, 64, 64);
	}
	m_rctx->modelViewMatrix.PopMatrix();

	if (t > m_lifeTime) {
		m_active = false;
	}

	drawIntensityText();
}

void KamehamehaRenderer::shoot(const XV3& origin, const XV3& forward, float intensity)
{
	if (m_active) {
		// ignore
		return;
	}

	m_active = true;
	m_ticker.lock();

	m_lifeTime = cramp(LIFE_TIME_MIN - 1.0f + square(intensity) * 2.0f, LIFE_TIME_MIN, LIFE_TIME_MAX);
	m_origin = origin;
	m_forward = forward;
	m_intensity = intensity;
}

void KamehamehaRenderer::adjust(const XV3& origin, const XV3& forward)
{
	m_origin = origin;
	m_forward = forward;
}

void KamehamehaRenderer::drawIntensityText()
{
	char buf[64];
	sprintf(buf, "%d%%", int(m_intensity * 100.0f));

	XV3 p(-0.95f, -0.95f, 0.0f), s(0.001f, 0.002f, 1.0f);
	float color[4] = { 1.0f, 0.0f, 0.0f, getCurrentIntensity() };
	float thickness = 5.0f;

	// TODO: clean up and move to util
	glUseProgram(0);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(p.X, p.Y, p.Z);
	glScalef(s.X, s.Y, s.Z);
	glLineWidth(getPointSize() * thickness);
	glColor4fv(color);
	for (const char* p = buf; *p; p++) {
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	}
	glPopMatrix();
}
