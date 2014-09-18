#include "ScriptMgr.h"
#include "SpellScript.h"
#include "ThreatManager.h"
#include "Player.h"

enum Spells
{
	SPELL_TRIGGERED = 18282
}

class speed_boost : public SpellScriptLoader
{
	public:
		speed_boost() : SpellScriptLoader("speed_boost") { }

		class speed_boostSpellScript : public SpellScript
		{
			PrepareSpellScript(speed_boostSpellScript);

			bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                // check if spellid 70522 exists in dbc, we will trigger it later
                if (!sSpellMgr->GetSpellInfo(SPELL_TRIGGERED))
                    return false;
                return true;
            }

            // function called just after script is added to spell
            // we initialize local variables if needed
            bool Load() override
            {

            	if (GetCaster()->GetTypeID() != TYPEID_PLAYER)
            		return false;
            	return true;
                // localVariable = "we're using local variable";
                // localVariable2 = new char();
                // return true;
                // return false - script will be immediately removed from the spell
                // for example - we don't want this script to be executed on a creature
                // if (GetCaster()->GetTypeID() != TYPEID_PLAYER)
                //     return false;
            }

            // function called just before script delete
            // we free allocated memory
            void Unload() override
            {
             
            }

            void HandleBeforeCast()
            {
                // this hook is executed before anything about casting the spell is done
                // after this hook is executed all the machinery starts
                TC_LOG_INFO("misc", "Caster just finished preparing the spell (cast bar has expired)");
            }

            void HandleOnCast()
            {
                // cast is validated and spell targets are selected at this moment
                // this is a last place when the spell can be safely interrupted
                TC_LOG_INFO("misc", "Spell is about to do take reagents, power, launch missile, do visuals and instant spell effects");
            }

            void HandleAfterCast()
            {
                TC_LOG_INFO("misc", "All immediate Actions for the spell are finished now");
                
                // this is a safe for triggering additional effects for a spell without interfering
                // with visuals or with other effects of the spell
                GetCaster()->CastSpell(GetCaster(), SPELL_TRIGGERED, true);
                GetCaster()->setSpeed(MOVE_RUN, 5, true);
            }

            SpellCastResult CheckRequirement()
            {
                // in this hook you can add additional requirements for spell caster (and throw a client error if reqs're not passed)
                // in this case we're disallowing to select non-player as a target of the spell
                //if (!GetExplTargetUnit() || GetExplTargetUnit()->ToPlayer())
                    //return SPELL_FAILED_BAD_TARGETS;
            	if (GetCaster()->isThreatListEmpty())
                	return SPELL_CAST_OK;
                return SPELL_FAILED_BAD_TARGETS;

            }


            void HandleDummyLaunch(SpellEffIndex /*effIndex*/)
            {
                TC_LOG_INFO("misc", "Spell %u with SPELL_EFFECT_DUMMY is just launched!", GetSpellInfo()->Id);
            }

            void HandleDummyLaunchTarget(SpellEffIndex /*effIndex*/)
            {
                ObjectGuid targetGUID;
                if (Unit* unitTarget = GetHitUnit())
                    targetGUID = unitTarget->GetGUID();
                // we're handling SPELL_EFFECT_DUMMY in effIndex 0 here
                TC_LOG_INFO("misc", "Spell %u with SPELL_EFFECT_DUMMY is just launched at it's target: " UI64FMTD "!", GetSpellInfo()->Id, targetGUID.GetRawValue());
            }

            void HandleDummyHit(SpellEffIndex /*effIndex*/)
            {
                TC_LOG_INFO("misc", "Spell %u with SPELL_EFFECT_DUMMY has hit!", GetSpellInfo()->Id);
            }

            void HandleDummyHitTarget(SpellEffIndex /*effIndex*/)
            {
                TC_LOG_INFO("misc", "SPELL_EFFECT_DUMMY is hits it's target!");
                // make caster cast a spell on a unit target of effect
                if (Unit* target = GetHitUnit())
                    GetCaster()->CastSpell(GetCaster(), SPELL_TRIGGERED, true);
            }

            void HandleBeforeHit()
            {
                TC_LOG_INFO("misc", "Spell is about to hit target!");
            }

            void HandleOnHit()
            {
                TC_LOG_INFO("misc", "Spell just hit target!");
            }

            void HandleAfterHit()
            {
                TC_LOG_INFO("misc", "Spell just finished hitting target!");
            }

            void FilterTargets(std::list<Unit*>& /*targetList*/)
            {
                // usually you want this call for Area Target spells
                TC_LOG_INFO("misc", "Spell is about to add targets from targetList to final targets!");
            }

            // register functions used in spell script - names of these functions do not matter
            void Register() override
            {
                // we're registering our functions here
                //BeforeCast += SpellCastFn(spell_ex_5581SpellScript::HandleBeforeCast);
                //OnCast += SpellCastFn(spell_ex_5581SpellScript::HandleOnCast);
                AfterCast += SpellCastFn(speeed_boostSpellScript::HandleAfterCast);
                OnCheckCast += SpellCheckCastFn(speeed_boostSpellScript::CheckRequirement);
                // function HandleDummy will be called when spell is launched, independant from targets selected for spell, just before default effect 0 launch handler
                //OnEffectLaunch += SpellEffectFn(spell_ex_5581SpellScript::HandleDummyLaunch, EFFECT_0, SPELL_EFFECT_DUMMY);
                // function HandleDummy will be called when spell is launched at target, just before default effect 0 launch at target handler
                //OnEffectLaunchTarget += SpellEffectFn(spell_ex_5581SpellScript::HandleDummyLaunchTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
                // function HandleDummy will be called when spell hits it's destination, independant from targets selected for spell, just before default effect 0 hit handler
                //OnEffectHit += SpellEffectFn(spell_ex_5581SpellScript::HandleDummyHit, EFFECT_0, SPELL_EFFECT_DUMMY);
                // function HandleDummy will be called when unit is hit by spell, just before default effect 0 hit target handler
                OnEffectHitTarget += SpellEffectFn(speeed_boostSpellScript::HandleDummyHitTarget, EFFECT_0, SPELL_EFFECT_DUMMY);
                // this will prompt an error on startup because effect 0 of spell 49375 is set to SPELL_EFFECT_DUMMY, not SPELL_EFFECT_APPLY_AURA
                //OnEffectHitTarget += SpellEffectFn(spell_gen_49375SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
                // this will make HandleDummy function to be called on first != 0 effect of spell 49375
                //OnEffectHitTarget += SpellEffectFn(spell_gen_49375SpellScript::HandleDummy, EFFECT_FIRST_FOUND, SPELL_EFFECT_ANY);
                // this will make HandleDummy function to be called on all != 0 effect of spell 49375
                //OnEffectHitTarget += SpellEffectFn(spell_gen_49375SpellScript::HandleDummy, EFFECT_ALL, SPELL_EFFECT_ANY);
                // bind handler to BeforeHit event of the spell
                //BeforeHit += SpellHitFn(spell_ex_5581SpellScript::HandleBeforeHit);
                // bind handler to OnHit event of the spell
                OnHit += SpellHitFn(speeed_boostSpellScript::HandleOnHit);
                // bind handler to AfterHit event of the spell
                AfterHit += SpellHitFn(speeed_boostSpellScript::HandleAfterHit);
                // bind handler to OnUnitTargetSelect event of the spell
                //OnUnitTargetSelect += SpellUnitTargetFn(spell_ex_5581SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_CASTER);
            }
        };

        // function which creates SpellScript
        SpellScript* GetSpellScript() const override
        {
            return new speed_boostSpellScript();
        }
};