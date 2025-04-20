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

#include "lvmodel.h"

#include <scgms/rtl/SolverLib.h>
#include <scgms/rtl/ApproxLib.h>
#include <scgms/utils/math_utils.h>
#include <scgms/utils/DebugHelper.h>

CLotka_Volterra_Model::CLotka_Volterra_Model(scgms::IModel_Parameter_Vector* parameters, scgms::IFilter* output)
	: CBase_Filter(output), mParameters(scgms::Convert_Parameters<lotka_volterra::TParameters>(parameters, lotka_volterra::default_parameters.vector)) {
	//
}

HRESULT CLotka_Volterra_Model::Do_Execute(scgms::UDevice_Event event) {
	return mOutput.Send(event);
}

HRESULT CLotka_Volterra_Model::Do_Configure(scgms::SFilter_Configuration configuration, refcnt::Swstr_list& error_description) {
	return S_OK;
}

HRESULT IfaceCalling CLotka_Volterra_Model::Initialize(const double current_time, const uint64_t segment_id) {
	mCurrent_Time = current_time;
	mSegment_Id = segment_id;

	auto& pred = mModel[NLV_Compartment::Predator].Create_Depot(mParameters.pred_0, false);
	auto& prey = mModel[NLV_Compartment::Prey].Create_Depot(mParameters.prey_0, false);

	pred
		.Set_Solution_Volume(1.0)
		.Set_Name(L"Predator")
		.Set_Persistent(true);

	prey
		.Set_Solution_Volume(1.0)
		.Set_Name(L"Prey")
		.Set_Persistent(true);

	auto& pred_src = mModel[NLV_Compartment::Predator_Source].Create_Depot<gct::CSource_Depot>(1.0, false);
	auto& pred_sink = mModel[NLV_Compartment::Predator_Sink].Create_Depot<gct::CSink_Depot>(1.0, false);
	auto& prey_src = mModel[NLV_Compartment::Prey_Source].Create_Depot<gct::CSource_Depot>(1.0, false);
	auto& prey_sink = mModel[NLV_Compartment::Prey_Sink].Create_Depot<gct::CSink_Depot>(1.0, false);

	pred_src
		.Set_Solution_Volume(1.0)
		.Set_Name(L"Predator Source")
		.Set_Persistent(true);

	pred_sink
		.Set_Solution_Volume(1.0)
		.Set_Name(L"Predator Sink")
		.Set_Persistent(true);

	prey_src
		.Set_Solution_Volume(1.0)
		.Set_Name(L"Prey Source")
		.Set_Persistent(true);

	prey_sink
		.Set_Solution_Volume(1.0)
		.Set_Name(L"Prey Sink")
		.Set_Persistent(true);

	// birth of prey (moderated positively by amount of prey)
	prey_src.Moderated_Link_To<gct::CConstant_Unbounded_Transfer_Function>(
		prey,
		[&prey, this](gct::CDepot_Link& link) {
			link.Add_Moderator<gct::CLinear_Moderation_No_Elimination_Function>(prey, 1.0);
		},
		gct::CTransfer_Function::Start,
		gct::CTransfer_Function::Unlimited,
		mParameters.prey_birth
	);

	// death of prey (moderated by current amount of predators)
	prey.Moderated_Link_To<gct::CConstant_Unbounded_Transfer_Function>(
		prey_sink,
		[&prey, &pred, this](gct::CDepot_Link& link) {
			link.Add_Moderator<gct::CLinear_Moderation_No_Elimination_Function>(pred, 1.0);
		},
		gct::CTransfer_Function::Start,
		gct::CTransfer_Function::Unlimited,
		mParameters.prey_death_by_pred
	);

	// birth of predators (moderated by current amount of prey)
	pred_src.Moderated_Link_To<gct::CConstant_Unbounded_Transfer_Function>(
		pred,
		[&pred, &prey, this](gct::CDepot_Link& link) {
			link.Add_Moderator<gct::CLinear_Moderation_No_Elimination_Function>(pred, 1.0);
			link.Add_Moderator<gct::CLinear_Moderation_No_Elimination_Function>(prey, 1.0);
		},
		gct::CTransfer_Function::Start,
		gct::CTransfer_Function::Unlimited,
		mParameters.pred_birth_by_prey
	);

	// death of predators
	pred.Link_To<gct::CConstant_Unbounded_Transfer_Function>(
		pred_sink,
		gct::CTransfer_Function::Start,
		gct::CTransfer_Function::Unlimited,
		mParameters.pred_death
	);


	// initialize all compartments
	std::for_each(mModel.begin(), mModel.end(), [current_time](auto& compartment) {
		compartment.Init(current_time);
	});

	return S_OK;
}

void CLotka_Volterra_Model::Emit_Signal_Level(double time, const GUID& signal_id, double level) {
	scgms::UDevice_Event event{ scgms::NDevice_Event_Code::Level };
	event.signal_id() = signal_id;
	event.device_time() = time;
	event.level() = level;
	event.segment_id() = mSegment_Id;
	event.device_id() = lotka_volterra::model_id;
	mOutput.Send(event);
}

void CLotka_Volterra_Model::Emit_All_Signals(double time) {

	// emit predator amount signal
	Emit_Signal_Level(time, lotka_volterra::predator_signal_id, mModel[NLV_Compartment::Predator].Get_Quantity());

	// emit prey amount signal
	Emit_Signal_Level(time, lotka_volterra::prey_signal_id, mModel[NLV_Compartment::Prey].Get_Quantity());
}

HRESULT IfaceCalling CLotka_Volterra_Model::Step(const double time_advance_delta) {
	if (time_advance_delta > 0) {

		mCurrent_Time += time_advance_delta;

		// step all compartments
		std::for_each(mModel.begin(), mModel.end(), [this](auto& compartment) {
			compartment.Step(mCurrent_Time);
		});

		// commit all compartments
		std::for_each(mModel.begin(), mModel.end(), [this](auto& compartment) {
			compartment.Commit(mCurrent_Time);
		});

		// emit all signals
		Emit_All_Signals(mCurrent_Time);
	}
	else if (time_advance_delta == 0.0) {

		// emit current state
		Emit_All_Signals(mCurrent_Time);
	}

	return S_OK;
}
