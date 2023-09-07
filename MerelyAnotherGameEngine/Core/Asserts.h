#pragma once

#define ensure(x) ((x) ? true : (__debugbreak(), false))
#define check(x) { if (!(x)) throw; }
