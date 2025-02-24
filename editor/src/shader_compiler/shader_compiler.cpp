#include "shader_compiler/shader_compiler.h"

#include <slang/slang.h>
#include <iostream>
#include <fstream>
#include <ermy_api.h>
#include <ermy_rendering.h>

using namespace ermy;

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

std::vector<slang::TargetDesc> gTargets;

std::string ErmyStageFromSlangStage(SlangStage stage)
{
	switch (stage)
	{
	case SlangStage::SLANG_STAGE_VERTEX:
		return "Vertex";
	case SlangStage::SLANG_STAGE_COMPUTE:
		return "Compute";
	case SlangStage::SLANG_STAGE_FRAGMENT:
		return "Fragment";
	case SlangStage::SLANG_STAGE_GEOMETRY:
		return "Geometry";
	}

	return "unknown";
}

std::string CompileTargetName(SlangCompileTarget target)
{
	switch (target)
	{
	case SLANG_SPIRV:
		return "spirv";
	case SLANG_DXIL:
		return "dxil";
	}

	return "unknown";
}

std::string TargetErmyHeader(SlangCompileTarget target)
{
	switch (target)
	{
	case SLANG_SPIRV:
		return "VULKAN";
	case SLANG_DXIL:
		return "D3D12";
	}

	return "unknown";
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

	TargetDXIL.profile = gSlangGlobalSession->findProfile("sm_6_5");

	std::vector<slang::CompilerOptionEntry> sharedCompilerOptions;

	slang::CompilerOptionEntry debugLevel;
	debugLevel.name = slang::CompilerOptionName::DebugInformation;
	debugLevel.value.intValue0 = 0;

	slang::CompilerOptionEntry optimizeLevel;
	optimizeLevel.name = slang::CompilerOptionName::Optimization;
	optimizeLevel.value.intValue0 = 0;

	slang::CompilerOptionEntry obfuscate;
	optimizeLevel.name = slang::CompilerOptionName::Obfuscate;
	optimizeLevel.value.intValue0 = 0;

	TargetDXIL.compilerOptionEntries = sharedCompilerOptions.data();
	TargetDXIL.compilerOptionEntryCount = (u32)sharedCompilerOptions.size();

	TargetSPIRV.compilerOptionEntries = sharedCompilerOptions.data();
	TargetSPIRV.compilerOptionEntryCount = (u32)sharedCompilerOptions.size();


	gTargets.push_back(TargetSPIRV);
	gTargets.push_back(TargetDXIL);
	//targets.push_back(TargetMETALLIB);
	//targets.push_back(TargetWEBGPU);

	slang::SessionDesc sdesc = {};
	sdesc.targetCount = (int)gTargets.size();
	sdesc.targets = gTargets.data();

	//sdesc.flags
	auto res2 = gSlangGlobalSession->createSession(sdesc, &gSlangSession);


	auto res3 = gSlangSession->createCompileRequest(&gSlangRequest);
	return true;
}

void ShaderCompiler::CompileShader(const std::string& shaderContent)
{

}

std::string ShaderDomainSuffix(ermy::ShaderDomainTag tag)
{
	std::string shaderDomainSuffix = "internal";
	if (tag == ermy::ShaderDomainTag::Editor)
	{
		shaderDomainSuffix = "editor";
	}
	if (tag == ermy::ShaderDomainTag::Runtime)
	{
		shaderDomainSuffix = "runtime";
	}
	if (tag == ermy::ShaderDomainTag::Application)
	{
		shaderDomainSuffix = "application";
	}
	return shaderDomainSuffix;
}
void AppendHeader(ermy::ShaderDomainTag shaderTag, std::ostringstream& stream, SlangCompileTarget target)
{
	stream << "#include <ermy_shader_internal.h>\n";
	stream << "#ifdef ERMY_GAPI_" << TargetErmyHeader(target) << std::endl;
	stream << "namespace ermy::shader_" << ShaderDomainSuffix(shaderTag) << std::endl;
	stream << "{\n";
}

void AppendFooter(std::ostringstream& stream)
{
	stream << "}\n"
		"#endif\n";
}

//#include <ermy_shader_internal.h>
//#ifdef ERMY_GAPI_VULKAN
//namespace ermy::shader_internal
//{
//	ShaderBytecode computeMain()
//	{
//		u8 data[] = { 3,5,63,6,3 };
//
//		ShaderBytecode result;
//		result.data = data;
//		result.size = sizeof(data);
//		result.isInternal = true;
//		result.stage = ShaderStage::Compute;
//	}
//}
//#endif

