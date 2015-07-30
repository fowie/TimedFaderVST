//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Version 2.4		$Date: 2006/11/13 09:08:28 $
//
// Category     : VST 2.x SDK Samples
// Filename     : FaderDisplay.cpp
// Created by   : Steinberg Media Technologies
// Description  : Simple Surround Delay plugin with Editor using VSTGUI
//
// � 2006, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#ifndef __faderdisplay__
#include "faderdisplay.h"
#endif

#ifndef __timedfader__
#include "timedfader.h"
#endif

#include <stdio.h>

#define GUI_WIDTH 190
#define GUI_HEIGHT 100
#define kPlayCounterX 50
#define kPlayCounterY 10
#define kPlayCounterWidth 50
#define kPlayCounterHeight 15

#define kFadeCounterX 50
#define kFadeCounterY 40
#define kFadeCounterWidth kPlayCounterWidth
#define kFadeCounterHeight kPlayCounterHeight
#define kFadeLightOneImageHeight 45


//-----------------------------------------------------------------------------
// resource id's
enum {
	// bitmaps
	kBackgroundId = 128,
	kFaderBodyId = 129,
	kFaderHandleId = 130,
	kTimerBackgroundId = 131,
	kLightsBackgroundId = 132,

	// positions
	kFaderX = 10,
	kFaderY = 10,

	kFaderInc = 10,

	kDisplayXWidth = 30,
	kDisplayHeight = 14,
};

//-----------------------------------------------------------------------------
// prototype string convert float -> percent
void percentStringConvert(float value, char* string);
void percentStringConvert(float value, char* string)
{
	sprintf(string, "%d%%", (int)(100 * value + 0.5f));
}

void playSecondsStringConvert(float value, char* string);
void playSecondsStringConvert(float value, char* string)
{
	sprintf(string, "%1.2f", value);
}

void fadeSecondsStringConvert(float value, char* string);
void fadeSecondsStringConvert(float value, char* string)
{
	sprintf(string, "%1.2f", value);
}
//-----------------------------------------------------------------------------
// FaderDisplay class implementation
//-----------------------------------------------------------------------------
FaderDisplay::FaderDisplay(AudioEffect *effect)
: AEffGUIEditor(effect)
{
	playCounterDisplay = 0;
	fadeCounterDisplay = 0;
	playDurationTextBox = 0;
	fadeDurationTextBox = 0;
	fadeDoneLight = 0;

	// load the background bitmap
	// we don't need to load all bitmaps, this could be done when open is called
	hBackground = new CBitmap(kBackgroundId);

	// init the size of the plugin
	rect.left = 0;
	rect.top = 0;
	rect.right = GUI_WIDTH;
	rect.bottom = GUI_HEIGHT;
}

//-----------------------------------------------------------------------------
FaderDisplay::~FaderDisplay()
{
	// free the background bitmap
	if (hBackground)
		hBackground->forget();
	hBackground = 0;
}

