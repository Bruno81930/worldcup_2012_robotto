// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bhv_basic_move.h"

#include "strategy.h"

#include "bhv_basic_tackle.h"

#include "fuzzy_offensive_pos_eval-inl.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/intercept_table.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

#include "neck_offensive_intercept_neck.h"

#include <algorithm>

using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
bool
Bhv_BasicMove::execute( PlayerAgent * agent )
{
	return executeRobotto(agent);

    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicMove" );

    //-----------------------------------------------
    // tackle
    if ( Bhv_BasicTackle( 0.8, 80.0 ).execute( agent ) )
    {
        return true;
    }

    const WorldModel & wm = agent->world();
    /*--------------------------------------------------------*/
    // chase ball
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();

    if ( ! wm.existKickableTeammate()
         && ( self_min <= 3
              || ( self_min <= mate_min
                   && self_min < opp_min + 3 )
              )
         )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": intercept" );
        Body_Intercept().execute( agent );
        agent->setNeckAction( new Neck_OffensiveInterceptNeck() );

        return true;
    }

    const Vector2D target_point = Strategy::i().getPosition( wm.self().unum() );
    const double dash_power = Strategy::get_normal_dash_power( wm );

    double dist_thr = wm.ball().distFromSelf() * 0.1;
    if ( dist_thr < 1.0 ) dist_thr = 1.0;

    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicMove target=(%.1f %.1f) dist_thr=%.2f",
                  target_point.x, target_point.y,
                  dist_thr );

    agent->debugClient().addMessage( "BasicMove%.0f", dash_power );
    agent->debugClient().setTarget( target_point );
    agent->debugClient().addCircle( target_point, dist_thr );

    if ( ! Body_GoToPoint( target_point, dist_thr, dash_power
                           ).execute( agent ) )
    {
        Body_TurnToBall().execute( agent );
    }

    if ( wm.existKickableOpponent()
         && wm.ball().distFromSelf() < 18.0 )
    {
        agent->setNeckAction( new Neck_TurnToBall() );
    }
    else
    {
        agent->setNeckAction( new Neck_TurnToBallOrScan() );
    }

    return true;
}

//robottoCode
bool
Bhv_BasicMove::executeRobotto( PlayerAgent * agent )
{
    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicMove" );

    const WorldModel & wm = agent->world();

    //-----------------------------------------------
    // tackle
    double prob;

     if (Strategy::i().isMarkerType(wm.self().unum() ) ||
         wm.ball().pos().x < -30) {
       //prob = 0.83;
       prob = 0.8;
     } else {
       prob = 0.8;
     }
    // if ( Bhv_BasicTackle( 0.8, 80.0 ).execute( agent ) )
    if ( Bhv_BasicTackle( prob, 80.0 ).execute( agent ) )
    {
        return true;
    }

    if (shouldIntercept(agent) ) {
    	dlog.addText( Logger::TEAM,
					  __FILE__": intercept" );
		  Body_Intercept().execute( agent );
		  agent->setNeckAction( new Neck_OffensiveInterceptNeck() );

		  return true;
    }

    Vector2D strPos = Strategy::i().getPosition(wm.self().unum() );

    if (wm.self().unum() == 4 ||
        wm.self().unum() == 5) {
      if (strPos.x > -15) {
        strPos.x = std::min(std::max(Strategy::i().getPosition(2).x,
                                     Strategy::i().getPosition(3).x),
                            strPos.x);
      }
      if (strPos.absY() - 1 > wm.ball().pos().absY() &&
          wm.ball().pos().x > -32 &&
          wm.ball().pos().absY() > 25) {
        strPos.y = wm.ball().pos().y - 1;
      }
    }

    // super defense hack
    if (wm.self().unum() == 6) {
      strPos.x = (strPos.x + Strategy::i().getPosition(2).x) / 2;
    }
    if (wm.self().unum() == 7 ||
        wm.self().unum() == 8) {
      if (wm.ball().pos().x < -20 || wm.self().stamina() <= 4000) {
        strPos.x = (strPos.x + Strategy::i().getPosition(2).x) / 2;
      } else if (wm.ball().pos().x < 10) {
        strPos.x = (strPos.x + Strategy::i().getPosition(2).x) /1.5;
      }
    }
 
    const Vector2D target_point = getMovingPosition(agent, strPos);

    /*const*/ double dash_power = Strategy::get_normal_dash_power( wm );

    rcsc::Vector2D ballFpos = wm.ball().inertiaFinalPoint();

    double delta;

    if (wm.ball().pos().x > -38) {
      delta = 4.5;
    } else {
      delta = 3;
    }

    if (Strategy::i().isMarkerType(wm.self().unum() ) ) {
    	if (wm.ball().pos().x < 0 &&
    		wm.ball().pos().x > -45) {
    		if (wm.self().pos().x - delta > ballFpos.x) {
    			dash_power = 1000;
    		}
    	}
    }

	double dist_thr = wm.ball().distFromSelf() * 0.1;
	if ( dist_thr < 1.0 ) dist_thr = 1.0;

	dlog.addText( Logger::TEAM,
				  __FILE__": Bhv_BasicMove target=(%.1f %.1f) dist_thr=%.2f",
				  target_point.x, target_point.y,
				  dist_thr );

	agent->debugClient().addMessage( "BasicMove%.0f", dash_power );
	agent->debugClient().setTarget( target_point );
	agent->debugClient().addCircle( target_point, dist_thr );

	if ( ! Body_GoToPoint( target_point, dist_thr, dash_power
						   ).execute( agent ) )
	{
		Body_TurnToBall().execute( agent );
	}

	if ( wm.existKickableOpponent()
		 && wm.ball().distFromSelf() < 18.0 )
	{
		agent->setNeckAction( new Neck_TurnToBall() );
	}
	else
	{
		agent->setNeckAction( new Neck_TurnToBallOrScan() );
	}

	return true;
}

