#include "TimedFader.h"

#include <stdio.h>
#include <float.h>
#include <math.h>

#include "FaderDisplay.h"

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	return new TimedFader(audioMaster);
}

TimedFader::TimedFader(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, 1, 3)
{
	//inits here!
	fOutput = 1.0f;
	faderSteps = 15;
	currentFaderFrameCount = 1;

	setNumInputs(2);
	setNumOutputs(2);
	setUniqueID('tFd1');    // identify here
	DECLARE_VST_DEPRECATED(canMono) ();
	canProcessReplacing();
	strcpy(programName, "Timed Fader 1");

	setParameter(PARAM_PlayDuration, 30.0f); // 10 seconds  //0.166667); //~30 seconds
	setParameter(PARAM_FadeDuration, 3.0f);  // 3 seconds

	fPlaySecondsRemaining = fPlayDuration;
	fFadeSecondsRemaining = fFadeDuration;
	state = STATE_DONE;

	muteGain = (float)pow(10.0f, 0.05f * -60.0f);
	muteGain *= 0.01f;

	fSecondsPerFrame = 1 / sampleRate;
	
	// Create the editor
	faderDisplay = new FaderDisplay(this);
	editor = faderDisplay;

	suspend();		// flush buffer
}

void TimedFader::setParameter(VstInt32 index, float value)
{
	// Parameters from the GUI are entered as floats in the range [0,1.0], so we have to convert to a displayable format
	// so when we are displaying the value, we have to re-convert back to the range [0,1.0]
	switch (index)
	{
	case PARAM_PlayDuration: 
		fPlayDurationParameter = value; 
		fPlayDuration = fPlayDurationParameter;
		if (state != STATE_PLAYING)
		{
			fPlaySecondsRemaining = fPlayDuration;
		}
		break;
	case PARAM_FadeDuration: 
		fFadeDurationParameter = value;
		fFadeDuration = fFadeDurationParameter;

		// length of my fade * sample rate = number of samples I'll fade over
		fFadeDurationInFrames = fFadeDuration*sampleRate;
		if (state != STATE_FADING)
		{
			fFadeSecondsRemaining = fFadeDuration;
		}
		break;
	case PARAM_Output: fOutput = value; break;
	}

	resume();
}

void TimedFader::setParameterAutomated(VstInt32 index, float value)
{
	setParameter(index, value);
}

// Gives a nice logarithmic curve [0,1.0] based on the total number of frames in the fade duration and the current frame we're on
float TimedFader::GetNextFaderStep()
{
	// Logarithmic work
	// put x in the range [0,1.0]
	float x = currentFaderFrameCount / fFadeDurationInFrames; 
	float step = 0.0f;

	/* inverse log (1/log(x)) 	
	// put x in the range [0,0.9999]  (don't go to 1 to avoid a /0 error)
	x = (x <= 0) ? 0.0001f : (x >= 1) ? 0.9f : x;
	float maxLog = fabs(1/log(0.9f));
	step = ((1 / log(x)) + maxLog)/maxLog;
	*/

	/* 2^x 
	// range of [0,7.2]
	x = 7.2f*x;
	step = 1 - pow(2, x);
	// step is now in the range of [0,-150], convert this to [0,1] and invert [1,0]
	step = step / -150;
	step = 1 - step; */

	/* Linear */
	step = 1 - x;

	// Just some sanity checks
	// never amplify
	if (step > 1.0)
	{
		step = 1.0;
	}
	// never go below -60dB
	if (step < muteGain)
	{
		step = muteGain;
	}

	return step;
}

TimedFader::~TimedFader()
{
}

bool TimedFader::getProductString(char* text) { strcpy(text, "Timed Fader"); return true; }
bool TimedFader::getVendorString(char* text) { strcpy(text, "Soundstream"); return true; }
bool TimedFader::getEffectName(char* text) { strcpy(text, "Timed fade"); return true; }

void TimedFader::suspend()
{
	fFadeSecondsRemaining = fFadeDuration;
	fPlaySecondsRemaining = fPlayDuration;
	state = STATE_IDLE;
	currentFaderFrameCount = 1;
	faderSteps = 15;
}

void TimedFader::open()
{
	state = STATE_IDLE;
}

void TimedFader::resume()
{
	state = STATE_IDLE;
}

void TimedFader::setProgramName(char* name)
{
	strcpy(programName, name);
}

void TimedFader::getProgramName(char* name)
{
	strcpy(name, programName);
}

bool TimedFader::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* name)
{
	if (index == 0)
	{
		strcpy(name, programName);
		return true;
	}
	return false;
}

float TimedFader::getParameter(VstInt32 index)
{
	float value = 0;

	switch (index)
	{
	case PARAM_PlayDuration:
		value = fPlayDurationParameter;
		break;
	case PARAM_FadeDuration:
		value = fFadeDurationParameter;
		break;
	case PARAM_Output:
		value = fOutput;
		break;
	case PARAM_PlaySecondsRemaining:
		if (state == STATE_PLAYING)
		{
			value = fPlaySecondsRemaining;
		}
		else
		{
			value = fPlayDuration;
		}
		break;
	case PARAM_FadeSecondsRemaining:
		if (state == STATE_FADING)
		{
			value = fFadeSecondsRemaining;
		}
		else
		{
			value = fFadeDuration;
		}
	}

	return value;
}

