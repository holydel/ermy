#pragma once
#include <string>
#include <filesystem>

class ShaderCompiler
{
public:
	ShaderCompiler();
	~ShaderCompiler();

	bool Initialize();

	void CompileShader(const std::string& shaderContent);
	void CompileShaderFile(const std::filesystem::path& shaderPath);
	void CompileShader(const char* shaderPath, const char* shaderName);
	void CompileAllShaders();


	void CompileAllInternalShaders(const std::string& moduleName, const std::filesystem::path& shadersFolder, const std::filesystem::path& outputHeader);
	void CompileAllEngineShaders();
	void CompileAllEditorShaders();

	static ShaderCompiler& Instance();
};