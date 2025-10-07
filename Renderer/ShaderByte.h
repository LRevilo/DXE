#pragma once
#include "DXE.h"
namespace DXE {
	struct DXE_API ShaderByteStruct {
		const char* name;
		const unsigned char* data;
		unsigned int length;
	};

	struct DXE_API ShaderStruct {
		ShaderByteStruct* vs;
		ShaderByteStruct* ps;
		ShaderByteStruct* cs;
		ShaderByteStruct* hs;
		ShaderByteStruct* ds;
		ShaderByteStruct* gs;
	};
}