void AppendShader(ermy::ShaderDomainTag shaderTag, std::ostringstream& stream, const char* name, SlangStage stage, ISlangBlob* shaderBlob, u64 bytecodeCRC64)
{
	stream << "	ShaderInfo " << name << "()\n";
	stream << "	{\n"
		"		static u8 data[] = {";

	u64 numBytes = shaderBlob->getBufferSize();
	const ermy::u8* data = static_cast<const ermy::u8*>(shaderBlob->getBufferPointer());

	for (u64 i = 0; i < numBytes; ++i)
	{
		if (i > 0)
		{
			stream << ",";

			if (i % 128 == 0)
				stream << std::endl;
		}

		stream << static_cast<uint32_t>(data[i]);
	}

	stream << "};\n\n";

	
	std::string tagName = "Internal";

	if (shaderTag == ermy::ShaderDomainTag::Editor)
	{
		tagName = "Editor";
	}
	if (shaderTag == ermy::ShaderDomainTag::Runtime)
	{
		tagName = "Runtime";
	}
	if (shaderTag == ermy::ShaderDomainTag::Application)
	{
		tagName = "Application";
	}

	stream << "		ShaderInfo result;\n"
		"		result.byteCode.data = data;\n"
		"		result.byteCode.size = sizeof(data);\n"
		"		result.byteCode.isInternal = true;\n";
	stream << "		result.byteCode.stage = ShaderStage::" << ErmyStageFromSlangStage(stage) << ";\n\n";

	stream << "		result.bytecodeCRC64 = " << bytecodeCRC64 << ";\n";
	stream << "		result.shaderName = \"" << name << "\";\n";
	stream << "		result.tag = ermy::ShaderDomainTag::" << tagName << ";\n";

	stream << "		return result;\n"
		"	}\n\n";
}

void AddInternalTranslationUnit(const std::filesystem::path& shadersFolder, const std::string& shaderName)
{
	gSlangRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, shaderName.c_str());
	auto fullShaderPath = (shadersFolder / (shaderName + ".slang"));
	gSlangRequest->addTranslationUnitSourceFile(0, fullShaderPath.string().c_str());
}

void WriteStringStreamToFile(const std::ostringstream& stream, const std::string& filename)
{
	std::ofstream outFile(filename);
	outFile << stream.str();
	outFile.close();
}

void ShaderCompiler::CompileAllShaders(ermy::ShaderDomainTag shaderTag, const std::string& moduleName, const std::filesystem::path& shadersFolder, const std::filesystem::path& outputHeader)
{
	AddInternalTranslationUnit(shadersFolder, moduleName);

	auto res1 = gSlangRequest->compile();
	if (res1 != SLANG_OK)
	{
		std::cout << "Shader compilation failed!" << std::endl;
	}

	const char* diagnostics = spGetDiagnosticOutput(gSlangRequest);
	std::cout << diagnostics << std::endl;

	slang::IComponentType* outProgram = nullptr;
	gSlangRequest->getProgram(&outProgram);

	SlangReflection* reflection = gSlangRequest->getReflection();


	std::ostringstream shadersEngineHeader;



	shadersEngineHeader << "#pragma once\n"
		"#include \"ermy_shader.h\"\n"
		"\n"
		"namespace ermy\n"
		"{\n"
		"	namespace shader_" << ShaderDomainSuffix(shaderTag) << "\n"
		"	{\n";

	std::vector<std::ostringstream> shaderCppEmbded(gTargets.size());

	//SlangReflection* reflection = spGetReflection(gSlangRequest);
	for (int i = 0; i < gTargets.size(); ++i)
	{
		AppendHeader(shaderTag, shaderCppEmbded[i], gTargets[i].format);
	}

	u32 entryPointCount = static_cast<u32>(spReflection_getEntryPointCount(reflection));
	for (u32 i = 0; i < entryPointCount; ++i)
	{
		auto entryPointInfo = spReflection_getEntryPointByIndex(reflection, i);
		const char* eName = spReflectionEntryPoint_getName(entryPointInfo);
		auto eStage = spReflectionEntryPoint_getStage(entryPointInfo);

		SlangUInt axesThread[8] = {};
		SlangUInt waveSize = 0;
		//spReflectionEntryPoint_getComputeThreadGroupSize(entryPointInfo,)
		spReflectionEntryPoint_getComputeWaveSize(entryPointInfo, &waveSize);

		shadersEngineHeader << "		ShaderInfo " << eName << "();\n";

		//size_t size = 0;
		//const void* spirvCode = spGetEntryPointCode(gSlangRequest, i, &size);
		for (int j = 0; j < 2; ++j)
		{
			ISlangBlob* shaderILCode = nullptr;
			auto res = spGetEntryPointCodeBlob(gSlangRequest, i, j, &shaderILCode);
			int targetBytecode = -1;
#ifdef ERMY_GAPI_VULKAN
			targetBytecode = 0;
#endif
#ifdef ERMY_GAPI_D3D12
			targetBytecode = 1;
#endif
			ermy::u64 shaderByteCodeCRC = ermy_utils::hash::CalculateCRC64(static_cast<const ermy::u8*>(shaderILCode->getBufferPointer()), static_cast<ermy::u32>(shaderILCode->getBufferSize()));

			if (j == targetBytecode)
			{
				ermy::rendering::UpdateShaderBytecode(shaderTag, eName, shaderByteCodeCRC, static_cast<const ermy::u8*>(shaderILCode->getBufferPointer()), static_cast<ermy::u32>(shaderILCode->getBufferSize()));
			}
			AppendShader(shaderTag, shaderCppEmbded[j], eName, eStage, shaderILCode, shaderByteCodeCRC);

			//size_t shaderSize = shaderILCode->getBufferSize();
			//auto shaderILCodeRaw = shaderILCode->getBufferPointer();
		}
		int a = 52;
	}



	shadersEngineHeader << "	}\n"
		"}";

	WriteStringStreamToFile(shadersEngineHeader, outputHeader.string());

	for (int i = 0; i < gTargets.size(); ++i)
	{
		AppendFooter(shaderCppEmbded[i]);
		auto fullPathCPP = shadersFolder / (CompileTargetName(gTargets[i].format) + ".cpp");
		WriteStringStreamToFile(shaderCppEmbded[i], fullPathCPP.string());
	}
	int a = 42;
}

