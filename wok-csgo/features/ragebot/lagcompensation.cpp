#include "../features.h"

void C_LagCompensation::frame_net_update_end() const
{
	if (globals::m_local == nullptr || !globals::m_local->is_alive() || globals::m_local->get_weapons() == nullptr || round_flags == ROUND_STARTING)
	{
		reset();
		return;
	}

	for (auto index = 1; index < interfaces::m_global_vars->m_max_clients; index++)
	{
		const auto entity = reinterpret_cast<c_base_entity*>(interfaces::m_entity_list->get_client_entity(index));
		
		const auto player = reinterpret_cast<c_cs_player*>(entity);

		if (!should_lag_compensate(entity))
		{
			G::player_lag_records[index - 1].records->clear();
			continue;
		}

		fix_animation_data(entity);
		set_interpolation_flags(entity, DISABLE_INTERPOLATION);
		update_player_record_data(entity);

		entity->client_side_animation() = true;
	}
}

bool C_LagCompensation::should_lag_compensate(c_base_entity* entity)
{
	if (globals::m_local == nullptr || G::settings.rage_positionadjustment == 0)
		return false;

	if (!G::hooks.createmove.ragebot->can_aimbot_player(entity))
		return false;

	return true;
}

void C_LagCompensation::set_interpolation_flags(c_base_entity* entity, int flag)
{
	auto var_map = reinterpret_cast<uintptr_t>(entity) + 36; // tf2 = 20
	auto var_map_list_count = *reinterpret_cast<int*>(var_map + 20);

	for (auto index = 0; index < var_map_list_count; index++)
		*reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(var_map) + index * 12) = flag;
}

float C_LagCompensation::get_interpolation()
{
	static auto cl_interp = interfaces::m_cvar_system->find_var(FNV1A("cl_interp"));
	static auto cl_interp_ratio = interfaces::m_cvar_system->find_var(FNV1A("cl_interp_ratio"));
	static auto sv_client_min_interp_ratio = interfaces::m_cvar_system->find_var(FNV1A("sv_client_min_interp_ratio"));
	static auto sv_client_max_interp_ratio = interfaces::m_cvar_system->find_var(FNV1A("sv_client_max_interp_ratio"));
	static auto cl_updaterate = interfaces::m_cvar_system->find_var(FNV1A("cl_updaterate"));
	static auto sv_minupdaterate = interfaces::m_cvar_system->find_var(FNV1A("sv_minupdaterate"));
	static auto sv_maxupdaterate = interfaces::m_cvar_system->find_var(FNV1A("sv_maxupdaterate"));

	auto updaterate = std::clamp(cl_updaterate->get_float(), sv_minupdaterate->get_float(), sv_maxupdaterate->get_float());
	auto lerp_ratio = std::clamp(cl_interp_ratio->get_float(), sv_client_min_interp_ratio->get_float(), sv_client_max_interp_ratio->get_float());
	return std::clamp(lerp_ratio / updaterate, cl_interp->get_float(), 1.0f);
}

bool C_LagCompensation::is_time_delta_too_large(C_Tick_Record wish_record) const
{
	// @note: бульдозер переделай interfaces::engine->get_avg_latency(0)
	auto predicted_arrival_tick = TIME_TO_TICKS(interfaces::engine->get_avg_latency(0) + G::interfaces.engine->get_avg_latency(1)) + (G::original_cmd.tick_count + G::interfaces.clientstate->client_state->choked_commands + 1);

	auto lerp_time = get_interpolation();
	auto correct = std::clamp(G::interfaces.engine->get_latency(0) + lerp_time, 0.f, 1.f);
	auto lag_delta = correct - TICKS_TO_TIME(predicted_arrival_tick + TIME_TO_TICKS(lerp_time - wish_record.simulation_time + lerp_time));

	return abs(lag_delta) > static_cast<float>(G::settings.rage_positionadjustment_max_history) / 1000.f;
}