//-----------------------------------------------------------------------------
bool FaderDisplay::open(void *ptr)
{
	// !!! always call this !!!
	AEffGUIEditor::open(ptr);

	//--load some bitmaps
	// CBitmap* hFaderBody = new CBitmap(kFaderBodyId);
	// CBitmap* hFaderHandle = new CBitmap(kFaderHandleId);
	// CBitmap* hTimerNumbers = new CBitmap("F12nums");
	CBitmap* hLights = new CBitmap(kLightsBackgroundId);
	//--init background frame-----------------------------------------------
	// We use a local CFrame object so that calls to setParameter won't call into objects which may not exist yet. 
	// If all GUI objects are created we assign our class member to this one. See bottom of this method.
	CRect size(0, 0, GUI_WIDTH, GUI_HEIGHT);
	CFrame* lFrame = new CFrame(size, ptr, this);
	lFrame->setBackground(hBackground);

	VstInt32 myHeight = kPlayCounterHeight;
	size(kFaderX/2, kFaderY + kPlayCounterHeight / 2 - myHeight/2 + kFaderInc,
		kFaderX/2 + kPlayCounterWidth,
		kFaderY + kPlayCounterHeight / 2 + myHeight + kFaderInc);
	playDurationTextBox = new CTextEdit(size, this, PARAM_PlayDuration);
	playDurationTextBox->setFont(kNormalFontBig);
	playDurationTextBox->setFontColor(kWhiteCColor);
	playDurationTextBox->setBackColor(kBlueCColor);
	playDurationTextBox->setFrameColor(kBlueCColor);
	lFrame->addView(playDurationTextBox);

	size.offset(0, kPlayCounterHeight + kFaderInc*2);
	fadeDurationTextBox = new CTextEdit(size, this, PARAM_FadeDuration);
	fadeDurationTextBox->setFont(kNormalFontBig);
	fadeDurationTextBox->setFontColor(kWhiteCColor);
	fadeDurationTextBox->setBackColor(kBlueCColor);
	fadeDurationTextBox->setFrameColor(kBlueCColor);
	lFrame->addView(fadeDurationTextBox);	

	// Button
	size(GUI_WIDTH - hLights->getWidth() - 10, kFaderY + kFaderInc,
		GUI_WIDTH + hLights->getWidth() - 10,
		kFaderY + kFadeLightOneImageHeight + kFaderInc);
	fadeDoneLight = new CMovieBitmap(size, this, 10, 2, kFadeLightOneImageHeight, hLights, CPoint(0,0));
	fadeDoneLight->setValue(1);
	lFrame->addView(fadeDoneLight);
	

	//--init the countdown displays----------------------------------------------
	//Play seconds remaining
	size(kPlayCounterWidth + kFaderInc*2, 
		(kFaderY + kPlayCounterHeight) - (kPlayCounterHeight)+kFaderInc,
		kPlayCounterWidth + kFaderInc*2 + kPlayCounterWidth, 
		(kFaderY + kPlayCounterHeight) + kPlayCounterHeight / 2 + kFaderInc);
	playCounterDisplay = new CParamDisplay(size, 0, kRightText);
	playCounterDisplay->setFont(kNormalFontBig);
	playCounterDisplay->setFontColor(kWhiteCColor);
	playCounterDisplay->setBackColor(kBlackCColor);
	playCounterDisplay->setFrameColor(kBlueCColor);
	playCounterDisplay->setValue(effect->getParameter(PARAM_PlaySecondsRemaining));
	lFrame->addView(playCounterDisplay);

	//Fade seconds remaining
	size.offset(0, kFaderInc*2 + kPlayCounterHeight );
	fadeCounterDisplay = new CParamDisplay(size, 0, kRightText);
	fadeCounterDisplay->setFont(kNormalFontBig);
	fadeCounterDisplay->setFontColor(kWhiteCColor);
	fadeCounterDisplay->setBackColor(kBlackCColor);
	fadeCounterDisplay->setFrameColor(kBlueCColor);
	fadeCounterDisplay->setValue(effect->getParameter(PARAM_FadeSecondsRemaining));
	lFrame->addView(fadeCounterDisplay);

	// Note : in the constructor of a CBitmap, the number of references is set to 1.
	// Then, each time the bitmap is used (for hinstance in a vertical slider), this
	// number is incremented.
	// As a consequence, everything happens as if the constructor by itself was adding
	// a reference. That's why we need til here a call to forget ().
	// You mustn't call delete here directly, because the bitmap is used by some CControls...
	// These "rules" apply to the other VSTGUI objects too.
	// hFaderBody->forget();
	// hFaderHandle->forget();
	// hTimerNumbers->forget();

	frame = lFrame;

	for (int i = 0; i < maxNumParameters; i++)
	{
		setParameter(i, effect->getParameter(i));
	}

	return true;
}

//-----------------------------------------------------------------------------
void FaderDisplay::close()
{
	delete frame;
	frame = 0;
}

//-----------------------------------------------------------------------------
// Called when the host automates one of the effect's parameters.
// The UI should reflect this new state so we set the value of the control to the new value
// VSTGUI will automatically redraw changed controls in the next idle
void FaderDisplay::setParameter(VstInt32 index, float value)
{
	char outputString[1024];
	if (frame && index < maxNumParameters)
	{
		switch (index)
		{
		case PARAM_PlayDuration:
			if (playDurationTextBox)
			{
				//playDurationTextBox->setValue(effect->getParameter(index));
				playSecondsStringConvert(effect->getParameter(index), outputString);
				playDurationTextBox->setText(outputString);
			}
			break;

		case PARAM_FadeDuration:
			if (fadeDurationTextBox)
			{
				//fadeDurationTextBox->setValue(effect->getParameter(index));
				fadeSecondsStringConvert(effect->getParameter(index), outputString);
				fadeDurationTextBox->setText(outputString);
			}
			break;

		case PARAM_PlaySecondsRemaining:
			if (playCounterDisplay)
				playCounterDisplay->setValue(effect->getParameter(index));
			break;
		case PARAM_FadeSecondsRemaining:
			if (fadeCounterDisplay)
				fadeCounterDisplay->setValue(effect->getParameter(index));
			break;
		case PARAM_FaderLight:
			if (fadeDoneLight)
			{
				fadeDoneLight->setValue(value);
				fadeDoneLight->setDirty();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Called whenever the user changes one of the controls in the UI
void FaderDisplay::valueChanged(CControl* control)
{
	long tag = control->getTag();
	float value = 0.0f;
	effect->setParameterAutomated(tag, control->getValue());
	switch (tag)
	{
	case PARAM_PlayDuration:  // update the display and the fader knob
		// convert string to float
		value = (float)atof(playDurationTextBox->getText());
		effect->setParameterAutomated(tag, value);
		playDurationTextBox->setDirty();
	case PARAM_PlaySecondsRemaining:
		playCounterDisplay->setValue(effect->getParameter(PARAM_PlaySecondsRemaining)); // can't use control->get value because this might be  fall through from PARAM_PlayDuration
		playCounterDisplay->setDirty();
		break;
	case PARAM_FadeDuration:  // update the display and the fader knob
		value = (float)atof(fadeDurationTextBox->getText());
		effect->setParameterAutomated(tag, value);
		fadeDurationTextBox->setDirty();
	case PARAM_FadeSecondsRemaining:
		fadeCounterDisplay->setValue(effect->getParameter(PARAM_FadeSecondsRemaining));
		fadeCounterDisplay->setDirty();
		break;
	}
	control->setDirty();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------