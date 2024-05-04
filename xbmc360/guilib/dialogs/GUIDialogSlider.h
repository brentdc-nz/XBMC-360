#ifndef CGUIDIALOGSLIDER_H
#define CGUIDIALOGSLIDER_H

#include "guilib\GUIDialog.h"
#include "guilib\GUISliderControl.h"

class CGUISliderControl;

/*
Interface class for callback from the slider dialog
Used to pass feedback from the slider dialog to a caller.  Users of the
slider dialog should derive from this class if they wish to respond to changes
in the slider by the user as they happen.  OnSliderChange is called in response
to the user moving the slider.  The caller may then update the text on the slider
and update anything that should be changed as the slider is adjusted.
*/
class ISliderCallback
{
public:
	virtual ~ISliderCallback() {}
	// Callback function called whenever the user moves the slider
	// data pointer of callbackData passed into CGUIDialogSlider::ShowAndGetInput()
	// slider pointer to the slider control on the dialog 
	virtual void OnSliderChange(void *data, CGUISliderControl *slider)=0;
};

class CGUIDialogSlider : public CGUIDialog
{
public:
	CGUIDialogSlider();
	virtual ~CGUIDialogSlider(void);
	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnAction(const CAction &action);

/*	Show the slider dialog and wait for the user to change the value
	Shows the slider until the user is happy with the adjusted value. Calls back with each change to the callback function
	allowing changes to take place immediately.
	\param label description of what is being changed by the slider
	\param value start value of the slider
	\param min minimal value the slider may take
	\param delta amount the slider advances for a single click
	\param max maximal value the slider may take
	\param callback callback class that implements ISliderCallback::OnSliderChange
	\param callbackData pointer to callback-specific data (defaults to NULL)
	\sa ISliderCallback, Display
*/
	static void ShowAndGetInput(const CStdString &label, float value, float min, float delta, float max, ISliderCallback *callback, void *callbackData = NULL);

/*	Show the slider dialog as a response to user input
	Shows the slider with the given values for a short period of time, used for UI feedback of a set user action.
	This function is asynchronous.
	\param label id of the description label for the slider
	\param value start value of the slider
	\param min minimal value the slider may take
	\param delta amount the slider advances for a single click
	\param max maximal value the slider may take
	\param callback callback class that implements ISliderCallback::OnSliderChange
	\sa ISliderCallback, ShowAndGetInput
*/
	static void Display(int label, float value, float min, float delta, float max, ISliderCallback *callback);

protected:
	void SetSlider(const CStdString &label, float value, float min, float delta, float max, ISliderCallback *callback, void *callbackData);
	virtual void OnWindowLoaded();

	ISliderCallback *m_callback;
	void *m_callbackData;
};

#endif //CGUIDIALOGSLIDER_H