//robottoCode
bool Bhv_BasicMove::shouldIntercept(rcsc::PlayerAgent * agent) {
	 const WorldModel & wm = agent->world();
	/*--------------------------------------------------------*/
	// chase ball
	const int self_min      = wm.interceptTable()->selfReachCycle          ();
	const int mate_min      = wm.interceptTable()->teammateReachCycle      ();
  const int scnd_mate_min = wm.interceptTable()->secondTeammateReachCycle();
	const int opp_min       = wm.interceptTable()->opponentReachCycle      ();
  const int scnd_opp_min  = wm.interceptTable()->secondOpponentReachCycle ();

	double ballDistFromOwnGoal = wm.ball().pos().dist(Vector2D(-52, 0) );
	double ballDistFromOppGoal = wm.ball().pos().dist(Vector2D( 52, 0) );
	double selfDistFromOwnGoal = wm.self().pos().dist(Vector2D(-52, 0) );

  rcsc::Vector2D strPos   = Strategy::i().getPosition(wm.self().unum() );
  rcsc::Vector2D ballPos  = wm.ball().pos();
  rcsc::Vector2D ballFpos = wm.ball().inertiaFinalPoint();

  if (wm.self().unum() == 2 ||
      wm.self().unum() == 3) {
    if (ballFpos.absY() > 15) {
      return false;
    }
  }

  if (wm.existKickableTeammate()  == true) {
    return false;
  }

  if (self_min > scnd_mate_min) {
    return false;
  }

  if (self_min <= 3) {
    return true;
  }

  if (self_min <= mate_min &&
      self_min <  opp_min + 3) {
    return true;
  }

  if ( (wm.self().pos().dist(ballFpos) <= 3 ) &&
       (ballFpos.x > -35)) {
    return true;
  }

  if ( (wm.self().pos().dist(strPos) > 10) &&
       (wm.self().pos().dist(strPos) > ballFpos.dist(strPos) ) &&
       (self_min > mate_min) ){
    return false;
  }

  if (Strategy::i().isMarkerType(wm.self().unum() ) ) {
    if ( (ballFpos.x < -45 && ballFpos.absY() < 15)||
         (wm.ball().pos().x < -45 && wm.ball().pos().absY() < 15) ) {
      if (self_min <= mate_min) {
        return true;
      }
    }
    if ( (ballFpos.x < wm.self().pos().x ||
          wm.ball().pos().x < wm.self().pos().x) &&
        ballFpos.absY() < 20 &&
        self_min <= mate_min) {
      return true;
    }
    if (ballFpos.x > 10) {
      return false;
    }

    return false;
  } else if (Strategy::i().isMidfielderType(wm.self().unum() ) ) {
    if (ballDistFromOwnGoal < 18 &&
        self_min <= mate_min ) {
      return true;
    }
    
    return false;
  } else if (Strategy::i().isAttackerType(wm.self().unum() ) ) {
    if (self_min > mate_min) {
      return false;
    }
    if (ballFpos.x > 34) {
      return true;
    }
    return false;
  }
	
  return false;
}

