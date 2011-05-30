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

#include "KamehamehaDetector.h"
#include "util.h"

KamehamehaDetector::KamehamehaDetector(HenshinDetector* henshinDetector, KamehamehaStatus* status, KamehamehaRenderer* kkhRenderer) :
	AbstractPoseDetector(henshinDetector->getUserDetector())
{
	// setRequiredPosingStability(3);

	m_status = status;
	m_kkhRenderer = kkhRenderer;
}

KamehamehaDetector::~KamehamehaDetector()
{
}

void KamehamehaDetector::transitTo(KamehamehaStatus::Stage stage)
{
	m_status->stage = stage;
}

void KamehamehaDetector::onDetectPost(float dt)
{
	if (m_status->stage == KamehamehaStatus::STAGE_SHOOT && !m_kkhRenderer->isActive()) {
		transitTo(KamehamehaStatus::STAGE_COOLINGDOWN);
	}
}

float KamehamehaDetector::getHandDistanceThreshold()
{
	return 250.0f;
}

float KamehamehaDetector::getArmStraightThreshold() {
	if (getConfiguration()->getPartyMode() == Configuration::PARTY_MODE_OFF) {
		return 0.8f;
	} else {
		return 0.75f; // bonus
	}
}

float KamehamehaDetector::getArmLevelThreshold() {
	if (getConfiguration()->getPartyMode() == Configuration::PARTY_MODE_OFF) {
		return 0.2f;
	} else {
		return 0.25f; // bonus
	}
}

float KamehamehaDetector::getMotionIntensityFactor() {
	if (getConfiguration()->getPartyMode() == Configuration::PARTY_MODE_OFF) {
		return 0.8f;
	} else {
		return 0.6f; // do not care about the motion much
	}
}

inline float getArmVerticalDiff(const XV3& v)
{
	return abs(v.Y / v.magnitude() - 0.1f);
}

bool KamehamehaDetector::isPosing(float dt)
{
	// TODO: more clean up

	// get skeleton
	XnSkeletonJointPosition jrh, jre, jrs, jlh, jle, jls;
	m_userDetector->getSkeletonJointPosition(XN_SKEL_RIGHT_HAND, &jrh);
	m_userDetector->getSkeletonJointPosition(XN_SKEL_RIGHT_ELBOW, &jre);
	m_userDetector->getSkeletonJointPosition(XN_SKEL_RIGHT_SHOULDER, &jrs);
	m_userDetector->getSkeletonJointPosition(XN_SKEL_LEFT_HAND, &jlh);
	m_userDetector->getSkeletonJointPosition(XN_SKEL_LEFT_ELBOW, &jle);
	m_userDetector->getSkeletonJointPosition(XN_SKEL_LEFT_SHOULDER, &jls);

	XV3 prh(jrh.position), pre(jre.position), prs(jrs.position);
	XV3 plh(jlh.position), ple(jle.position), pls(jls.position);

	// check if boths hands are close

	bool areBothHandsClose =
		(isConfident(jrh) || isConfident(jlh)) &&
		prh.distance2(plh) < square(getHandDistanceThreshold());

	if (m_status->stage == KamehamehaStatus::STAGE_READY && areBothHandsClose) {
		updateGeometry(prh.interpolate(plh), prs.interpolate(pls));
		m_status->timeGrowth = std::min(m_status->timeGrowth + 0.2f * dt, 1.0f);
	} else {
		m_status->timeGrowth = std::max(m_status->timeGrowth - 1.0f * dt, 0.0f);
	}
	
	if (m_status->stage == KamehamehaStatus::STAGE_COOLINGDOWN && m_status->timeGrowth == 0.0f) {
		transitTo(KamehamehaStatus::STAGE_READY);
	}

	// check if arms are level and sticked

	XV3 vrse(pre - prs), vreh(prh - pre), vlse(ple - prs), vleh(plh - ple);
	float yrse(getArmVerticalDiff(vrse)), yreh(getArmVerticalDiff(vreh));
	float ylse(getArmVerticalDiff(vlse)), yleh(getArmVerticalDiff(vleh));
	bool isRightArmLevel = yrse < getArmLevelThreshold() && yreh < getArmLevelThreshold() && isConfident(jrh);
	bool isLeftArmLevel =  ylse < getArmLevelThreshold() && yleh < getArmLevelThreshold() && isConfident(jlh);

	bool result;
	if (isRightArmLevel || isLeftArmLevel) {
		XV3 p, v;
		if (isRightArmLevel && !isLeftArmLevel) {
			updateGeometry(prh, prs);
		} else if (isLeftArmLevel && isRightArmLevel) {
			updateGeometry(plh, pls);
		} else {
			updateGeometry(prh.interpolate(plh), prs.interpolate(pls));
		}
		result = true;
	} else {
		result = false;
	}

	updatePoseGrowth(dt);
	updateForwardVectorHistory(dt);
	return result;
}

