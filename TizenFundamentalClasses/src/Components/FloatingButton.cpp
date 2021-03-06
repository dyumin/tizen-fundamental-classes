/*
 * Tizen Fundamental Classes - TFC
 * Copyright (c) 2016-2017 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *    Components/FloatingButton.cpp
 *
 * Created on:  May 17, 2016
 * Author: 		Kevin Winata (k.winata@samsung.com)
 * Contributor: Gilang Mentari Hamidy (g.hamidy@samsung.com)
 */

#include "TFC/Components/FloatingButton.h"
#include "TFC/Framework/Application.h"
#include <efl_extension.h>

using namespace TFC::Components;

LIBAPI FloatingButton::FloatingButton() :
		buttonLeftImage(""),
		buttonRightImage(""),
		naviframe(nullptr),
		floatingButton(nullptr),
		buttonLeft(nullptr),
		buttonRight(nullptr),
		doubleButton(false),
		movementBlock(false),
		ButtonLeftImage(this),
		ButtonRightImage(this),
		DoubleButton(this),
		MovementBlock(this)
{

}

LIBAPI Evas_Object* FloatingButton::CreateComponent(Evas_Object* root)
{
	naviframe = root;

	floatingButton = eext_floatingbutton_add(naviframe);
	elm_object_part_content_set(naviframe, "elm.swallow.floatingbutton", floatingButton);

	buttonLeft = elm_button_add(floatingButton);
	elm_object_part_content_set(floatingButton, "button1", buttonLeft);
	//evas_object_smart_callback_add(buttonLeft, "clicked", &EFL::EvasSmartEventHandler, &eventButtonLeftClick);
	eventButtonLeftClick.Bind(buttonLeft, "clicked");

	if (buttonLeftImage.size() > 0) {
		auto image = elm_image_add(floatingButton);
		std::string path = TFC::Framework::ApplicationBase::GetResourcePath(buttonLeftImage.c_str());
		elm_image_file_set(image, path.c_str(), NULL);
		elm_object_part_content_set(buttonLeft, "icon", image);
	}

	if (doubleButton) {
		buttonRight = elm_button_add(floatingButton);
		elm_object_part_content_set(floatingButton, "button2", buttonRight);
		//evas_object_smart_callback_add(buttonRight, "clicked", &EFL::EvasSmartEventHandler, &eventButtonRightClick);
		eventButtonRightClick.Bind(buttonRight, "clicked");

		if (buttonRightImage.size() > 0) {
			auto image = elm_image_add(floatingButton);
			std::string path = TFC::Framework::ApplicationBase::GetResourcePath(buttonRightImage.c_str());
			elm_image_file_set(image, path.c_str(), NULL);
			elm_object_part_content_set(buttonRight, "icon", image);
		}
	}

	if (movementBlock) {
		eext_floatingbutton_movement_block_set(floatingButton, EINA_TRUE);
	}

	evas_object_show(floatingButton);

	return floatingButton;
}

LIBAPI void FloatingButton::SetWhiteBackground()
{
	elm_object_style_set(floatingButton, "white_bg");
}

LIBAPI void FloatingButton::SetButtonStyle(bool left, const std::string& style)
{
	auto button = (left) ? buttonLeft : buttonRight;
	elm_object_style_set(button, style.c_str());
}
