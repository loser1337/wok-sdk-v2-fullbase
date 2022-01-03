#pragma once

#define LAG_COMPENSATION_TICKS 32
#define DISABLE_INTERPOLATION 0
#define ENABLE_INTERPOLATION 1

#define TIME_TO_TICKS( dt ) ( (int)( 0.5f + (float)(dt) / interfaces::m_global_vars->m_interval_per_tick ) )
#define TICKS_TO_TIME( t )  ( interfaces::m_global_vars->m_interval_per_tick * ( t ) )
#define ROUND_TO_TICKS( t ) ( interfaces::m_global_vars->m_interval_per_tick * TIME_TO_TICKS( t ) )

class C_Tick_Record
{
public:
	explicit C_Tick_Record() : sequence(0), entity_flags(0), simulation_time(0), lower_body_yaw(0), cycle(0)
	{
	}

	~C_Tick_Record()
	{
	}

	void reset()
	{
		if (data_filled)
			return;

		origin.clear();
		velocity.clear();
		object_mins.clear();
		object_maxs.clear();
		hitbox_positon.clear();

		eye_angles.clear();
		abs_eye_angles.clear();

		sequence = 0;
		entity_flags = 0;

		simulation_time = 0.f;
		lower_body_yaw = 0.f;
		cycle = 0.f;

		//fill(begin(pose_paramaters), end(pose_paramaters), 0.f);
		//fill(begin(rag_positions), end(rag_positions), 0.f);

		data_filled = false;
	}

	vec3_t origin;
	vec3_t abs_origin;
	vec3_t velocity;
	vec3_t object_mins;
	vec3_t object_maxs;
	vec3_t hitbox_positon;

	qangle_t eye_angles;
	qangle_t abs_eye_angles;

	int sequence;
	int entity_flags;

	float simulation_time;
	float lower_body_yaw;
	float cycle;

	std::array<float, 24> pose_paramaters;
	std::array<float, 24> rag_positions;

	bool data_filled = false;
};

class C_Simulation_Data
{
public:
	C_Simulation_Data() : entity(nullptr), on_ground(false)
	{
	}

	~C_Simulation_Data()
	{
	}

	c_base_entity* entity;

	vec3_t origin;
	vec3_t velocity;

	bool on_ground;

	bool data_filled = false;
};

class C_Player_Record
{
public:
	C_Player_Record() : entity(nullptr), tick_count(0), being_lag_compensated(false), lower_body_yaw(0), last_lower_body_yaw_last_update(0)
	{
	}

	~C_Player_Record()
	{
	}

	void reset()
	{
		entity = nullptr;
		tick_count = -1;
		hitbox_position.clear();
		eye_angles.clear();
		being_lag_compensated = false;
		lower_body_yaw_resolved = false;
		lower_body_yaw = 0.f;
		last_lower_body_yaw_last_update = 0.f;

		restore_record.reset();

		if (!records->empty())
			records->clear();
	}

	c_base_entity* entity;
	C_Tick_Record restore_record;
	int tick_count;
	vec3_t hitbox_position;
	qangle_t eye_angles;
	bool being_lag_compensated;
	bool lower_body_yaw_resolved = false;
	float lower_body_yaw_resolved_yaw;
	float lower_body_yaw, last_lower_body_yaw_last_update;

	std::deque<C_Tick_Record> records[LAG_COMPENSATION_TICKS];
};

class C_LagCompensation
{
public:
	C_LagCompensation()
	{
	}

	~C_LagCompensation()
	{
	}

	enum round_gameflags
	{
		ROUND_STARTING = 0,
		ROUND_IN_PROGRESS,
		ROUND_ENDING,
	};

	round_gameflags round_flags;

	void frame_net_update_end() const;
	void paint_debug() const;
	static bool should_lag_compensate(c_base_entity* entity);
	static void set_interpolation_flags(c_base_entity* entity, int flag);
	static float get_interpolation();
	bool is_time_delta_too_large(C_Tick_Record wish_record) const;
	void update_player_record_data(c_base_entity* entity) const;
	static void fix_animation_data(c_base_entity* entity);
	static void store_record_data(c_base_entity* entity, C_Tick_Record* record_data);
	static void apply_record_data(c_base_entity* entity, C_Tick_Record* record_data);
	static void simulate_movement(C_Simulation_Data* data);
	void predict_player(c_base_entity* entity, C_Tick_Record* current_record, C_Tick_Record* next_record) const;
	bool get_lowerbodyyaw_update_tick(c_base_entity* entity, C_Tick_Record* tick_record, int* record_index) const;
	int start_lag_compensation(c_base_entity* entity, int wish_tick, C_Tick_Record* output_record = nullptr) const;
	void start_position_adjustment() const;
	void start_position_adjustment(c_base_entity* entity) const;
	void finish_position_adjustment() const;
	static void finish_position_adjustment(c_base_entity* entity);
	static void reset();
};