void TimedFader::getParameterName(VstInt32 index, char *label)
{
	switch (index)
	{
	case PARAM_PlayDuration:
		strcpy(label, "Play duration (Seconds)");
		break;
	case PARAM_FadeDuration:
		strcpy(label, "Fade duration (Seconds)");
		break;
	case PARAM_Output:
		strcpy(label, "Output");
		break;
	}
}

void TimedFader::getParameterDisplay(VstInt32 index, char* text)
{
	// Parameters from the GUI are entered as floats in the range [0,1.0], so we have to convert to a displayable format
	// so when we are displaying the value, we have to re-convert back to the range [0,1.0]
	switch (index)
	{
	case PARAM_PlayDuration:
		sprintf(text, "%.0f", fPlayDurationParameter*MAX_PLAY_DURATION);
		break;
	case PARAM_FadeDuration:
		sprintf(text, "%.0f", fFadeDurationParameter*MAX_FADE_DURATION);
		break;
	case PARAM_Output:
		sprintf(text, "%.0f", fOutput);
		break;
	}
}

void TimedFader::getParameterLabel(VstInt32 index, char* label)
{
	switch (index)
	{
	case PARAM_PlayDuration:
		strcpy(label, "sec");
		break;
	case PARAM_FadeDuration:
		strcpy(label, "sec");
		break;
	case PARAM_Output:
		strcpy(label, "dB");
		break;
	}
}

/// ------------------------------
/// PROCESS
/// ------------------------------
void TimedFader::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	float* in1 = inputs[0];
	float* in2 = inputs[1];
	float* out1 = outputs[0];
	float* out2 = outputs[1];

	float max1 = 0;
	float max2 = 0;

	VstTimeInfo* timeInfo = NULL;
	bool audioIsPlaying = true;

	--in1;
	--in2;
	--out1;
	--out2;

	VstInt32 maxCounter = sampleFrames;
	while (--maxCounter >= 0)
	{
		if (fabs(inputs[0][maxCounter]) > max1)
		{
			max1 = fabs(inputs[0][maxCounter]);
		}
		if (fabs(inputs[1][maxCounter]) > max2)
		{
			max2 = fabs(inputs[1][maxCounter]);
		}
	}

	if (max1 < 0.000001 && max2 < 0.000001)
	{
		audioIsPlaying = false;
	}

	// In any state, if no audio is playing, go to Idle
	if (!audioIsPlaying)
	{
		state = STATE_IDLE;
	}

	switch (state)
	{
	case STATE_PLAYING:	
		// Do nothing else, just let audio play
		while (fPlaySecondsRemaining > 0.0 && --sampleFrames >= 0)
		{
			fPlaySecondsRemaining -= fSecondsPerFrame;
			// let audio pass through
			*++out1 = *++in1 * 1.0;
			*++out2 = *++in2 * 1.0;
		}

		if (fPlaySecondsRemaining <= 0.0)
		{
			state = STATE_FADING;
		}
		faderDisplay->setParameter(PARAM_PlaySecondsRemaining, fPlaySecondsRemaining);
		break;
	case STATE_FADING:
		while (fFadeSecondsRemaining > 0.0 && --sampleFrames >= 0)
		{
			currentFaderFrameCount++;
			fFadeSecondsRemaining -= fSecondsPerFrame;
			// Fade based on current fade tap
			currentFaderMultiplier = GetNextFaderStep();
			*++out1 = *++in1 * currentFaderMultiplier;
			*++out2 = *++in2 * currentFaderMultiplier;
		}

		if (fFadeSecondsRemaining <= 0.0)
		{
			// silence out the rest of the sample
			while (--sampleFrames >= 0)
			{
				*++out1 = *++in1 * muteGain;
				*++out2 = *++in2 * muteGain;
			}
			faderDisplay->setParameter(PARAM_FaderLight, 1);
			state = STATE_DONE;
		}
		faderDisplay->setParameter(PARAM_FadeSecondsRemaining, fFadeSecondsRemaining);
		break;
	case STATE_DONE:
		fPlaySecondsRemaining = fPlayDuration;
		fFadeSecondsRemaining = fFadeDuration;
		// make sure the audio stays muted
		// and wait here until all audio stops playing, then move back to Idle
		while (--sampleFrames >= 0)
		{
			*++out1 = *++in1 * muteGain;
			*++out2 = *++in2 * muteGain;
		}
		break;
	case STATE_IDLE:
		// Set these here again in case someone changed the durations while we are in the IDLE state
		currentFaderFrameCount = 1;
		fPlaySecondsRemaining = fPlayDuration;
		fFadeSecondsRemaining = fFadeDuration;
		faderDisplay->setParameter(PARAM_PlaySecondsRemaining, fPlayDuration);
		faderDisplay->setParameter(PARAM_FadeSecondsRemaining, fFadeDuration);
		faderDisplay->setParameter(PARAM_FaderLight, 0);
		/* Wish this worked...
		//timeInfo = getTimeInfo(kVstTransportPlaying | kVstTransportChanged);
		if (timeInfo != NULL && (timeInfo->flags & kVstTransportPlaying) )
		{
		// start playing
		state = STATE_PLAYING;
		} */
		// Instead, messy hack:
		if (fabs(inputs[0][0]) > 0.0000001 && fabs(inputs[1][0]) > 0.0000001)
		{
			// Audio is coming through, start the timer...
			state = STATE_PLAYING;
		}
		break;
	}
}

