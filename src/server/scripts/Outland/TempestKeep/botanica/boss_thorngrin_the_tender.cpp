/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "the_botanica.h"

enum Says
{
    SAY_AGGRO                   = 0,
    SAY_20_PERCENT_HP           = 1,
    SAY_KILL                    = 2,
    SAY_CAST_SACRIFICE          = 3,
    SAY_50_PERCENT_HP           = 4,
    SAY_CAST_HELLFIRE           = 5,
    SAY_DEATH                   = 6,
    EMOTE_ENRAGE                = 7,
    SAY_INTRO                   = 8
};

enum Spells
{
    SPELL_SACRIFICE             = 34661,
    SPELL_HELLFIRE              = 34659,
    SPELL_ENRAGE                = 34670
};

enum Events
{
    EVENT_SACRIFICE             = 1,
    EVENT_HELLFIRE              = 2,
    EVENT_ENRAGE                = 3,
    EVENT_HEALTH_CHECK_50       = 4,
    EVENT_HEALTH_CHECK_20       = 5
};

class boss_thorngrin_the_tender : public CreatureScript
{
public:
    boss_thorngrin_the_tender() : CreatureScript("thorngrin_the_tender") { }

    struct boss_thorngrin_the_tenderAI : public BossAI
    {
        boss_thorngrin_the_tenderAI(Creature* creature) : BossAI(creature, DATA_THORNGRIN_THE_TENDER)
        {
            me->m_SightDistance = 100.0f;
            _intro = false;
        }

        void Reset() override
        {
            _Reset();
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (!_intro && who->GetTypeId() == TYPEID_PLAYER)
            {
                _intro = true;
                Talk(SAY_INTRO);
            }
            BossAI::MoveInLineOfSight(who);
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            _JustEngagedWith();
            Talk(SAY_AGGRO);
            events.ScheduleEvent(EVENT_SACRIFICE, 6000);
            events.ScheduleEvent(EVENT_HELLFIRE, 18000);
            events.ScheduleEvent(EVENT_ENRAGE, 15000);
            events.ScheduleEvent(EVENT_HEALTH_CHECK_50, 500);
            events.ScheduleEvent(EVENT_HEALTH_CHECK_20, 500);
        }

        void KilledUnit(Unit* victim) override
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_KILL);
        }

        void JustDied(Unit* /*killer*/) override
        {
            _JustDied();
            Talk(SAY_DEATH);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EVENT_SACRIFICE:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 1, 0.0f, true))
                    {
                        Talk(SAY_CAST_SACRIFICE);
                        me->CastSpell(target, SPELL_SACRIFICE, false);
                    }
                    events.ScheduleEvent(EVENT_SACRIFICE, 30000);
                    break;
                case EVENT_HELLFIRE:
                    if (roll_chance_i(50))
                        Talk(SAY_CAST_HELLFIRE);
                    me->CastSpell(me, SPELL_HELLFIRE, false);
                    events.ScheduleEvent(EVENT_HELLFIRE, 22000);
                    break;
                case EVENT_ENRAGE:
                    Talk(EMOTE_ENRAGE);
                    me->CastSpell(me, SPELL_ENRAGE, false);
                    events.ScheduleEvent(EVENT_ENRAGE, 30000);
                    break;
                case EVENT_HEALTH_CHECK_50:
                    if (me->HealthBelowPct(50))
                    {
                        Talk(SAY_50_PERCENT_HP);
                        break;
                    }
                    events.ScheduleEvent(EVENT_HEALTH_CHECK_50, 500);
                    break;
                case EVENT_HEALTH_CHECK_20:
                    if (me->HealthBelowPct(20))
                    {
                        Talk(SAY_20_PERCENT_HP);
                        break;
                    }
                    events.ScheduleEvent(EVENT_HEALTH_CHECK_20, 500);
                    break;
            }

            DoMeleeAttackIfReady();
        }

    private:
        bool _intro;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetTheBotanicaAI<boss_thorngrin_the_tenderAI>(creature);
    }
};

void AddSC_boss_thorngrin_the_tender()
{
    new boss_thorngrin_the_tender();
}
