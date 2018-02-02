/* 
 * Originally written in 2014-2015 by Sebastiano Vigna (vigna@acm.org)
 *
 * To the extent possible under law, the author has dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * See <http://creativecommons.org/publicdomain/zero/1.0/>. 
 */

#ifndef H_SLIB_UTIL_RANDOM_H
#define H_SLIB_UTIL_RANDOM_H

#include <mutex>

namespace slib {

/** 
 * xorshift1024* - this is a fast, top-quality generator, using 1024 bits of state.
 *
 * Note that the three lowest bits of this generator are LSFRs, and thus
 * they are slightly less random than the other bits.
 *
 * The state is seeded based on a 64-bit seed, using a splitmix64 generator.
 */
class Random {
private:
	uint64_t _s[16];		// state
	int _p;
public:
	static uint64_t splitmix64_next(uint64_t& x) {
		uint64_t z = (x += UINT64_C(0x9E3779B97F4A7C15));
		z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
		z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
		return z ^ (z >> 31);
	}
public:
	Random(uint64_t seed) {
		uint64_t x = seed;
		for (int i = 0; i < 16; i++)
			_s[i] = splitmix64_next(x);
		_p = 0;
	}

	uint64_t nextULong() {
		const uint64_t s0 = _s[_p];
		uint64_t s1 = _s[_p = (_p + 1) & 15];
		s1 ^= s1 << 31; // a
		_s[_p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30); // b,c
		return _s[_p] * UINT64_C(1181783497276652981);
	}

	uint32_t nextUInt() {
		return (uint32_t)(nextULong() >> 32);
	}
};

class StaticRandom {
private:
	static Random _random;
	static std::mutex _lock;
public:
	static uint64_t nextULong() {
		std::lock_guard<std::mutex> aLock(_lock);
		return _random.nextULong();
	}

	static uint32_t nextUInt() {
		std::lock_guard<std::mutex> aLock(_lock);
		return _random.nextUInt();
	}
};

class ThreadSafeRandom {
private:
	static __thread uint64_t _s[16];		// state
	static __thread int _p;
	static __thread bool _initialized;
private:
	static void init() {
		if (!_initialized) {
			uint64_t seed = StaticRandom::nextULong();
			for (int i = 0; i < 16; i++)
				_s[i] = Random::splitmix64_next(seed);
			_p = 0;
			_initialized = true;
		}
	}
public:
	static uint64_t nextULong() {
		init();
		const uint64_t s0 = _s[_p];
		uint64_t s1 = _s[_p = (_p + 1) & 15];
		s1 ^= s1 << 31; // a
		_s[_p] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30); // b,c
		return _s[_p] * UINT64_C(1181783497276652981);
	}

	static uint32_t nextUInt() {
		return (uint32_t)(nextULong() >> 32);
	}
};

} // namespace slib

#endif // H_SLIB_UTIL_RANDOM_H