void C_LagCompensation::store_record_data(c_base_entity* entity, C_Tick_Record* record_data)
{
	if (entity == nullptr)
		return;

	record_data->origin = entity->get_origin();
	record_data->abs_origin = entity->get_abs_origin();
	record_data->velocity = entity->get_abs_velocity();
	// @note: нужно адднуть то чего нету (get_obbmins, get_obbmaxs, get_eye_angles, get_abs_eye_angles,
	//  get_sequence, get_entity_flags, get_simulation_time, get_lower_body_yaw, get_cycle, get_pose_paramaters, get_ragdoll_pos)
	//  и убрать коммент

	// @note: спиздить можно из гучи https://github.com/notphage/gucci

	//    record_data->object_mins = entity->get_obbmins();
	//    record_data->object_maxs = entity->get_obbmaxs();
	//    record_data->eye_angles = entity->get_eye_angles();
	//    record_data->abs_eye_angles = entity->get_abs_eye_angles();
	//    record_data->sequence = entity->get_sequence();
	//    record_data->entity_flags = entity->get_entity_flags();
	//    record_data->simulation_time = entity->get_simulation_time();
	//    record_data->lower_body_yaw = entity->get_lower_body_yaw();
	//    record_data->cycle = entity->get_cycle();
	//    record_data->pose_paramaters = entity->get_pose_paramaters();
	//    record_data->rag_positions = entity->get_ragdoll_pos();

	record_data->data_filled = true;
}

// @note: чекни коммент у store_record_data
void C_LagCompensation::apply_record_data(c_base_entity* entity, C_Tick_Record* record_data)
{
	if (entity == nullptr || !record_data->data_filled)
		return;

	entity->get_origin() = record_data->origin;
	entity->get_velocity() = record_data->velocity;
	entity->get_obbmins() = record_data->object_mins;
	entity->get_obbmaxs() = record_data->object_maxs;
	entity->get_eye_angles() = record_data->eye_angles;
	entity->get_abs_eye_angles() = record_data->abs_eye_angles;
	entity->get_sequence() = record_data->sequence;
	entity->get_entity_flags() = record_data->entity_flags;
	entity->get_simulation_time() = record_data->simulation_time;
	entity->get_lower_body_yaw() = record_data->lower_body_yaw;
	entity->get_cycle() = record_data->cycle;
	entity->get_pose_paramaters() = record_data->pose_paramaters;
	entity->get_ragdoll_pos() = record_data->rag_positions;
	entity->set_abs_angles(record_data->abs_eye_angles);
	entity->set_abs_origin(record_data->abs_origin);
}

void C_LagCompensation::simulate_movement(C_Simulation_Data* data)
{
	c_game_trace trace;
	c_trace_filter filter(globals::m_local, TRACE_EVERYTHING);

	auto sv_gravity = interfaces::m_cvar_system->find_var(FNV1A("sv_gravity"))->get_int();
	auto sv_jump_impulse = interfaces::m_cvar_system->find_var(FNV1A("sv_jump_impulse"))->get_float();
	auto gravity_per_tick = sv_gravity * interfaces::m_global_vars->m_interval_per_tick;
	auto predicted_origin = data->origin;

	if (data->on_ground)
		data->velocity.z -= gravity_per_tick;

	predicted_origin += data->velocity * interfaces::m_global_vars->m_interval_per_tick;

	interfaces::m_trace_system->trace_ray(ray_t(data->origin, predicted_origin, data->entity->get_obbmins(), data->entity->get_obbmaxs()), CONTENTS_SOLID, &filter, &trace);

	if (trace.m_fraction == 1.f)
		data->origin = predicted_origin;

	interfaces::m_trace_system->trace_ray(ray_t(data->origin, data->origin - vec3_t(0.f, 0.f, 2.f), data->entity->get_obbmins(), data->entity->get_obbmaxs()), CONTENTS_SOLID, &filter, &trace);

	data->on_ground = trace.m_fraction == 0.f;
}

int C_LagCompensation::start_lag_compensation(c_base_entity* entity, int wish_tick, C_Tick_Record* output_record) const
{
	if (!should_lag_compensate(entity))
		return -1;

	auto player_index = entity->get_index() - 1;
	auto player_record = G::player_lag_records[player_index];

	if (player_record.records->empty() || wish_tick + 1 > player_record.records->size() - 1)
		return -1;

	auto current_record = player_record.records->at(wish_tick);
	auto next_record = player_record.records->at(wish_tick + 1);

	if (!current_record.data_filled || !next_record.data_filled || wish_tick > 0 && is_time_delta_too_large(current_record))
		return -1;

	if (wish_tick == 0 && (current_record.origin - next_record.origin).length_sqr() > 4096.f)
		predict_player(entity, &current_record, &next_record);

	if (output_record != nullptr && current_record.data_filled)
		*output_record = current_record;

	return TIME_TO_TICKS(current_record.simulation_time + get_interpolation());
}