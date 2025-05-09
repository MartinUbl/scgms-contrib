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

#include "factory.h"

#include "descriptor.h"

#include <map>
#include <functional>

#include <scgms/rtl/manufactory.h>
#include <scgms/rtl/DeviceLib.h>

using TCreate_Signal = std::function<HRESULT(scgms::ITime_Segment *segment, scgms::ISignal **signal)>;

class CId_Dispatcher {
	protected:
		std::map<const GUID, TCreate_Signal, std::less<GUID>> id_map;

		template <typename T>
		HRESULT Create_X(scgms::ITime_Segment *segment, scgms::ISignal **signal) const {
			scgms::WTime_Segment weak_segment{ segment };
			return Manufacture_Object<T, scgms::ISignal>(signal, weak_segment);
		}

		template <typename T>
		void Add_Signal(const GUID &id) {
			id_map[id] = std::bind(&CId_Dispatcher::Create_X<T>, this, std::placeholders::_1, std::placeholders::_2);
		}
	public:
		CId_Dispatcher() {
			// placeholder, will be utilized in future
			//Add_Signal<CSignal_Class_Please_Define_Me>(my_model::signal_My_Model_Signal_Please_Define_Me);
		}

		HRESULT Create_Signal(const GUID &calc_id, scgms::ITime_Segment *segment, scgms::ISignal **signal) const {
			const auto iter = id_map.find(calc_id);
			if (iter != id_map.end()) {
				return iter->second(segment, signal);
			}
			else {
				return E_NOTIMPL;
			}
		}
};

static CId_Dispatcher Id_Dispatcher;

DLL_EXPORT HRESULT IfaceCalling do_create_signal(const GUID *calc_id, scgms::ITime_Segment *segment, const GUID * approx_id, scgms::ISignal **signal) {
	if ((calc_id == nullptr) || (segment == nullptr)) {
		return E_INVALIDARG;
	}
	return Id_Dispatcher.Create_Signal(*calc_id, segment, signal);
}
