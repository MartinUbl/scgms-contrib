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

#include "descriptor.h"

#include <scgms/utils/descriptor_utils.h>
#include <scgms/iface/DeviceIface.h>
#include <scgms/lang/dstrings.h>
#include <scgms/rtl/manufactory.h>

#include "lotka-volterra/lvmodel.h"

#include <vector>

namespace lotka_volterra {

	const wchar_t* model_param_ui_names[param_count] = {
		L"predators_0", L"prey_0", L"prey_birth", L"prey_death_by_pred", L"pred_birth_by_prey", L"pred_death"
	};

	const scgms::NModel_Parameter_Value model_param_types[param_count] = {
		scgms::NModel_Parameter_Value::mptDouble, scgms::NModel_Parameter_Value::mptDouble,
		scgms::NModel_Parameter_Value::mptDouble, scgms::NModel_Parameter_Value::mptDouble,
		scgms::NModel_Parameter_Value::mptDouble, scgms::NModel_Parameter_Value::mptDouble,
	};

	constexpr size_t number_of_calculated_signals = 2;

	const GUID calculated_signal_ids[number_of_calculated_signals] = {
		predator_signal_id,
		prey_signal_id
	};

	const wchar_t* calculated_signal_names[number_of_calculated_signals] = {
		L"LV - Predator count",
		L"LV - Prey count"
	};

	const GUID reference_signal_ids[number_of_calculated_signals] = {
		prey_signal_id,
		predator_signal_id
	};

	scgms::TModel_Descriptor desc = {
		model_id,
		scgms::NModel_Flags::Discrete_Model,
		L"Lotka-Volterra",
		nullptr,
		param_count,
		param_count,
		model_param_types,
		model_param_ui_names,
		nullptr,
		lower_bounds.vector,
		default_parameters.vector,
		upper_bounds.vector,

		number_of_calculated_signals,
		calculated_signal_ids,
		reference_signal_ids,
	};

	const scgms::TSignal_Descriptor predator_desc{ predator_signal_id, L"Lotka-Volterra - Predators", L"", scgms::NSignal_Unit::Unitless, 0xFFff2200, 0xFFff2200, scgms::NSignal_Visualization::smooth, scgms::NSignal_Mark::none, nullptr, 1.0 };
	const scgms::TSignal_Descriptor prey_desc{ prey_signal_id, L"Lotka-Volterra - Prey", L"", scgms::NSignal_Unit::Unitless, 0xFF22ff00, 0xFF22ff00, scgms::NSignal_Visualization::smooth, scgms::NSignal_Mark::none, nullptr, 1.0 };
}

const std::array<scgms::TModel_Descriptor, 1> model_descriptions = { {
	lotka_volterra::desc
} };

const std::array<scgms::TSignal_Descriptor, 2> signals_descriptors = { {
	lotka_volterra::predator_desc,
	lotka_volterra::prey_desc
} };

DLL_EXPORT HRESULT IfaceCalling do_get_model_descriptors(scgms::TModel_Descriptor **begin, scgms::TModel_Descriptor **end) {
	*begin = const_cast<scgms::TModel_Descriptor*>(model_descriptions.data());
	*end = *begin + model_descriptions.size();
	return S_OK;
}

DLL_EXPORT HRESULT IfaceCalling do_get_signal_descriptors(scgms::TSignal_Descriptor * *begin, scgms::TSignal_Descriptor * *end) {
	*begin = const_cast<scgms::TSignal_Descriptor*>(signals_descriptors.data());
	*end = *begin + signals_descriptors.size();
	return S_OK;
}

DLL_EXPORT HRESULT IfaceCalling do_create_discrete_model(const GUID *model_id, scgms::IModel_Parameter_Vector *parameters, scgms::IFilter *output, scgms::IDiscrete_Model **model) {
	if (*model_id == lotka_volterra::desc.id) {
		return Manufacture_Object<CLotka_Volterra_Model>(model, parameters, output);
	}

	return E_NOTIMPL;
}
