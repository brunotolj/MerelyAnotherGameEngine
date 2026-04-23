#pragma once

//#ifdef MAGE_DEBUG
#define mage_ensure(x) ((x) ? true : (__debugbreak(), false))
#define mage_check(x) { if (!(x)) throw; }
//#else
//#define mage_ensure(x) (x)
//#define mage_check(x)
//#endif
