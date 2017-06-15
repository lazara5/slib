/*
 * Originally written in 2014-2015 by Sebastiano Vigna (vigna@acm.org)
 *
 * To the extent possible under law, the author has dedicated all copyright
 *and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * See <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include "slib/util/Random.h"
#include "slib/util/System.h"

namespace slib {

Random StaticRandom::_random(System::currentTimeMillis());
std::mutex StaticRandom::_lock;

__thread uint64_t ThreadSafeRandom::_s[16];		// state
__thread int ThreadSafeRandom::_p;
__thread bool ThreadSafeRandom::_initialized = false;

} // namespace
