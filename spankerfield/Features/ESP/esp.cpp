#include "esp.h"
#include "../../settings.h"
#include "../../SDK/sdk.h"
#include "../../Rendering/draw-list.h"
#include "../../Utilities/w2s.h"
#include "../../Utilities/other.h"
#include "../../Rendering/nicknames.h"
#include "../../global.h"

using namespace big;
namespace plugins
{
	void draw_bone(const ImColor& color, RagdollComponent* ragdoll, UpdatePoseResultData::BONES bone_1, UpdatePoseResultData::BONES bone_2, bool dots)
	{
		if (!IsValidPtr(ragdoll))
			return;

		Vector3 bone_1_vec;
		if (!ragdoll->GetBone(bone_1, bone_1_vec))
			return;

		Vector3 bone_2_vec;
		if (!ragdoll->GetBone(bone_2, bone_2_vec))
			return;

		if (!world_to_screen(bone_1_vec))
			return;

		if (!world_to_screen(bone_2_vec))
			return;

		auto pos_1 = Vector2(bone_1_vec.x, bone_1_vec.y);
		auto pos_2 = Vector2(bone_2_vec.x, bone_2_vec.y);

		if (dots)
		{
			m_drawing->DrawFillArea(pos_1.x, pos_1.y, 3.5f, 3.5f, color);
			m_drawing->DrawFillArea(pos_2.x, pos_2.y, 3.5f, 3.5f, color);
		}

		m_drawing->AddLine(ImVec2(pos_1.x, pos_1.y), ImVec2(pos_2.x, pos_2.y), color);
	}

	std::map<uint64_t, std::string> streamer_personas{};
	void draw_esp()
	{
		if (!g_settings.esp) return;

		const auto game_context = ClientGameContext::GetInstance();
		if (!game_context) return;

		const auto player_manager = game_context->m_pPlayerManager;
		if (!player_manager) return;

		const auto local_player = player_manager->m_pLocalPlayer;
		if (!local_player) return;

		const auto local_soldier = local_player->GetSoldier();
		if (!local_soldier) return;

		if (!local_soldier->IsAlive()) return;

		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			const auto player = player_manager->m_ppPlayers[i];
			if (!player)
				continue;

			if (player == local_player)
				continue;

			bool teammate = player->m_TeamId == local_player->m_TeamId;
			if (teammate && !g_settings.esp_draw_teammates)
				continue;

			const auto soldier = player->GetSoldier();
			if (!soldier) continue;

			if (!soldier->IsAlive())
				continue;

			TransformAABBStruct transform = get_transform(player);
			TransformAABBStruct local_transform = get_transform(local_player);;

			Vector3 local_pos = (Vector3)local_transform.Transform.m[3];
			Vector3 pos = (Vector3)transform.Transform.m[3];
			Vector2 box_coords[2];

			const char* nickname = IsValidPtr(player->m_Name) ? player->m_Name : xorstr_("Unknown");
			
			// Streamer mode
			if (g_settings.streamer_mode)
			{
				// Player ID
				uint64_t persona_id = player->m_onlineId.m_personaid;
				
				// Saving the nickname for that player in memory so they are consistent
				if (persona_id != NULL)
				{
					if (streamer_personas.count(persona_id) <= 0)
						streamer_personas[persona_id] = get_random_string(weird_nicknames);
				}

				nickname = streamer_personas[persona_id].empty() ? xorstr_("Unknown") : streamer_personas[persona_id].c_str();
			}

			float distance = get_distance(pos, local_pos);
			RagdollComponent* ragdoll_component = soldier->m_pRagdollComponent;

			ClientVehicleEntity* vehicle = player->GetVehicle();
			if (!g_settings.esp_draw_vehicles && vehicle)
				continue;

			float health_player = 0.f, max_health_player = 0.f, health_vehicle = 0.f, max_health_vehicle = 0.f;
			if (IsValidPtr(vehicle))
			{
				if (IsValidPtr(vehicle->m_pHealthComp) && vehicle->m_pHealthComp->m_VehicleHealth)
					health_vehicle = vehicle->m_pHealthComp->m_VehicleHealth;

				const auto data = get_vehicle_data(vehicle);
				if (IsValidPtrWithVTable(data))
				{
					if (data->m_MaxHealth)
						max_health_vehicle = data->m_MaxHealth;
				}
			}
			else
			{
				if (IsValidPtr(soldier->m_pHealthComp))
				{
					health_player = soldier->m_pHealthComp->m_Health;
					max_health_player = soldier->m_pHealthComp->m_MaxHealth;
				}
			}

			if (get_box_coords(transform, &box_coords[0]) && distance <= g_settings.esp_distance)
			{
				float box_x = box_coords[0].x;
				float box_y = box_coords[0].y;
				float box_width = box_coords[1].x - box_coords[0].x;
				float box_height = box_coords[1].y - box_coords[0].y;
				float health = vehicle ? health_vehicle : health_player;
				float max_health = vehicle ? max_health_vehicle : max_health_player;

				if (g_settings.esp_draw_box)
				{
					ImColor box_color = teammate ? g_settings.esp_teammate_color : soldier->m_Occluded ? g_settings.esp_box_color_occluded : g_settings.esp_box_color;
					m_drawing->DrawEspBox(g_settings.esp_box_style, box_coords[0].x, box_coords[0].y, box_coords[1].x - box_coords[0].x, box_coords[1].y - box_coords[0].y, box_color.Value.x, box_color.Value.y, box_color.Value.z, box_color.Value.w);
				}

				if (g_settings.esp_draw_line)
				{
					ImColor line_color = soldier->m_Occluded ? g_settings.esp_line_color_occluded : g_settings.esp_line_color;
					ImVec2 box_center = ImVec2(box_coords[0].x + box_width / 2.0f, box_coords[0].y + box_height / 2.0f);
					ImVec2 drawing_from;

					switch (g_settings.esp_draw_line_from) {
					case 0:
						drawing_from = ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y / 2.0f);
						break;
					case 1:
						drawing_from = ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f, ImGui::GetIO().DisplaySize.y);
						break;
					case 2:
						drawing_from = ImVec2(ImGui::GetIO().DisplaySize.x / 2.0f, 0);
						break;
					case 3:
						drawing_from = ImVec2(0, ImGui::GetIO().DisplaySize.y / 2.0f);
						break;
					case 4:
						drawing_from = ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y / 2.0f);
						break;
					case 5:
						drawing_from = ImVec2(0, 0);
						break;
					case 6:
						drawing_from = ImVec2(ImGui::GetIO().DisplaySize.x, 0);
						break;
					case 7:
						drawing_from = ImVec2(0, ImGui::GetIO().DisplaySize.y);
						break;
					case 8:
						drawing_from = ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
						break;
					default:
						return; // Invalid position, do nothing
					}

