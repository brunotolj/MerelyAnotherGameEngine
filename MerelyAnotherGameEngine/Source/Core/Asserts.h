#pragma once

#define mage_ensure(x) ((x) ? true : (__debugbreak(), false))
#define mage_check(x) { if (!(x)) throw; }
