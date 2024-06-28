#include "../../Utilities/other.h"
#include "../../settings.h"
#include "autofire.h"

using namespace big;

namespace big
{
    Timer fireTimer;
    bool canFire = true;
}

namespace plugins
{
    void autofire()
    {
        if (!g_settings.autofire) return;

        const auto game_context = ClientGameContext::GetInstance();
        if (!game_context) return;

        const auto player_manager = game_context->m_pPlayerManager;
        if (!player_manager) return;

        const auto local_player = player_manager->m_pLocalPlayer;
        if (!local_player) return;

        const auto local_soldier = local_player->GetSoldier();
        if (!local_soldier) return;

        const auto weapon = WeaponFiring::GetInstance();
        if (!weapon) return;

        if (!local_soldier->IsAlive()) return;

        const auto weapon_component = local_soldier->m_pWeaponComponent;
        if (!weapon_component) return;

        const auto border_input_node = BorderInputNode::GetInstance();
        if (!border_input_node) return;

        const auto mouse = border_input_node->m_pMouse;
        if (!mouse) return;

        const auto device = mouse->m_pDevice;
        if (!device) return;

        auto current_weapon_class = WeaponFiring::GetInstance()->GetWeaponClass();
        if (current_weapon_class != WeaponClass::Smg &&
            current_weapon_class != WeaponClass::Sniper &&
            current_weapon_class != WeaponClass::Shotgun)
            return;

        if (weapon->m_JustShot)
        {
            device->m_Buffer.buttons[0] = false;
        }
        else if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
        {
            const auto input_cache = border_input_node->m_InputCache;
            if (!input_cache) return;

            const auto input = input_cache->m_Event;
            if (!input) return;

            if (weapon->m_FirstSlotBullets <= 0)
            {
                device->m_Buffer.buttons[0] = false;
                device->m_Buffer.buttons[1] = false;
                input[ConceptAltFire] = 0.f;
                input[ConceptZoom] = 0.f;
                input[ConceptReload] = 1.f;
            }
            else
            {
                input[ConceptReload] = 0.f;

                if (canFire) {
                    input[ConceptFire] = 1.f;
                    canFire = false;

                    fireTimer.start(generate_random_int(90, 140), []() {
                        canFire = true;
                        });
                }
                else {
                    input[ConceptFire] = 0.f;
                }
            }
        }
    }
}