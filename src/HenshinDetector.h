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

#ifndef _HENSHIN_DETECTOR_H_
#define _HENSHIN_DETECTOR_H_

#include "common.h"
#include "AbstractPoseDetector.h"
#include "IUserListener.h"
#include "UserDetector.h"

class HenshinDetector : public AbstractPoseDetector, IUserListener
{
public:
	enum Stage {
		STAGE_HUMAN,
		STAGE_CALIBRATION,
		STAGE_HENSHINED
	};

private:
	Stage m_stage;
	float m_calibrationProgress;
	float m_henshinProgress;

public:
	HenshinDetector(UserDetector* userDetector);
	virtual ~HenshinDetector();

	UserDetector* getUserDetector() { return m_userDetector; }
	HenshinDetector::Stage getStage() { return m_stage; }
	float getCalibrationProgress() { return m_calibrationProgress; }
	float getHenshinProgress() { return m_henshinProgress; }
	void reset();

	virtual void onDetectPre(float dt);

	//
	// user listener methods
	//

	virtual void onNewUser(XuUserID userID);
	virtual void onLostUser(XuUserID userID);

private:
	void transitToHuman();
	void transitToCalibration();
	void transitToHenshined();
};

#endif
