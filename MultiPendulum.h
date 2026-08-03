#pragma once

#include "struct.h"
#include <vector>
#include <cstdint>
#include <memory>

namespace Engine {

	class DirectXCommon;
	class ModelPSO;
	class DirectionalLight;
	class Model;
	class Object3D;
	class Camera;
	class Line;
	class LinePSO;
}

class MultiPendulum {
public:
	MultiPendulum() = default;
	~MultiPendulum();

	void Initialize(
		uint32_t particleCount,
		const Vector3& origin,
		float segmentLength,
		Engine::DirectXCommon* dxCommon,
		Engine::ModelPSO* modelPSO,
		Engine::LinePSO* linePSO,
		Engine::DirectionalLight* directionalLight,
		Engine::Model* sphereModel
	);

	void Update(
		float deltaTime,
		const Engine::Camera& camera
	);
	void Draw();

	void Reset();
	void Rebuild(uint32_t particleCount);

	/*==================== アクセッサの宣言、定義 ====================*/
	bool GetIsRunning() const {
		return isRunning_;
	}

	void SetIsRunning(bool isRunning) {
		isRunning_ = isRunning;
	}

private:
	void Integrate(float deltaTime);
	void SolveConstraints();
	void UpdateTransforms();

private:
	// 実装のため専用関数として
	float Length(Vector3 vector);

	Engine::DirectXCommon* dxCommon_ = nullptr;
	Engine::ModelPSO* modelPSO_ = nullptr;
	Engine::LinePSO* linePSO_ = nullptr;
	Engine::DirectionalLight* directionalLight_ = nullptr;
	Engine::Model* sphereModel_ = nullptr;

	// 物理計算用
	std::vector<PendulumParticle> particles_;
	std::vector<DistanceConstraint> constraints_;

	// 描画用
	// 質点描画用Object3D
	std::vector<std::unique_ptr<Engine::Object3D>>
		particleObjects_;

	// 軸描画用Line
	std::vector<std::unique_ptr<Engine::Line>>
		segmentLines_;


	Vector3 origin_{};

	Vector3 gravity_ = {
		0.0f,
		-9.8f,
		0.0f
	};

	float segmentLength_ = 1.0f;
	float damping_ = 0.995f;

	uint32_t constraintIterationCount_ = 8;
	uint32_t requestedParticleCount_ = 5;

	bool isRunning_ = false;
	bool isParameterChanged_ = false;
};