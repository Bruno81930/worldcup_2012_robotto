//robottoCode

#ifndef BHV_SET_PLAY_GETRCVPOS_H
#define BHV_SET_PLAY_GETRCVPOS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "strategy.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/say_message_builder.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/circle_2d.h>
#include <rcsc/math_util.h>

inline
rcsc::Vector2D getSetPlayReceiverPosition(rcsc::PlayerAgent* agent,
										  rcsc::Vector2D target_point) {
	const rcsc::WorldModel & wm = agent->world();

	const rcsc::PlayerPtrCont & teammates = wm.teammatesFromBall();

	if (teammates.size() >= 2) {
		if (target_point.dist(wm.ball().pos()) <
			teammates[1]->pos().dist(wm.ball().pos())) {
			double x, y;
			if (wm.ball().pos().x < 40) {
				x = wm.ball().pos().x + 6;
			} else {
				x = wm.ball().pos().x - 4;
			}

			if (wm.ball().pos().y > 0) {
				y = wm.ball().pos().y - 5;
			} else {
				y = wm.ball().pos().y + 5;
			}

			return rcsc::Vector2D(x, y);
		}
	}

	return target_point;
}

#endif
