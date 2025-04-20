/**
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Copyright (c) since 2018 University of West Bohemia.
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Univerzitni 8, 301 00 Pilsen
 * Czech Republic
 * 
 * 
 * Purpose of this software:
 * This software is intended to demonstrate work of the diabetes.zcu.cz research
 * group to other scientists, to complement our published papers. It is strictly
 * prohibited to use this software for diagnosis or treatment of any medical condition,
 * without obtaining all required approvals from respective regulatory bodies.
 *
 * Especially, a diabetic patient is warned that unauthorized use of this software
 * may result into severe injure, including death.
 *
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under these license terms is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * a) This file is available under the Apache License, Version 2.0.
 * b) When publishing any derivative work or results obtained using this software, you agree to cite the following paper:
 *    Tomas Koutny and Martin Ubl, "SmartCGMS as a Testbed for a Blood-Glucose Level Prediction and/or 
 *    Control Challenge with (an FDA-Accepted) Diabetic Patient Simulation", Procedia Computer Science,  
 *    Volume 177, pp. 354-362, 2020
 */

#pragma once

#include <scgms/iface/DeviceIface.h>
#include <scgms/iface/UIIface.h>
#include <scgms/rtl/hresult.h>
#include <scgms/rtl/ModelsLib.h>

namespace lotka_volterra {
	constexpr const GUID model_id = { 0x948c69e3, 0x88c0, 0x4571, { 0x93, 0x39, 0xf5, 0x46, 0x9d, 0x7a, 0x23, 0x48 } }; // {948C69E3-88C0-4571-9339-F5469D7A2348}
	constexpr const GUID predator_signal_id = { 0x4f065b, 0xd40b, 0x4830, { 0xa1, 0xec, 0xef, 0x99, 0x6, 0x28, 0x10, 0x41 } }; // {004F065B-D40B-4830-A1EC-EF9906281041}
	constexpr const GUID prey_signal_id = { 0xa8b52c6c, 0x95a7, 0x4a74, { 0xb3, 0xcf, 0xd, 0x29, 0x47, 0xd9, 0xec, 0x87 } }; // {A8B52C6C-95A7-4A74-B3CF-0D2947D9EC87}

	const size_t param_count = 6;

	struct TParameters {
		union {
			struct {
				double pred_0;
				double prey_0;
				double prey_birth;
				double prey_death_by_pred;
				double pred_birth_by_prey;
				double pred_death;
			};
			double vector[param_count];
		};
	};

	const TParameters lower_bounds = {
		0,
		0,
		0.001,
		0.001,
		0.001,
		0.001
	};
	const TParameters default_parameters = {
		50,
		50,
		0.01,
		0.01,
		0.01,
		0.01
	};
	const TParameters upper_bounds = {
		100000,
		100000,
		0.9,
		0.9,
		0.9,
		0.9
	};
}
