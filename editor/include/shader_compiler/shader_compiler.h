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

	static ShaderCompiler& Instance();
};