//robottoCode
rcsc::Vector2D Bhv_BasicMove::getMovingPosition(rcsc::PlayerAgent * agent,
								 	 	 	 	rcsc::Vector2D strategicPosition) {
	if (shouldMoveToIntercept(agent, strategicPosition) ) {
		return getInterceptionPosition (agent, strategicPosition);
	}
	if (shouldMark(agent, strategicPosition) ) {
		return getMarkingPosition (agent, strategicPosition);
	}
	if (shouldMoveToAttack(agent, strategicPosition) ) {
		return getAttackPosition(agent, strategicPosition);
	}

	return strategicPosition;
}

//robottoCode
bool Bhv_BasicMove::shouldMoveToIntercept(rcsc::PlayerAgent * agent,
						   	   	   	   	  rcsc::Vector2D strategicPosition) {
	const WorldModel & wm = agent->world();

  rcsc::Vector2D ballFpos = wm.ball().inertiaFinalPoint();

  if (wm.self().unum() == 2 ||
      wm.self().unum() == 3) {
    if (ballFpos.absY() > 20 ||
        strategicPosition.x > -25) {
      if (wm.self().pos().x + 3 < wm.ball().pos().x) {
        return false;
      }
    }
  }

  if (wm.self().unum() == 4 ||
      wm.self().unum() == 5) {
    if (wm.ball().pos().x > -30 &&
        wm.ball().pos().absY() > 20 &&
        wm.self().pos().x - 2 > wm.ball().pos().x) {
      return false;
    }
  }

	if (wm.existKickableTeammate() ) {
		return false;
	}

  const int self_min      = wm.interceptTable()->selfReachCycle          ();
  const int mate_min      = wm.interceptTable()->teammateReachCycle      ();
  const int scnd_mate_min = wm.interceptTable()->secondTeammateReachCycle();

	const rcsc::PlayerPtrCont & matesFromBall = wm.teammatesFromBall();

	int matesCount = matesFromBall.size();

  if (self_min > scnd_mate_min) {
    return false;
  }

  if (self_min <= mate_min) {
    return true;
  }

	if (matesCount < 2) {
    return false;
	}

	if (matesFromBall[1]->pos().dist(wm.ball().pos() ) <
			    wm.self().pos().dist(wm.ball().pos() ) ) {
		return false;
	}

	if (matesFromBall[0]->pos().dist(wm.ball().pos() ) >
	            wm.self().pos().dist(wm.ball().pos() ) ) {
		return true;
	}

  /*
	if (matesFromBall[0]->pos().x > wm.ball().pos().x &&
				wm.self().pos().x <= wm.ball().pos().x ) {
		return true;
	}
  */

	return false;
}