					m_drawing->AddLine(drawing_from, box_center, line_color, g_settings.esp_line_thickness);
				}

				if (g_settings.esp_draw_health)
				{
					ImColor hb_color(BYTE(255 - max(health - max_health / 2, 0) * (255 / (max_health / 2))), BYTE(255 - max(max_health / 2 - health, 0) * (255 / (max_health / 2))), 0, 255);
					float hb_width = max(box_width, 5.0f);
					float hb_height = max(box_width / 50.0f, 3.0f);
					float hb_width_offset = max((hb_width - box_width) / 2, 0);
					float hb_height_offset = 5.0f;
					float hb_perc_width = hb_width * (health / max_health);

					m_drawing->DrawBoxOutline(box_coords[0].x - hb_width_offset, box_coords[1].y + hb_height_offset, hb_width, hb_height, ImColor(0, 0, 0, 255));
					m_drawing->DrawFillArea(box_coords[0].x - hb_width_offset, box_coords[1].y + hb_height_offset, hb_perc_width, hb_height, hb_color);
				}

				// player->m_AttachedEntryId is 0 when the person is driving the vehicle. 
				bool allow_text = true;
				if (vehicle)
				{
					int entry_id = player->m_AttachedEntryId;
					if (entry_id != 0)
						allow_text = false;
				}

				if (allow_text)
				{
					ImColor text_color = teammate ? g_settings.esp_teammate_color : soldier->m_Occluded ? g_settings.text_color_occluded : g_settings.text_color;
					float base[2] = { box_coords[1].x + 3.5f, box_coords[0].y - 3.f };

					if (g_settings.esp_draw_name)
					{
						m_drawing->AddText(base[0], base[1], text_color, 14.f, FL_NONE, nickname);
						base[1] += g_settings.esp_text_spacing;
					}

					if (g_settings.esp_draw_distance)
					{
						m_drawing->AddText(base[0], base[1], text_color, 14.f, FL_NONE, fmt::format(xorstr_("{}m"), abs(ceil(distance))).c_str());
						base[1] += g_settings.esp_text_spacing;
					}

					if (g_settings.esp_draw_vehicle_tag && vehicle)
					{
						m_drawing->AddText(base[0], base[1], g_settings.esp_additional_tags_color, 14.f, FL_NONE, xorstr_("VEHICLE"));
						base[1] += g_settings.esp_text_spacing;
					}
				}

				if (g_settings.skeleton)
				{
					bool dots = false;
					if (g_settings.skeleton_use_dots)
					{
						if (distance <= g_settings.skeleton_dots_distance)
							dots = true;
					}

					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::Head, UpdatePoseResultData::Neck, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::Neck, UpdatePoseResultData::Spine2, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::Spine2, UpdatePoseResultData::Spine1, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::Spine1, UpdatePoseResultData::Spine, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::Neck, UpdatePoseResultData::LeftShoulder, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::LeftShoulder, UpdatePoseResultData::LeftElbowRoll, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::RightShoulder, UpdatePoseResultData::RightElbowRoll, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::LeftElbowRoll, UpdatePoseResultData::LeftHand, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::RightElbowRoll, UpdatePoseResultData::RightHand, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::Spine, UpdatePoseResultData::RightKneeRoll, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::Spine, UpdatePoseResultData::LeftKneeRoll, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::RightKneeRoll, UpdatePoseResultData::RightFoot, dots);
					draw_bone(g_settings.skeleton_color, ragdoll_component, UpdatePoseResultData::LeftKneeRoll, UpdatePoseResultData::LeftFoot, dots);		
				}
			}
		}
		if (g_globals.g_hasPredictedAimPoint && g_settings.esp_draw_aim_point)
		{			
			Vector2 aimpoint_screen_coords;
			if (world_to_screen(g_globals.g_pred_aim_point, aimpoint_screen_coords))
			{
				m_drawing->AddCircleFilled(
					ImVec2(aimpoint_screen_coords.x, aimpoint_screen_coords.y),
					6.0f,
					g_settings.esp_aim_point_color);
			}
		}
	}
}