void ShaderCompiler::CompileAllEditorShaders()
{
	std::filesystem::path shadersPath = "../../editor/src/shaders/";
	std::filesystem::path shadersHeaderPath = "../../editor/include/editor_shader_internal.h";

	CompileAllShaders(ShaderDomainTag::Editor, "editor", shadersPath, shadersHeaderPath);
}

void ShaderCompiler::CompileAllEngineShaders()
{
	std::filesystem::path shadersPath = "../../engine/src/rendering/internal_shaders/";
	std::filesystem::path shadersHeaderPath = "../../engine/include/ermy_shader_internal.h";

	CompileAllShaders(ShaderDomainTag::Internal, "internal", shadersPath, shadersHeaderPath);
}

void ShaderCompiler::CompileShaderFile(const std::filesystem::path& shaderPath)
{
	//temprorary compile internal shaders
	//"../../engine/engine/src/rendering/internal_shaders/spirv.h"
	//"../../engine/engine/src/rendering/internal_shaders/dxil.h"

	gSlangRequest->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, "internal");
	gSlangRequest->addTranslationUnitSourceFile(0, shaderPath.string().c_str());

	auto res1 = gSlangRequest->compile();
	if (res1 != SLANG_OK)
	{
		std::cout << "Shader compilation failed!" << std::endl;
	}

	const char* diagnostics = spGetDiagnosticOutput(gSlangRequest);
	std::cout << diagnostics << std::endl;

	slang::IComponentType* outProgram = nullptr;
	gSlangRequest->getProgram(&outProgram);

	auto tcnt = gSlangRequest->getTranslationUnitCount();
	SlangReflection* reflection = gSlangRequest->getReflection();



	u32 entryPointCount = static_cast<u32>(spReflection_getEntryPointCount(reflection));
	for (u32 i = 0; i < entryPointCount; ++i)
	{
		auto entryPointInfo = spReflection_getEntryPointByIndex(reflection, i);
		const char* eName = spReflectionEntryPoint_getName(entryPointInfo);
		auto eStage = spReflectionEntryPoint_getStage(entryPointInfo);

		SlangUInt axesThread[8] = {};
		SlangUInt waveSize = 0;
		//spReflectionEntryPoint_getComputeThreadGroupSize(entryPointInfo,)
		spReflectionEntryPoint_getComputeWaveSize(entryPointInfo, &waveSize);

		//size_t size = 0;
		//const void* spirvCode = spGetEntryPointCode(gSlangRequest, i, &size);
		for (int j = 0; j < 2; ++j)
		{

			ISlangBlob* shaderILCode = nullptr;
			auto res = spGetEntryPointCodeBlob(gSlangRequest, i, j, &shaderILCode);

			size_t shaderSize = shaderILCode->getBufferSize();
			auto shaderILCodeRaw = shaderILCode->getBufferPointer();
		}
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