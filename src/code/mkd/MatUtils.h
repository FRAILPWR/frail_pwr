#pragma once

#include "pch.h"
#include "Game.h"
#include "Rocket.h"
#include "Character.h"

inline double sqr(double x)
{
	return x*x;
}

inline mkVec3 PredictFireball(const mkVec3 &myPos, const mkVec3 &enemyPos, const mkVec3 &enemySpeed, double fireballSpeed)
{
	static mkVec3 lastPos;

	mkVec3 D = enemyPos - myPos;
	double D2 = D.dotProduct(D);

	mkVec3 V = enemySpeed;
		//(enemy->getWorldPosition() - lastPos) / g_game->getTimeDelta();

	double V2 = V.dotProduct(V);
	const double vf2 = sqr(fireballSpeed);
	double delta = sqr(D.dotProduct(V) * 2) - 4 * (V2 - vf2) * D2;

	if (delta <= 0)
		return enemyPos;

	double sqrtDelta = sqrt(delta);

	double dt1 = (D.dotProduct(V) * 2 + sqrtDelta) / (2 * (V2 - vf2));
	double dt2 = (D.dotProduct(V) * 2 - sqrtDelta) / (2 * (V2 - vf2));

	double dt = 0;

	if (dt1 > 0 && dt2 > 0)
		dt = std::min(dt1, dt2);
	else if (dt1 > 0)
		dt = dt1;
	else if (dt2 > 0)
		dt = dt2;

	if (dt == 0)
		return enemyPos;

	mkVec3 dvf = D + (V * dt);

	return myPos + dvf;
}