#include "shader_compiler/shader_compiler.h"

#include <slang/slang.h>
#include <iostream>
#include <fstream>

ShaderCompiler gShaderCompiler;

ShaderCompiler::ShaderCompiler()
{

}

ShaderCompiler::~ShaderCompiler()
{

}

void ShaderCompiler::CompileShader(const char* shaderPath, const char* shaderName)
{

}

SlangSession* gSlangSession = nullptr;
SlangCompileRequest* gSlangRequest = nullptr;


void ShaderCompiler::CompileAllShaders()
{
	//spAddTranslationUnit(gSlangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

}

ShaderCompiler& ShaderCompiler::Instance()
{
	return gShaderCompiler;
}


bool ShaderCompiler::Initialize()
{
	// Initialize the Slang session
	gSlangSession = spCreateSession(nullptr);

	gSlangRequest = spCreateCompileRequest(gSlangSession);
	//spSetCompileFlags(gSlangRequest, SLANG_COMPILE_FLAG_ENABLE_REFLECTION);

	return true;
}

void ShaderCompiler::CompileShader(const std::string& shaderContent)
{

}

void ShaderCompiler::CompileShaderFile(const std::filesystem::path& shaderPath)
{
	spAddTranslationUnit(gSlangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
	spAddTranslationUnitSourceFile(gSlangRequest, 0, shaderPath.string().c_str());
	
	spSetCodeGenTarget(gSlangRequest, SlangCompileTarget::SLANG_TARGET_NONE);
	auto res = spCompile(gSlangRequest);
	SlangReflection* reflection2 = spGetReflection(gSlangRequest);

	int entryPointCount2 = spReflection_getEntryPointCount(reflection2);
	for (int i = 0; i < entryPointCount2; ++i)
	{
		auto entryPointInfo = spReflection_getEntryPointByIndex(reflection2, i);
		const char* eName = spReflectionEntryPoint_getName(entryPointInfo);
		auto eStage = spReflectionEntryPoint_getStage(entryPointInfo);
	}

	spSetCodeGenTarget(gSlangRequest, SlangCompileTarget::SLANG_DXIL);

	spSetTargetProfile(gSlangRequest, 0, spFindProfile(gSlangSession, "cs_6_0"));
	// Compile the shader
	if (spCompile(gSlangRequest) != SLANG_OK) {
		std::cout << "Shader compilation failed!" << std::endl;
		const char* diagnostics = spGetDiagnosticOutput(gSlangRequest);
		std::cerr << diagnostics << std::endl;
	}
	else {
		std::cout << "Shader compiled successfully!" << std::endl;
	}
	
	auto tcnt = spGetTranslationUnitCount(gSlangRequest);
	for (int t = 0; t < tcnt; ++t)
	{
		//int entryPointCount = spGetTranslationUnitEntryPointCount(request, tuIndex);
	}

	SlangReflection* reflection = spGetReflection(gSlangRequest);

	int entryPointCount = spReflection_getEntryPointCount(reflection);
	for (int i = 0; i < entryPointCount; ++i)
	{
		auto entryPointInfo = spReflection_getEntryPointByIndex(reflection, i);
		const char* eName = spReflectionEntryPoint_getName(entryPointInfo);
		auto eStage = spReflectionEntryPoint_getStage(entryPointInfo);

		SlangUInt axesThread[8] = {};
		SlangUInt waveSize = 0;
		//spReflectionEntryPoint_getComputeThreadGroupSize(entryPointInfo,)
		spReflectionEntryPoint_getComputeWaveSize(entryPointInfo, &waveSize);

		size_t size = 0;
		const void* spirvCode = spGetEntryPointCode(gSlangRequest, i, &size);

		int a = 52;
	}
	int a = 42;
	//int entryPointCount = spGetEntryPointCount(gSlangRequest);

}