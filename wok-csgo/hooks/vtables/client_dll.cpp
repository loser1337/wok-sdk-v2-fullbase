#include "../hooks.h"
#include "../../features/ragebot/lagcompensation.h"

int __stdcall hooks::client_dll::hkSendDatagram::fn(i_net_channel* thisptr, int edx, c_bf_write* pDatagram)
{
	static const auto original = m_client_dll->get_original<T>(index);

	i_net_channel_info* pNetChannelInfo = interfaces::m_engine->get_net_channel_info();
	static c_cvar* sv_maxunlag = interfaces::m_cvar_system->find_var(FNV1A("sv_maxunlag"));

	if (!interfaces::m_engine->is_in_game() || pDatagram != nullptr || pNetChannelInfo == nullptr || sv_maxunlag == nullptr)
		return original(thisptr, edx, pDatagram);

	const int iOldInReliableState = thisptr->m_in_reliable_state;
	const int iOldInSequenceNr = thisptr->m_in_sequence_number;

	const float flMaxLatency = std::max(0.f, std::clamp(1.f, 0.f, sv_maxunlag->get_float()) - pNetChannelInfo->get_latency(FLOW_OUTGOING));
	lagcompensation->AddLatencyToNetChannel(thisptr, flMaxLatency);

	const int iReturn = original(thisptr, edx, pDatagram);

	thisptr->m_in_reliable_state = iOldInReliableState;
	thisptr->m_in_sequence_number = iOldInSequenceNr;

	return iReturn;
}

void __stdcall hooks::client_dll::frame_stage_notify::fn(e_client_frame_stage stage) {
	static const auto original = m_client_dll->get_original<T>(index);

	if (!interfaces::m_engine->is_in_game()) {
        lagcompensation->ClearIncomingSequences();
		return original(stage);
	}

	if (interfaces::m_engine->is_taking_screenshot())
		return original(stage);

	if (globals::m_local == nullptr)
		return original(stage);

	static qangle_t angOldAimPunch = { }, angOldViewPunch = { };

	switch (stage)
	{
	    case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
	    {
		/*
		 * data has been received and we are going to start calling postdataupdate
		 * e.g. resolver or skinchanger and other visuals
		 */

		    break;
	    }
	    case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
	    {
		/*
		 * data has been received and called postdataupdate on all data recipients
		 * e.g. now we can modify interpolation, other lagcompensation stuff
		 */

		    break;
	    }
	    case FRAME_NET_UPDATE_END:
	    {
		/*
		 * received all packets, now do interpolation, prediction, etc
		 * e.g. backtrack stuff
		 */
		    break;
	    }
	    case FRAME_RENDER_START:
	    {
		    if (interfaces::m_engine->is_in_game())
		    	interfaces::m_engine->get_view_angles(globals::angles::m_view);
			break;
	    }
	    case FRAME_RENDER_END:
	    {
		/*
		 * finished rendering the scene
		 * here we can restore our modified things
		 */

		    break;
	    }
	    default:
		    break;
	}

	original(stage);
}

__declspec (naked) void __stdcall hooks::client_dll::create_move::gate(int sequence_number, float input_sample_frame_time, bool active) {
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esp
		push dword ptr [active]
		push dword ptr [input_sample_frame_time]
		push dword ptr [sequence_number]
		call fn
		pop ebx
		pop ebp
		retn 0Ch
	}
}

void __stdcall hooks::client_dll::create_move::fn(int sequence_number, float input_sample_frame_time, bool active, bool& packet) {
	static const auto original = m_client_dll->get_original<T>(index);

	original(interfaces::m_client_dll, sequence_number, input_sample_frame_time, active);

	globals::m_packet = packet = true;

	if (!globals::m_local)
		return;

	const auto cmd = interfaces::m_input->get_user_cmd(sequence_number);
	if (!cmd
		|| !cmd->m_command_number)
		return;

	globals::m_cur_cmd = cmd;

	movement->set_view_angles(cmd->m_view_angles);

	engine_prediction->update();

	movement->on_create_move(false);

	engine_prediction->process();

	lagcompensation->Run(cmd);

	engine_prediction->restore();

	cmd->m_view_angles.sanitize();

	globals::angles::m_anim = cmd->m_view_angles;

	movement->on_create_move(true);

	packet = globals::m_packet;

	const auto verified = interfaces::m_input->get_verified_user_cmd(sequence_number);

	verified->m_cmd = *cmd;
	verified->m_crc = cmd->get_check_sum();
}
