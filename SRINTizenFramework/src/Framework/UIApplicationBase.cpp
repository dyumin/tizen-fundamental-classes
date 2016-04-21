/*
 * UIApplicationBase.cpp
 *
 *  Created on: Apr 11, 2016
 *      Author: Gilang M. Hamidy (g.hamidy@samsung.com)
 */

#include "SRIN/Framework/Application.h"

using namespace SRIN::Framework;

/*====================================================================================================================*
 * UIApllicationBase class implementation																			  *
 *====================================================================================================================*/

void win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	UIApplicationBase* app = (UIApplicationBase*) data;
	dlog_print(DLOG_DEBUG, LOG_TAG, "Back button pressed");
	app->BackButtonPressed();
}


SimpleReadOnlyProperty<UIApplicationBase, UIApplicationBase*> UIApplicationBase::CurrentInstance;

LIBAPI SRIN::Framework::UIApplicationBase::UIApplicationBase(CString packageName) :
	ApplicationBase(packageName)
{
	this->rootFrame = this->win = this->conform = NULL;
	this->backButtonCallback = nullptr;
	this->backButtonInstance = nullptr;
	this->haveEventBackPressed = false;
}

LIBAPI void SRIN::Framework::UIApplicationBase::SetIndicatorColor(Color color)
{
	Evas_Object* bg = elm_object_part_content_get(this->conform, "elm.swallow.indicator_bg");

	if(bg == nullptr)
	{
		bg = evas_object_rectangle_add(evas_object_evas_get(this->win));
		elm_object_part_content_set(this->conform, "elm.swallow.indicator_bg", bg);
	}

	evas_object_color_set(bg, color.r, color.g, color.b, color.a);

	elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_OPAQUE);
}

