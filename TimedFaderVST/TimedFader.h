#ifndef __TimedFader_H
#define __TimedFader_H

#define _CRT_SECURE_NO_WARNINGS 1

#include "public.sdk\source\vst2.x\audioeffectx.h"

enum
{
	PARAM_PlayDuration = 0,
	PARAM_FadeDuration,
	PARAM_Output,
	PARAM_PlaySecondsRemaining,
	PARAM_FadeSecondsRemaining,
	PARAM_FaderLight,
	maxNumParameters
};

#define STATE_IDLE 0
#define STATE_PLAYING 1
#define STATE_FADING 2
#define STATE_DONE 3

#define MAX_PLAY_DURATION 180  // 3 minutes
#define MAX_FADE_DURATION 30   // 30 seconds
class FaderDisplay;

class TimedFader : public AudioEffectX
{

public:
	TimedFader(audioMasterCallback audioMaster);
	~TimedFader();

	virtual void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
	virtual void setProgramName(char* name);
	virtual void getProgramName(char* name);
	virtual bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char* name);
	virtual void setParameter(VstInt32 index, float value);
	virtual void setParameterAutomated(VstInt32 index, float value);
	virtual float getParameter(VstInt32 index);
	virtual void getParameterLabel(VstInt32 index, char *label);
	virtual void getParameterDisplay(VstInt32 index, char *text);
	virtual void getParameterName(VstInt32 index, char *text);
	virtual void suspend();
	virtual void resume();
	virtual void open();

	virtual bool getEffectName(char *name);
	virtual bool getVendorString(char *text);
	virtual bool getProductString(char *text);
	virtual VstInt32 getVendorVersion() { return 1000; }

protected:
	float GetNextFaderStep();

	float fPlayDurationParameter;
	float fFadeDurationParameter;
	float fPlayDuration;
	float fFadeDuration;
	float fPlaySecondsRemaining;
	float fFadeSecondsRemaining;
	float muteGain;

	char programName[32];

	VstInt32 currentFaderFrameCount; // current step (out of the total # of frames)
	float currentFaderMultiplier; // amount to fade in each step
	VstInt32 faderSteps; // number of steps
	float fOutput;

	VstInt32 state; // 0 = none, 1 = playing, 2 = fading

	float fSecondsPerFrame;
	float fFadeDurationInFrames;

	FaderDisplay *faderDisplay;
};

#endif