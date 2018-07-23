#pragma once

#define MFF_FEPSILON 0.00001
#define MFF_DEPSILON 0.00000001
bool FloatEqual(float a, float b, float epsilon = MFF_FEPSILON);
bool DoubleEqual(double a, double b, double epsilon = MFF_DEPSILON);