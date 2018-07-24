#include <stdio.h>
#include <iostream>
#include "version.h"
#include "version_api.h"

using namespace std;

using dsp::lts::Version;

Version version_g;

#ifdef __cplusplus
extern "C" {
#endif

int get_compile_time(char *ver)
{
	string v = version_g.GetCurrent();
	memcpy(ver, v.c_str(), v.length());

	return 0;
}


#ifdef __cplusplus
}
#endif



