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

#include "HenshinDetector.h"
#include "util.h"

HenshinDetector::HenshinDetector(UserDetector* userDetector) : AbstractPoseDetector(userDetector)
{
	m_userDetector->addListener(this);
	transitToHuman();
}

HenshinDetector::~HenshinDetector()
{
}

void HenshinDetector::transitToHuman()
{
	printf("Henshin Stage = HUMAN\n");
	m_stage = STAGE_HUMAN;
	m_henshinProgress = 0;
	m_calibrationProgress = 0;
	m_userDetector->stopTracking();
}

void HenshinDetector::transitToCalibration()
{
	printf("Henshin Stage = CALIBRATION\n");
	m_stage = STAGE_CALIBRATION;
	m_henshinProgress = 0;
	m_calibrationProgress = 0;
}

void HenshinDetector::transitToHenshined()
{
	printf("Henshin Stage = HENSHINED\n");
	m_stage = STAGE_HENSHINED;
	m_calibrationProgress = 0;
	m_henshinProgress = 0;
}

void HenshinDetector::onDetectPre(float dt)
{
	if (m_stage == STAGE_HENSHINED) {
		if (m_henshinProgress < 1.0f) { 
			m_henshinProgress = std::min(m_henshinProgress + 0.5f * dt, 1.0f);
		}
	} else if (m_stage == STAGE_CALIBRATION) {
		m_calibrationProgress += dt;
	}
}

//
// listener methods
//

void HenshinDetector::onNewUser(XnUserID userID)
{
}

void HenshinDetector::onLostUser(XnUserID userID)
{
	if (m_userDetector->isTrackedUserID(userID)) {
		transitToHuman();
	}
}

void HenshinDetector::onCalibrationStart(XnUserID userID)
{
	transitToCalibration();
}

void HenshinDetector::onCalibrationEnd(XnUserID userID, bool isSuccess)
{
	if (isSuccess) {
		transitToHenshined();
	} else {
		transitToHuman();
	}
}

void HenshinDetector::onPoseStart(XnUserID userID, const XnChar* pose)
{
}

void HenshinDetector::onPoseEnd(XnUserID userID, const XnChar* pose)
{
}
