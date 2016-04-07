//////////////////////////////////////////////////////////////////////////////
// This file is part of the Journey MMORPG client                           //
// Copyright � 2015 Daniel Allendorf                                        //
//                                                                          //
// This program is free software: you can redistribute it and/or modify     //
// it under the terms of the GNU Affero General Public License as           //
// published by the Free Software Foundation, either version 3 of the       //
// License, or (at your option) any later version.                          //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU Affero General Public License for more details.                      //
//                                                                          //
// You should have received a copy of the GNU Affero General Public License //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "MapObjects.h"
#include "Mob.h"

#include "Gameplay\Combat\Attack.h"
#include "Gameplay\Combat\SpecialMove.h"
#include "Gameplay\Spawn.h"

#include <list>

namespace Gameplay
{
	class MapMobs : public MapObjects
	{
	public:
		void draw(int8_t layer, Point<int16_t> viewpos, float alpha) const override;
		void update(const Physics& physics) override;

		void sendspawn(const MobSpawn& spawn);
		void movemob(int32_t oid, Point<int16_t> start, const Movement& movement);
		void killmob(int32_t oid, int8_t effect);
		void sendmobhp(int32_t oid, int8_t percent, uint16_t playerlevel);
		void setcontrol(int32_t oid, bool control);

		AttackResult sendattack(const Attack& attack);
		void showresult(const Char& user, const SpecialMove& skill, const AttackResult& result);

		Optional<Mob> getmob(int32_t oid);
		Optional<const Mob> getmob(int32_t oid) const;

	private:
		template <typename T>
		using list = std::list<T>;

		class DamageEffect
		{
		public:
			DamageEffect(const SpecialMove& m, AttackUser u, DamageNumber n, bool tl, int32_t dm, int32_t t, uint16_t d)
				: move(m), user(u), number(n), toleft(tl), damage(dm), target(t), delay(d) {}

			void apply(Mob& target) const
			{
				move.applyhiteffects(user, target);

				target.applydamage(damage, toleft);
			}

			bool expired() const
			{
				return delay <= Constants::TIMESTEP;
			}

			bool update()
			{
				if (delay <= Constants::TIMESTEP)
				{
					return true;
				}
				else
				{
					delay -= Constants::TIMESTEP;
					return false;
				}
			}

			int32_t gettarget() const
			{
				return target;
			}

			DamageNumber getnumber() const
			{
				return number;
			}

		private:
			const DamageEffect& operator =(const DamageEffect&) = delete;

			const SpecialMove& move;
			AttackUser user;
			DamageNumber number;
			int32_t damage;
			bool toleft;
			int32_t target;
			uint16_t delay;
		};

		class BulletEffect
		{
		public:
			BulletEffect(Bullet b, Point<int16_t> t, DamageEffect de)
				: bullet(b), target(t), damageeffect(de) {

				fired = false;
			}

			void draw(Point<int16_t> viewpos, float alpha) const
			{
				if (fired)
					bullet.draw(viewpos, alpha);
			}

			bool update()
			{
				if (fired)
				{
					return bullet.update(target);
				}
				else
				{
					bool expired = damageeffect.update();
					if (expired)
					{
						fired = true;
						return bullet.settarget(target);
					}
					else
					{
						return false;
					}
				}
			}

			bool update(Point<int16_t> newtarget)
			{
				target = newtarget;

				return update();
			}

			int32_t gettarget() const
			{
				return damageeffect.gettarget();
			}

			const DamageEffect& geteffect() const
			{
				return damageeffect;
			}

		private:
			const BulletEffect& operator =(const BulletEffect&) = delete;

			Bullet bullet;
			Point<int16_t> target;
			DamageEffect damageeffect;
			bool fired;
		};

		void applyeffect(const DamageEffect& effect);
		vector<int32_t> findclosest(rectangle2d<int16_t> range, Point<int16_t> origin, uint8_t mobcount) const;

		list<DamageNumber> damagenumbers;
		list<DamageEffect> damageeffects;
		list<BulletEffect> bulleteffects;
	};
}

