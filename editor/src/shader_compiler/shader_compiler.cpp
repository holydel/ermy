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


slang::IGlobalSession* gSlangGlobalSession = nullptr;
slang::ISession* gSlangSession = nullptr;
slang::ICompileRequest* gSlangRequest = nullptr;


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
	//SlangGlobalSessionDesc
	//auto res1 = slang_createGlobalSessionWithoutCoreModule(SLANG_API_VERSION, &gSlangGlobalSession);
	auto res1 = slang_createGlobalSession(SLANG_API_VERSION, &gSlangGlobalSession);

	slang::TargetDesc TargetSPIRV;
	slang::TargetDesc TargetDXIL;
	slang::TargetDesc TargetMETALLIB;
	slang::TargetDesc TargetWEBGPU;

	TargetSPIRV.format = SlangCompileTarget::SLANG_SPIRV;
	TargetDXIL.format = SlangCompileTarget::SLANG_DXIL;
	TargetMETALLIB.format = SlangCompileTarget::SLANG_METAL_LIB;
	TargetWEBGPU.format = SlangCompileTarget::SLANG_WGSL_SPIRV;

	TargetDXIL.profile = gSlangGlobalSession->findProfile("sm_6_0");
	std::vector<slang::TargetDesc> targets;
	targets.push_back(TargetSPIRV);
	targets.push_back(TargetDXIL);

	slang::SessionDesc sdesc = {};
	sdesc.targetCount = (int)targets.size();
	sdesc.targets = targets.data();

	//sdesc.flags
	auto res2 = gSlangGlobalSession->createSession(sdesc, &gSlangSession);

	// Initialize the Slang session
	//gSlangSession = spCreateSession(nullptr);

	//gSlangRequest = spCreateCompileRequest(gSlangSession);
	//spSetCompileFlags(gSlangRequest, SLANG_COMPILE_FLAG_ENABLE_REFLECTION);

	auto res3 = gSlangSession->createCompileRequest(&gSlangRequest);
	return true;
}

void ShaderCompiler::CompileShader(const std::string& shaderContent)
{

}

void ShaderCompiler::CompileShaderFile(const std::filesystem::path& shaderPath)
{
	gSlangRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "main");
	gSlangRequest->addTranslationUnitSourceFile(0, shaderPath.string().c_str());

	auto res1 = gSlangRequest->compile();
	if (res1 != SLANG_OK)
	{
		std::cout << "Shader compilation failed!" << std::endl;
		const char* diagnostics = spGetDiagnosticOutput(gSlangRequest);
		std::cerr << diagnostics << std::endl;
	}
	
	slang::IComponentType* outProgram = nullptr;
	gSlangRequest->getProgram(&outProgram);
	
	auto tcnt = gSlangRequest->getTranslationUnitCount();
	SlangReflection *reflection = gSlangRequest->getReflection();

	//SlangReflection* reflection = spGetReflection(gSlangRequest);
	
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
 
	//spAddTranslationUnit(gSlangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
	//spAddTranslationUnitSourceFile(gSlangRequest, 0, shaderPath.string().c_str());
	//

	//spSetCodeGenTarget(gSlangRequest, SlangCompileTarget::SLANG_SPIRV);
	////spSetTargetProfile(gSlangRequest, 0, spFindProfile(gSlangSession, "sm_6_0"));
	//spSetDebugInfoLevel(gSlangRequest, SLANG_DEBUG_INFO_LEVEL_NONE);
	//
	//// Compile the shader
	//if (spCompile(gSlangRequest) != SLANG_OK) {
	//	std::cout << "Shader compilation failed!" << std::endl;
	//	const char* diagnostics = spGetDiagnosticOutput(gSlangRequest);
	//	std::cerr << diagnostics << std::endl;
	//}
	//else {
	//	std::cout << "Shader compiled successfully!" << std::endl;
	//}
	//
	//auto tcnt = spGetTranslationUnitCount(gSlangRequest);
	//for (int t = 0; t < tcnt; ++t)
	//{
	//	//int entryPointCount = spGetTranslationUnitEntryPointCount(request, tuIndex);
	//}

	//SlangReflection* reflection = spGetReflection(gSlangRequest);

	//int entryPointCount = spReflection_getEntryPointCount(reflection);
	//for (int i = 0; i < entryPointCount; ++i)
	//{
	//	auto entryPointInfo = spReflection_getEntryPointByIndex(reflection, i);
	//	const char* eName = spReflectionEntryPoint_getName(entryPointInfo);
	//	auto eStage = spReflectionEntryPoint_getStage(entryPointInfo);

	//	SlangUInt axesThread[8] = {};
	//	SlangUInt waveSize = 0;
	//	//spReflectionEntryPoint_getComputeThreadGroupSize(entryPointInfo,)
	//	spReflectionEntryPoint_getComputeWaveSize(entryPointInfo, &waveSize);

	//	size_t size = 0;
	//	const void* spirvCode = spGetEntryPointCode(gSlangRequest, i, &size);

	//	int a = 52;
	//}
	//int a = 42;
	//int entryPointCount = spGetEntryPointCount(gSlangRequest);

}