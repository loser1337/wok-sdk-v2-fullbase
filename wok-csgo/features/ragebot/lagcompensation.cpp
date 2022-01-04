#include "lagcompensation.h"

std::deque<CLagRecord> pLagRecords;

float_t CLagCompensation::lerp_time()
{
	int32_t iUpdateRate = interfaces::m_cvar_system->find_var(FNV1A("cl_updaterate"))->get_int();
	c_cvar* minupdate = interfaces::m_cvar_system->find_var(FNV1A("sv_minupdaterate"));
	c_cvar* maxupdate = interfaces::m_cvar_system->find_var(FNV1A("sv_maxupdaterate"));

	if (minupdate && maxupdate)
		iUpdateRate = maxupdate->get_int();
	float_t flratio = interfaces::m_cvar_system->find_var(FNV1A("cl_interp_ratio"))->get_float();

	if (flratio == 0)
		flratio == 1.0f;
	float_t flLerp = interfaces::m_cvar_system->find_var(FNV1A("cl_interp"))->get_float();
	c_cvar* cmin = interfaces::m_cvar_system->find_var(FNV1A("sv_client_min_interp_ratio"));
	c_cvar* cmax = interfaces::m_cvar_system->find_var(FNV1A("sv_client_max_interp_ratio"));

	if (cmin && cmax && cmin->get_float() != 1)
		flratio = std::clamp(flratio, cmin->get_float(), cmax->get_float());

	return std::max(flLerp, (flratio / iUpdateRate));
}

bool CLagCompensation::valid_tick(int iTick)
{
	auto nci = interfaces::m_engine->get_net_channel_info();
	if (!nci)
		return false;
	auto iPredictedCmdArrivalTick = interfaces::m_global_vars->m_tick_count + 1 + TIME_TO_TICKS(nci->get_avg_latency(FLOW_INCOMING) + nci->get_avg_latency(FLOW_OUTGOING));
	auto iCorrect = std::clamp(lerp_time() + nci->get_latency(FLOW_OUTGOING), 0.f, 1.f) - TIME_TO_TICKS(iPredictedCmdArrivalTick + TIME_TO_TICKS(lerp_time()) - (iTick + TIME_TO_TICKS(lerp_time())));

	return(abs(iCorrect) <= 0.2f);
}

void CLagCompensation::Run(c_user_cmd* pCmd)
{
	for (int32_t pPlayerID = 1; pPlayerID < interfaces::m_global_vars->m_max_clients; pPlayerID++)
	{
		c_cs_player* pPlayer = (c_cs_player*)(interfaces::m_entity_list->get_client_entity(pPlayerID));

		if (pPlayer == globals::local)
			continue;
	}
}

void CLagCompensation::UpdateIncomingSequences(i_net_channel* pNetChannel)
{
	if (pNetChannel == nullptr)
		return;

	if (nLastIncomingSequence == 0)
		nLastIncomingSequence = pNetChannel->m_in_sequence_number;

	if (pNetChannel->m_in_sequence_number > nLastIncomingSequence)
	{
		nLastIncomingSequence = pNetChannel->m_in_sequence_number;
		vecSequences.emplace_front(SequenceObject_t(pNetChannel->m_in_reliable_state, pNetChannel->m_out_reliable_state, pNetChannel->m_in_sequence_number, interfaces::m_global_vars->m_real_time));
	}

	if (vecSequences.size() > 2048U)
		vecSequences.pop_back();
}

// @note: solvet or maximbulldozer add store_record and apply_store_record.

void CLagCompensation::ClearIncomingSequences()
{
	if (!vecSequences.empty())
	{
		nLastIncomingSequence = 0;
		vecSequences.clear();
	}
}

void CLagCompensation::AddLatencyToNetChannel(i_net_channel* pNetChannel, float flLatency)
{
	for (const auto& sequence : vecSequences)
	{
		if (interfaces::m_global_vars->m_real_time - sequence.flCurrentTime >= flLatency)
		{
			pNetChannel->m_in_reliable_state = sequence.iInReliableState;
			pNetChannel->m_in_sequence_number = sequence.iSequenceNr;
			break;
		}
	}
}

void UpdateAnimation(c_cs_player* pEntity)
{
	anim_layers_t* pAnimLayers; 
	float flPoseParameters[24];
	float_t flBackupCurtime = interfaces::m_global_vars->m_cur_time;
	int32_t iBackupFrameCount = interfaces::m_global_vars->m_frame_count;
	float_t flBackupFrameTime = interfaces::m_global_vars->m_frame_time;

	interfaces::m_global_vars->m_cur_time = pEntity->get_sim_time();
	interfaces::m_global_vars->m_frame_time = interfaces::m_global_vars->m_interval_per_tick;

	pEntity->get_eflags() = pEntity->get_eflags() & 0x1000;

	pEntity->update_client_side_animation();
	pEntity->get_client_side_animation() = true;
	pEntity->update_client_side_animation();
	pEntity->get_client_side_animation() = false;

	// * @note: commented code need fix (btw ia ne spal yzhe dnya 3)
	// *
	// *

	//std::memcpy(pAnimLayers, &pEntity->get_anim_layers(), sizeof(anim_layers_t) * 13);
    std::memcpy(flPoseParameters, pEntity->get_pose_params().data(), 24);

	interfaces::m_global_vars->m_cur_time = flBackupCurtime;
	interfaces::m_global_vars->m_frame_time = flBackupFrameTime;

	//std::memcpy(&pEntity->get_anim_layers(), pAnimLayers, sizeof(anim_layers_t) * 13);
	std::memcpy(pEntity->get_pose_params().data(), flPoseParameters, 24);
}