//robottoCode
rcsc::Vector2D Bhv_BasicMove::getInterceptionPosition (rcsc::PlayerAgent * agent,
													   rcsc::Vector2D strategicPosition) {
	const WorldModel & wm = agent->world();

  rcsc::Vector2D ballFpos = wm.ball().inertiaFinalPoint();

  /*
  if (wm.self().unum() == 2 || wm.self().unum() == 3) {
    if (wm.self().pos().x > - 43) {
      if (wm.self().pos().x <= std::min(wm.ball().pos().x, ballFpos.x) ) {
        return rcsc::Vector2D(std::min(wm.ball().pos().x, ballFpos.x) - 3,
                              wm.self().pos().y);
      }
    }
  }
  */

	if (wm.self().pos().x > wm.ball().pos().x) {
		if (wm.ball().pos().absY() < 20) {
			return ballFpos - rcsc::Vector2D(2, 0);
		} else {
      if (strategicPosition.absY() < ballFpos.absY()) {
        return rcsc::Vector2D (ballFpos.x - 0.5, strategicPosition.y);
      } else {
        return ballFpos - rcsc::Vector2D(2, 0);
      }
		}
	}

  rcsc::Vector2D ballPos;

  if (ballFpos.dist(wm.ball().pos() ) < 5) {
    ballPos = wm.ball().pos();
  } else if (ballFpos.x < wm.ball().pos().x) {
    ballPos = ballFpos;
  } else {
    ballPos = wm.ball().pos();
  }

  int factor = ballPos.y < 0? 1 : -1;

	if (ballPos.absY() > 15) {
    if (strategicPosition.x < ballPos.x) {
      if (Strategy::i().isMarkerType(wm.self().unum() ) &&
          strategicPosition.dist(ballPos) > 8) {
        return rcsc::Vector2D(strategicPosition.x, ballPos.y + factor);
      }
      return ballPos - rcsc::Vector2D(1, -factor);
    } else {
      return ballPos - rcsc::Vector2D(1, -factor);
    }
	}

  if (strategicPosition.x < ballPos.x) {
    if (Strategy::i().isMarkerType(wm.self().unum() ) &&
        strategicPosition.dist(ballPos) > 8) {
      return rcsc::Vector2D(strategicPosition.x, ballPos.y + factor);
    }
    return ballPos - rcsc::Vector2D(1, 0);
  } else {
    return ballPos - rcsc::Vector2D(1, 0);
  }
}

//robottoCode
bool Bhv_BasicMove::shouldMark(rcsc::PlayerAgent * agent,
				rcsc::Vector2D strategicPosition) {
  const WorldModel & wm = agent->world();
  
  if (wm.self().unum() > 5) {
    if (wm.existKickableTeammate() == true) {
      return false;
    }
  }

	const int self_min = wm.interceptTable()->selfReachCycle();
	const int mate_min = wm.interceptTable()->teammateReachCycle();
	const int  opp_min = wm.interceptTable()->opponentReachCycle();

  rcsc::Vector2D ballFpos = wm.ball().inertiaFinalPoint();

	if (! Strategy::i().isMarkerType(wm.self().unum() ) ) {
    if (Strategy::i().isMidfielderType(wm.self().unum() ) &&
        (wm.ball().pos().x < -25 ||
         ballFpos.x < -25) ) {
      return true;
    } else {
		  // return false;
    }
	}

	if (opp_min <= self_min &&
		  opp_min <= mate_min &&
      strategicPosition.x < -25 &&
      !Strategy::i().isMarkerType(wm.self().unum() ) ) {
		return true;
	}

  if (Strategy::i().isMarkerType(wm.self().unum() ) &&
      strategicPosition.x < -10) {
    return true;
  }

  /*
	if (wm.ball().pos().x < 15) {
		return true;
	}
  */

	return false;
}

