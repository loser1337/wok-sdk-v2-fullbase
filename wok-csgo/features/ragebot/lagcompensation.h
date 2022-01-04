#include "../../globals.h"

#pragma region lagcompensation_definitions
#define LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR ( 64.0f * 64.0f )
#define LAG_COMPENSATION_EPS_SQR ( 0.1f * 0.1f )
#define LAG_COMPENSATION_ERROR_EPS_SQR ( 4.0f * 4.0f )
#pragma endregion

#pragma region lagcompensation_enumerations
enum ELagCompensationState
{
	LC_NO = 0,
	LC_ALIVE = (1 << 0),
	LC_ORIGIN_CHANGED = (1 << 8),
	LC_ANGLES_CHANGED = (1 << 9),
	LC_SIZE_CHANGED = (1 << 10),
	LC_ANIMATION_CHANGED = (1 << 11)
};
#pragma endregion

struct SequenceObject_t
{
	SequenceObject_t(int iInReliableState, int iOutReliableState, int iSequenceNr, float flCurrentTime)
		: iInReliableState(iInReliableState), iOutReliableState(iOutReliableState), iSequenceNr(iSequenceNr), flCurrentTime(flCurrentTime) { }

	int iInReliableState;
	int iOutReliableState;
	int iSequenceNr;
	float flCurrentTime;
};

class CLagRecord
{
	vec3_t vecOrigin, vecVelocity, vecMins, vecMaxs;
	qangle_t angAngles, angAbsAngels;
	int32_t iFlags, iEFlags, iSimulationTime, iPlayerID;
	float_t flLby, flDuckAmount;
	bool bIsDormant, bIsShooting, bIsImmune, bIsValid;


	c_anim_state* pAnimstate;
	anim_layers_t  pLeftLayers, pRightLayers, pCenterLayers;
	std::array<float_t, 24> flPoseParameters;
	c_cs_player* pPlayer;


	CLagRecord(c_cs_player* pPlayer)
	{
		vecOrigin = pPlayer->get_origin(), vecVelocity = pPlayer->get_velocity();
		angAngles = pPlayer->get_eye_angles(), angAbsAngels = pPlayer->get_abs_angles();
		iFlags = pPlayer->get_flags(), iEFlags = pPlayer->get_eflags(), iSimulationTime = pPlayer->get_sim_time(), iPlayerID = pPlayer->get_index();
		flLby = pPlayer->get_lby();
		bIsDormant = pPlayer->is_dormant(), bIsImmune = pPlayer->is_immune();
		pAnimstate = pPlayer->get_anim_state();
		this->pPlayer = pPlayer;
	};

	void Reset()
	{
		vecOrigin = vec3_t(0, 0, 0), vecVelocity = vec3_t(0, 0, 0), vecMins = vec3_t(0, 0, 0), vecMaxs = vec3_t(0, 0, 0);
		angAngles = qangle_t(0, 0, 0), angAbsAngels = qangle_t(0, 0, 0);
		iFlags = int32_t(0), iEFlags = int32_t(0), iSimulationTime = int32_t(0), iPlayerID = int32_t(0);
		flLby = float_t(0.f), flDuckAmount = float_t(0.f);
		bIsDormant = bool(false), bIsShooting = bool(false), bIsImmune = bool(false), bIsValid = bool(false);
		pPlayer = nullptr;
	};

};

extern std::deque<CLagRecord> pLagRecords;

class CLagCompensation : public c_singleton<CLagCompensation>
{
public:
	void Run(c_user_cmd* pCmd);

	void UpdateIncomingSequences(i_net_channel* pNetChannel);
	void ClearIncomingSequences();
	void AddLatencyToNetChannel(i_net_channel* pNetChannel, float flLatency);
	void store_record(c_cs_player* pEntity);
	float_t lerp_time();
	bool valid_tick(int iTick);
private:
	std::deque<SequenceObject_t> vecSequences = { };
	int nRealIncomingSequence = 0;
	int nLastIncomingSequence = 0;
};

#define lagcompensation CLagCompensation::instance()