LIBAPI bool UIApplicationBase::ApplicationCreate()
{
	CurrentInstance = this;

	elm_config_accel_preference_set("3d");

	// Window
	// Create and initialize elm_win.
	// elm_win is mandatory to manipulate window.
	this->win = elm_win_util_standard_add(packageName, packageName);

	elm_win_autodel_set(this->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(this->win))
	{
		int rots[1] =
		{ 0 };
		elm_win_wm_rotation_available_rotations_set(this->win, (const int *) (&rots), 1);
	}

	//eext_object_event_callback_add(this->win, EEXT_CALLBACK_BACK, win_delete_request_cb, (void*)this);

	//evas_object_smart_callback_add(this->win, "delete,request", win_delete_request_cb, NULL);
	//eext_object_event_callback_add(this->win, EEXT_CALLBACK_BACK, win_delete_request_cb, this);
	EnableBackButtonCallback(true);
	// Conformant
	// Create and initialize elm_conformant.
	// elm_conformant is mandatory for base gui to have proper size
	// when indicator or virtual keypad is visible.
	this->conform = elm_conformant_add(this->win);
	elm_win_indicator_mode_set(this->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(this->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(this->conform, EVAS_HINT_EXPAND,
	EVAS_HINT_EXPAND);
	elm_win_resize_object_add(this->win, this->conform);
	evas_object_show(this->conform);

	// Naviframe
	this->rootFrame = elm_naviframe_add(this->conform);

	elm_naviframe_prev_btn_auto_pushed_set(this->rootFrame, EINA_TRUE);
	elm_naviframe_content_preserve_on_pop_set(this->rootFrame, EINA_FALSE);
	elm_object_content_set(this->conform, this->rootFrame);
	evas_object_show(this->rootFrame);

	evas_object_show(this->win);

	this->OnApplicationCreated();
	return true;
}

LIBAPI void SRIN::Framework::UIApplicationBase::SetIndicatorVisibility(bool value)
{
	if (value)
	{
		elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
	}
	else
	{
		elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_HIDE);
	}
}

LIBAPI void UIApplicationBase::BackButtonPressed()
{
	bool backResult = backButtonCallback ? (backButtonInstance->*backButtonCallback)() : OnBackButtonPressed();
	if (backResult)
		ui_app_exit();
}

LIBAPI Evas_Object* UIApplicationBase::GetApplicationConformant()
{
	return this->conform;
}

LIBAPI void UIApplicationBase::OnApplicationCreated()
{

}

LIBAPI bool UIApplicationBase::AcquireExclusiveBackButtonPressed(EventClass* instance, BackButtonCallback callback)
{
}

LIBAPI bool UIApplicationBase::ReleaseExclusiveBackButtonPressed(EventClass* instance, BackButtonCallback callback)
{
}

LIBAPI void UIApplicationBase::Attach(ViewBase* view)
{
	Evas_Object* viewComponent = view->Create(this->rootFrame);

	//show to window
	if (viewComponent != NULL)
	{
		auto naviframeContent = dynamic_cast<INaviframeContent*>(view);

		CString naviframeStyle = nullptr;

		if (naviframeContent)
			naviframeStyle = naviframeContent->GetContentStyle();

		auto naviframeItem = elm_naviframe_item_push(this->rootFrame, view->Title->c_str(), NULL, NULL,
			viewComponent, naviframeStyle);

		auto backButton = elm_object_item_part_content_get(naviframeItem, "elm.swallow.prev_btn");
		auto style = elm_object_style_get(backButton);

		// Title button handling

		if (naviframeContent)
		{
			CString buttonPart = "title_left_btn";
			auto left = naviframeContent->GetTitleLeftButton(&buttonPart);
			if (left)
			{
				auto oldObj = elm_object_item_part_content_unset(naviframeItem, buttonPart);
				evas_object_hide(oldObj);
				elm_object_item_part_content_set(naviframeItem, buttonPart, left);
				evas_object_show(left);
			}

			buttonPart = "title_right_btn";
			auto right = naviframeContent->GetTitleRightButton(&buttonPart);
			if (right)
			{
				auto oldObj = elm_object_item_part_content_unset(naviframeItem, buttonPart);
				evas_object_hide(oldObj);
				elm_object_item_part_content_set(naviframeItem, buttonPart, right);
				evas_object_show(right);
			}

			naviframeContent->RaiseAfterNaviframePush(naviframeItem);
		}



		dlog_print(DLOG_DEBUG, LOG_TAG, "Prev Button Style: %s", style);

		evas_object_smart_callback_add(backButton, "clicked", [] (void* a, Evas_Object* b, void* c)
		{	static_cast<UIApplicationBase*>(a)->BackButtonPressed();}, this);

		evas_object_show(viewComponent);
	}
}

LIBAPI void UIApplicationBase::Detach()
{
	elm_naviframe_item_pop(this->rootFrame);
}


LIBAPI bool UIApplicationBase::OnBackButtonPressed()
{
	return true;
}

LIBAPI void UIApplicationBase::EnableBackButtonCallback(bool enable)
{
	if (enable)
	{
		if (!this->haveEventBackPressed)
		{
			eext_object_event_callback_add(this->win, EEXT_CALLBACK_BACK, win_delete_request_cb, this);
			this->haveEventBackPressed = true;
		}
	}
	else
	{
		eext_object_event_callback_del(this->win, EEXT_CALLBACK_BACK, win_delete_request_cb);
		this->haveEventBackPressed = false;
	}
}

LIBAPI void SRIN::Framework::IndicatorStyler::OnPostNavigation(Event<ControllerManager*, ControllerBase*>* event,
	ControllerManager* manager, ControllerBase* controller)
{
	auto colorable = dynamic_cast<IIndicatorColor*>(controller->View);
	if(colorable)
		app->SetIndicatorColor(colorable->IndicatorColor);
	else
		app->SetIndicatorColor(defaultColor);
}

/*====================================================================================================================*
 * IndicatorStyler class implementation																				  *
 *====================================================================================================================*/

LIBAPI SRIN::Framework::IndicatorStyler::IndicatorStyler(UIApplicationBase* app, ControllerManager* manager, Color defaultColor) :
	app(app), manager(manager), defaultColor(defaultColor)
{
	manager->NavigationProcessed += { this, &IndicatorStyler::OnPostNavigation };
}

LIBAPI SRIN::Framework::IndicatorStyler::~IndicatorStyler()
{
	manager->NavigationProcessed -= { this, &IndicatorStyler::OnPostNavigation };
}

LIBAPI SRIN::Framework::INaviframeContent::INaviframeContent() :
	naviframeItem(nullptr)
{

}