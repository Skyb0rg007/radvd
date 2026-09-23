/*
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996-2000 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from https://github.com/radvd-project/radvd/issues
 *
 */

#include "config.h"
#include "radvd.h"

struct timespec next_timespec(double next)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	ts.tv_sec += (int)next;
	ts.tv_nsec += 1000000000ULL * (next - (int)next);
	return ts;
}

int64_t timespecdiff(struct timespec const *a, struct timespec const *b)
{
	int64_t msec;
	msec = ((int64_t)a->tv_sec - b->tv_sec) * 1000;
	msec += ((int64_t)a->tv_nsec - b->tv_nsec) / (1000 * 1000);
	return msec;
}

/*
 * Token bucket rate limiter. Allows bursts of up to `burst` events, refilled
 * at `rate` events per second. Returns 1 and consumes a token if the event is
 * allowed, 0 otherwise.
 */
int ratelimit_allow(struct ratelimit *rl, struct timespec const *now, double rate, double burst)
{
	double elapsed = (now->tv_sec - rl->last.tv_sec) + (now->tv_nsec - rl->last.tv_nsec) / 1000000000.0;
	rl->last = *now;

	if (elapsed > 0)
		rl->tokens = min(burst, rl->tokens + elapsed * rate);

	if (rl->tokens < 1.0)
		return 0;

	rl->tokens -= 1.0;
	return 1;
}

/* Returns when the next time should expire in milliseconds. */
uint64_t next_time_msec(struct Interface const *iface)
{
	struct timespec ts;
	int64_t diff_ms;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	diff_ms = timespecdiff(&iface->times.next_multicast, &ts);
	if (diff_ms <= 0)
		return 0;
	return (uint64_t)diff_ms;
}
