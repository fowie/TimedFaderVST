#ifndef __FaderDisplay__
#define __FaderDisplay__

#define _CRT_SECURE_NO_WARNINGS 1

#ifndef __vstgui__
#include "vstgui.sf\vstgui\vstgui.h"
#include "public.sdk\source\vst2.x\audioeffectx.h"
#include "vstgui.sf\vstgui\aeffguieditor.h"
#endif

class FaderDisplay : public AEffGUIEditor, public CControlListener
{
public:
	FaderDisplay(AudioEffect* effect);
	virtual ~FaderDisplay();

public:
	virtual bool open(void* ptr);
	virtual void close();

	virtual void valueChanged(CControl* control);
	virtual void setParameter(VstInt32 index, float value);

private:
	// Controls
	/*CHorizontalSlider* playDurationSlider;
	CHorizontalSlider* fadeDurationSlider;

	CParamDisplay* playDurationDisplay;
	CParamDisplay* fadeDurationDisplay;*/
	CParamDisplay* fadeCounterDisplay;
	CParamDisplay* playCounterDisplay;

	CTextEdit* playDurationTextBox;
	CTextEdit* fadeDurationTextBox;

	CMovieBitmap* fadeDoneLight;

	// Bitmap
	CBitmap* hBackground;
};

#endif