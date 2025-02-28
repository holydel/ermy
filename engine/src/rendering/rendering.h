#pragma once

#include "ermy_api.h"
#include <glm/glm.hpp>

namespace rendering
{
	void BeginFrame();
	void Initialize();
	void Shutdown();
	void Process();
	void EndFrame();
}

struct ErmyFrame
{
	glm::mat4 ViewMatrix[2];
	glm::mat4 ProjMatrix[2];
	glm::mat4 ViewProjMatrix[2];

	glm::mat4 ViewMatrixInv[2];
	glm::mat4 ProjMatrixInv[2];
	glm::mat4 ViewProjMatrixInv[2];

	glm::mat2 canvasPreRotate;
	glm::vec4 canvasClearColor;

	glm::vec4 CameraWorldPos[2];
	glm::vec4 CameraViewDir[2];

	glm::vec2 canvasRepSizeHalf; //float2(2.0 / width, 2.0 / height)

	float time;
	float deltaTime;
	float refreshRateTime;
};

extern ErmyFrame gErmyFrameConstants;