void KamehamehaDetector::onPoseDetected(float dt)
{
	switch (m_status->stage) {
		case KamehamehaStatus::STAGE_READY:
			{
				float motionIntensity = getMotionIntensity();
				float intensity = m_status->getGrowth() * motionIntensity * getMotionIntensityFactor();
				if (intensity > 0.2f) {
					printf("INTENSITY=%.3f (TIME=%.3f, POSE=%.3f, MOTION=%.3f)\n", intensity, m_status->timeGrowth, m_status->poseGrowth, motionIntensity);
					m_kkhRenderer->shoot(m_status->center, m_status->forward, intensity);
					transitTo(KamehamehaStatus::STAGE_SHOOT);
				}
			}
			break;
		case KamehamehaStatus::STAGE_SHOOT:
			m_kkhRenderer->adjust(m_status->center, m_status->forward);
			break;
	}
}

void KamehamehaDetector::updateGeometry(const XV3& center, const XV3& base)
{
	m_status->center = center;
	m_status->forward = (center - base).normalize();
}

static float calcPoseGrowthFactor(const XV3& h, const XV3& k)
{
	XV3 v(h - k);
	float y = v.Y / v.magnitude();
	return (1.0f - cramp(y, 0.75f, 1.0f)) / 0.25f;
}

void KamehamehaDetector::updatePoseGrowth(float dt)
{
	XnSkeletonJointPosition jrh, jrk, jlh, jlk;
	m_userDetector->getSkeletonJointPosition(XN_SKEL_RIGHT_HIP, &jrh);
	m_userDetector->getSkeletonJointPosition(XN_SKEL_RIGHT_KNEE, &jrk);
	m_userDetector->getSkeletonJointPosition(XN_SKEL_LEFT_HIP, &jlh);
	m_userDetector->getSkeletonJointPosition(XN_SKEL_LEFT_KNEE, &jlk);

	float g = 0;
	int numOfValidPoints = 0;
	if (isConfident(jrh) && isConfident(jrk)) {
		g += calcPoseGrowthFactor(jrh.position, jrk.position) * 0.4f;
		numOfValidPoints++;
	}

	if (isConfident(jlh) && isConfident(jlk)) {
		g += calcPoseGrowthFactor(jlh.position, jlk.position) * 0.4f;
		numOfValidPoints++;
	}

	if (numOfValidPoints == 1) {
		// when one leg is hidden, add some bonus
		g *= 1.6f;
	}

	if (getConfiguration()->getPartyMode() == Configuration::PARTY_MODE_ON) {
		g += 0.8f; // bonus
	}

	m_status->poseGrowth += cramp(0.5f + g - m_status->poseGrowth, -0.25f, 0.25f) * dt;
}

void KamehamehaDetector::updateForwardVectorHistory(float dt)
{
	const float DURATION = 0.5f;
	m_forwardVectorHistory.push_front(TimeVectorEntry(dt, m_status->forward));

	if (m_forwardVectorHistory.size() > 1) {
		float t = 0.0f;
		std::list<TimeVectorEntry>::iterator i = m_forwardVectorHistory.begin();
		while (i != m_forwardVectorHistory.end()) {
			t += i->dt;
			if (t > DURATION) {
				break;
			}
			i++;
		}
		m_forwardVectorHistory.erase(i, m_forwardVectorHistory.end());
	}
}

float KamehamehaDetector::getMotionIntensity()
{
	XV3 oldest(m_forwardVectorHistory.back().v), latest(m_forwardVectorHistory.front().v);
	XV3 front(0, 0, -1);

	// @ja ³–ÊŒü‚«‚ÅŒ‚‚Âê‡‚É‚ÍŒŸ’n‚µ‚É‚­‚¢‚Ì‚Å­‚µ’êã‚°‚µ‚Ä‚ ‚°‚é
	float adjustment = square(front.dot(latest)) * 0.3f;

	if (getConfiguration()->getPartyMode() == Configuration::PARTY_MODE_ON) {
		adjustment += 0.5f; // bonus
	}

	return (1.0f - oldest.dot(latest) * 0.75f) + adjustment;
}