//robottoCode
Vector2D Bhv_BasicMove::getMarkingPosition(PlayerAgent * agent,
										   Vector2D strategicPosition) {
  // static const int thr = 3;
  static const int thr = 4;
	
  const WorldModel & wm = agent->world();

	double distNearestOpp = wm.getDistOpponentNearestTo(strategicPosition ,thr);

  rcsc::Vector2D ballFpos = wm.ball().inertiaFinalPoint();

	if ( (strategicPosition.dist(wm.ball().pos() ) <  distNearestOpp + 3) ||
       (strategicPosition.dist(ballFpos) <  distNearestOpp + 3) ) {
		// return strategicPosition;
	}

  int distThr;

  if (Strategy::i().isMidfielderType(wm.self().unum() ) ) {
    if (strategicPosition.x > -28) {
      distThr = 7;
    } else {
      distThr = 7;
    }
  } else if (Strategy::i().isMarkerType(wm.self().unum() ) ) {
    if (strategicPosition.x > - 30) {
      distThr = 5;
    } else {
      distThr = 7;
    }
  } else {
    distThr = 7;
  }

	if (distNearestOpp < distThr) {
		const PlayerObject* opp = 
      wm.getOpponentNearestTo(strategicPosition, thr, &distNearestOpp);

		double dist = 0;

		const PlayerObject* opp2 =
      wm.getOpponentNearestTo(wm.ball().pos(), thr, &dist);
      // wm.getOpponentNearestTo(ballFpos, thr, &dist);

		if (opp == opp2) {
			// return strategicPosition;
		}

		double distNearestMate = 1000;
		const PlayerObject * nearestTeammate =
      wm.getTeammateNearestTo(opp, thr, &distNearestMate);

		if (distNearestMate < distNearestOpp) {
			if (nearestTeammate->pos().x <= opp->pos().x) {
        if (distNearestMate > 1.5 &&
            opp->pos().dist(Vector2D(-52, 0) ) < 15 ) {
        } else {
          return strategicPosition;
        }
			}
		}

    int factor = wm.ball().pos().y >= 0? 1 : -1;
		if (opp->pos().x <= wm.ourDefenseLineX() && wm.ourDefenseLineX() > -30) {
			// return rcsc::Vector2D (wm.ourDefenseLineX() - 2, opp->pos().y /*+ factor*/);
      if (opp->pos().x > wm.ourDefenseLineX() - 1.5) {
        return rcsc::Vector2D (strategicPosition.x - 2, opp->pos().y);
      } else {
        return rcsc::Vector2D (strategicPosition.x, opp->pos().y);
      }
		}

    double xD;

    if (wm.self().pos().dist(rcsc::Vector2D(-52, 0) ) > 30) {
      xD = 3;
    } else if (wm.self().pos().dist(rcsc::Vector2D(-52, 0) ) > 25) {
      xD = 2;
    } else if (wm.self().pos().dist(rcsc::Vector2D(-52, 0) ) > 18){
      xD = 0.7;
    } else {
      xD = 0.3;
    }

		if (wm.self().pos().dist(rcsc::Vector2D(-52, 0) ) >=
		         opp->pos().dist(rcsc::Vector2D(-52, 0) ) ) {
      return opp->pos() - rcsc::Vector2D(xD, 0);
		}

		if (wm.self().pos().x > opp->pos().x) {
			if (abs(opp->pos().y) < 20) {
				// return opp->pos() - rcsc::Vector2D(1, 0);
        return opp->pos() - rcsc::Vector2D(xD, 0);
			} else {
				// return rcsc::Vector2D (opp->pos().x - 0.4, strategicPosition.y);
        return opp->pos() - rcsc::Vector2D(xD, 0);
			}
		}

		if (opp->pos().dist(rcsc::Vector2D(-52, 0) ) < 18) {
			return opp->pos() - rcsc::Vector2D(0.25, 0);
		}

		// return rcsc::Vector2D(strategicPosition.x, opp->pos().y);
    return opp->pos() - rcsc::Vector2D(xD, 0);
	}

	return strategicPosition;
}

//robottoCode
bool Bhv_BasicMove::shouldMoveToAttack(rcsc::PlayerAgent * agent,
									   rcsc::Vector2D strategicPosition) {
	const WorldModel & wm = agent->world();

	if (! wm.existKickableTeammate() ) {
		return false;
	}

  if (wm.self().stamina() < 5000) {
    return false;
  }

  double ballThr, selfThr;

  if (wm.self().unum() > 8) {
    ballThr = 20;
    selfThr = 25;
  } else {
    ballThr = 25;
    selfThr = 30;
  }

	if (wm.ball().pos().x < ballThr) {
		return false;
	}

	if (wm.self().pos().x < selfThr) {
		return false;
	}

	if (wm.self().unum() < 7) {
		return false;
	}

	const rcsc::PlayerPtrCont & matesFromBall = wm.teammatesFromBall();

	int matesCount = matesFromBall.size();

	// if (matesCount < 3) {
  if (matesCount < 4) {
		return false;
	}

  // if (matesFromBall[2]->pos().dist(wm.ball().pos() ) <
  if (wm.self().unum() > 8) {
	  if (matesFromBall[3]->pos().dist(wm.ball().pos() ) <
		  	wm.self().pos().dist(wm.ball().pos() ) ) {
		  return false;
	  }
  } else {
    if (matesFromBall[2]->pos().dist(wm.ball().pos() ) <
        wm.self().pos().dist(wm.ball().pos() ) ) { 
      return false;
    }
  }

	return true;
}

//robottoCode
rcsc::Vector2D Bhv_BasicMove::getAttackPosition(rcsc::PlayerAgent * agent,
								 	 	 	 	rcsc::Vector2D strategicPosition) {
	return getFuzzyAttackPosition(agent, strategicPosition);

	const WorldModel & wm = agent->world();

	int delta = 7;

	int yFactor;

	if (abs(wm.self().pos().y) - abs(wm.ball().pos().y) < 2) {
		yFactor = 0;
	} else if (wm.self().pos().y < wm.ball().pos().y) {
		yFactor = 1;
	} else {
		yFactor = -1;
	}

	if (wm.self().unum() == 11) {
		yFactor *= -1;
	}

	rcsc::Vector2D pos1 = rcsc::Vector2D(strategicPosition + rcsc::Vector2D(-delta, 0));

	rcsc::Vector2D pos2 = rcsc::Vector2D(strategicPosition + rcsc::Vector2D(0, yFactor * delta));

	if (wm.self().pos().dist(pos1) < 1.5) {
		return pos2;
	} else if (wm.self().pos().dist(pos2) < 1.5) {
		return pos1;
	}

	return wm.self().pos().dist(pos1) < wm.self().pos().dist(pos2)? pos1 : pos2;
}

//robottoCode
rcsc::Vector2D Bhv_BasicMove::getFuzzyAttackPosition(rcsc::PlayerAgent * agent,
								 	 	 	 	rcsc::Vector2D strategicPosition) {
	const WorldModel & wm = agent->world();

	double maxDisplacement = 6;
	double step            = 2;

	int yFactor;

  if (wm.self().pos().y < 0) {
    yFactor = 1;
  } else {
    yFactor = -1;
  }

  int xFactor;

  if (wm.self().unum() > 8) {
    if (wm.self().pos().x > 42) {
      xFactor = 1;
    } else {
      xFactor = -1;
    }
  } else {
    if (wm.self().pos().x > 30) {
      xFactor = -1;
    } else {
      xFactor = 1;
    }
  }

  if (wm.self().unum() > 8) {
    maxDisplacement = 10;
  }

  /*
	if (wm.self().pos().y < wm.ball().pos().y) {
		yFactor = 1;
	} else {
		yFactor = -1;
	}
  */

  /*
	if (wm.self().unum() == 11) {
		if (wm.ball().pos().y < 0) {
			yFactor = 1;
		} else {
			-1;
		}
	}
  */

	rcsc::Vector2D bestAttackPos,
	               currAttackPos;

	bestAttackPos = strategicPosition;

	double distBallPos,
	       distOppPos,
		   distBallLineOpp,
		   distCurrPos,
		   distOppGoalPos;

	double bestPosEval,
	       currPosEval;

	bestPosEval = -10;

	for (double i = 0; i < maxDisplacement; i += step) {
		for (double j = 0; j < 3; j ++) {
			if (j == 0) {
				currAttackPos = strategicPosition + rcsc::Vector2D(-i * xFactor, 0);
			} else if (j == 1) {
				currAttackPos = strategicPosition + rcsc::Vector2D( 0, yFactor * i);
			} else if (j == 2) {
				currAttackPos = strategicPosition + rcsc::Vector2D(-i * xFactor, yFactor * i);
			}

			distBallPos = currAttackPos.dist(wm.ball().pos() );
			distOppPos  = wm.getDistOpponentNearestTo(currAttackPos, 5);

			rcsc::Segment2D* ballLine = new rcsc::Segment2D (currAttackPos, wm.ball().pos());

			const PlayerPtrCont & opps = wm.opponentsFromBall();

			if (opps.size() < 2) {
				distBallLineOpp = 100;
			} else {
				distBallLineOpp = std::min(ballLine->dist(opps[0]->pos() ),
						                   ballLine->dist(opps[1]->pos() ) );
			}

			delete ballLine;

			distCurrPos = currAttackPos.dist(wm.self().pos() );

			distOppGoalPos = currAttackPos.dist(rcsc::Vector2D(45, 0) );

			currPosEval = FuzzyOffensivePosEval::getOffensivePosEval(distBallPos,
					                                                 distOppPos,
					                                                 distBallLineOpp,
					                                                 distCurrPos,
					                                                 distOppGoalPos);

			if (currPosEval > bestPosEval) {
				bestPosEval = currPosEval;

				bestAttackPos = currAttackPos;
			}
		}
	}

	return bestAttackPos;
}
