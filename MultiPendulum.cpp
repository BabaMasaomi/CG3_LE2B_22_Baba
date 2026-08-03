#include "MultiPendulum.h"
#include "Engine/Base/DirectXCommon.h"
#include "Engine/Graphics/ModelPSO.h"
#include "Engine/Resource/Model.h"
#include "Engine/Graphics/Object3D.h"
#include "LinePSO.h"
#include "Line.h"
#include "Engine/Graphics/DirectionalLight.h"
#include "Engine/Math/Utility.h"
#include <cassert>
#include <algorithm>
#include <cmath>


MultiPendulum::~MultiPendulum() = default;

/*--------------------  --------------------*/
void MultiPendulum::Initialize(
	uint32_t particleCount,
	const Vector3& origin,
	float segmentLength,
	Engine::DirectXCommon* dxCommon,
	Engine::ModelPSO* modelPSO,
	Engine::LinePSO* linePSO,
	Engine::DirectionalLight* directionalLight,
	Engine::Model* sphereModel) {

	assert(dxCommon != nullptr);
	assert(modelPSO != nullptr);
	assert(linePSO != nullptr);
	assert(directionalLight != nullptr);
	assert(sphereModel != nullptr);

	origin_ = origin;
	segmentLength_ = segmentLength;

	dxCommon_ = dxCommon;
	modelPSO_ = modelPSO;
	linePSO_ = linePSO;
	directionalLight_ = directionalLight;
	sphereModel_ = sphereModel;

	Rebuild(particleCount);
}

/*--------------------  --------------------*/
void MultiPendulum::Reset() {
	if (particles_.empty()) {
		return;
	}

	Vector3 initialDirection = {
		0.4f,
		-1.0f,
		0.2f
	};

	const float directionLength =
		Length(initialDirection);

	if (directionLength > 0.00001f) {
		initialDirection = {
			initialDirection.x / directionLength,
			initialDirection.y / directionLength,
			initialDirection.z / directionLength
		};
	}

	for (uint32_t i = 0;
		i < particles_.size();
		++i) {

		const float index =
			static_cast<float>(i);

		const Vector3 initialPosition =
			origin_ +
			initialDirection *
			(segmentLength_ * index);

		PendulumParticle& particle =
			particles_[i];

		particle.position =
			initialPosition;

		// Verlet積分の速度も0へ戻す
		particle.previousPosition =
			initialPosition;

		particle.acceleration = {
			0.0f,
			0.0f,
			0.0f
		};
	}

	particles_[0].position = origin_;
	particles_[0].previousPosition = origin_;
	particles_[0].mass = 0.0f;
	particles_[0].inverseMass = 0.0f;
	particles_[0].isFixed = true;

	UpdateTransforms();
}

/*--------------------  --------------------*/
void MultiPendulum::Rebuild(uint32_t particleCount) {
	particles_.clear();
	constraints_.clear();

	particleObjects_.clear();
	segmentLines_.clear();

	particleCount = (std::max)(particleCount, 2u);

	particles_.resize(particleCount);
	particleObjects_.resize(particleCount);

	// 軸の本数は質点数より1本少ない
	segmentLines_.resize(particleCount - 1);

	for (uint32_t i = 0; i < particleCount; ++i) {
		PendulumParticle& particle = particles_[i];

		particle.position = {};
		particle.previousPosition = {};
		particle.acceleration = {};
		particle.mass = 1.0f;
		particle.inverseMass = 1.0f;
		particle.isFixed = false;

		// 質点用Object3Dを生成
		particleObjects_[i] =
			std::make_unique<Engine::Object3D>();

		particleObjects_[i]->Initialize(
			dxCommon_,
			modelPSO_,
			directionalLight_
		);

		// 全質点で同じ球モデルを共有
		particleObjects_[i]->SetModel(sphereModel_);

		// ライティングを不要にする場合
		// particleObjects_[i]->SetLightingMode(
		//     Engine::LightingMode::None);
	}

	// 先頭を支点として固定
	particles_[0].mass = 0.0f;
	particles_[0].inverseMass = 0.0f;
	particles_[0].isFixed = true;

	for (uint32_t i = 0; i + 1 < particleCount; ++i) {
		DistanceConstraint constraint{};

		constraint.particleIndexA = i;
		constraint.particleIndexB = i + 1;
		constraint.restLength = segmentLength_;

		constraints_.push_back(constraint);

		// 軸用Lineを生成
		segmentLines_[i] =
			std::make_unique<Engine::Line>();

		segmentLines_[i]->Initialize(
			dxCommon_,
			linePSO_,
			origin_,
			origin_
		);
	}

	Reset();
}

/*--------------------  --------------------*/
void MultiPendulum::Update(
	float deltaTime,
	const Engine::Camera& camera) {

	if (isRunning_) {
		constexpr uint32_t kSubstepCount = 4;

		const float subDeltaTime =
			deltaTime /
			static_cast<float>(kSubstepCount);

		for (uint32_t i = 0;
			i < kSubstepCount;
			++i) {

			Integrate(subDeltaTime);
			SolveConstraints();
		}
	}

	// 物理位置を描画オブジェクトへ反映
	UpdateTransforms();

	// 質点Object3DのWVP行列を更新
	for (const auto& object : particleObjects_) {
		if (object == nullptr) {
			continue;
		}

		object->Update(camera);
	}

	// Lineの頂点とWVP行列を更新
	for (const auto& line : segmentLines_) {
		if (line == nullptr) {
			continue;
		}

		line->Update(camera);
	}
}

/*-------------------- Verlet積分 --------------------*/
void MultiPendulum::Integrate(float deltaTime) {
	const float deltaTimeSquared = deltaTime * deltaTime;

	for (PendulumParticle& particle : particles_) {
		if (particle.isFixed) {
			continue;
		}

		const Vector3 velocity =
			(particle.position - particle.previousPosition) * damping_;

		const Vector3 currentPosition = particle.position;

		particle.acceleration = gravity_;

		particle.position +=
			velocity +
			particle.acceleration * deltaTimeSquared;

		particle.previousPosition = currentPosition;
	}
}

/*-------------------- 距離拘束 --------------------*/
void MultiPendulum::SolveConstraints() {
	for (uint32_t iteration = 0;
		iteration < constraintIterationCount_;
		++iteration) {

		// 支点を固定位置へ戻す
		particles_[0].position = origin_;

		for (const DistanceConstraint& constraint : constraints_) {
			PendulumParticle& particleA =
				particles_[constraint.particleIndexA];

			PendulumParticle& particleB =
				particles_[constraint.particleIndexB];

			Vector3 difference =
				particleB.position - particleA.position;

			const float distance = Length(difference);

			if (distance <= 0.00001f) {
				continue;
			}

			const float error =
				distance - constraint.restLength;

			const Vector3 direction = { difference.x / distance ,difference.y / distance ,difference.z / distance };

			const float inverseMassSum =
				particleA.inverseMass +
				particleB.inverseMass;

			if (inverseMassSum <= 0.0f) {
				continue;
			}

			const Vector3 correction =
				direction * error;

			if (!particleA.isFixed) {
				particleA.position +=
					correction *
					(particleA.inverseMass / inverseMassSum);
			}

			if (!particleB.isFixed) {
				particleB.position -=
					correction *
					(particleB.inverseMass / inverseMassSum);
			}
		}
	}

	particles_[0].position = origin_;
}

/*--------------------  --------------------*/
void MultiPendulum::UpdateTransforms() {
	if (particleObjects_.size() != particles_.size()) {
		return;
	}

	// 質点の位置をObject3Dへ反映
	for (size_t i = 0; i < particles_.size(); ++i) {
		if (particleObjects_[i] == nullptr) {
			continue;
		}

		const PendulumParticle& particle =
			particles_[i];

		Engine::Object3D& object =
			*particleObjects_[i];

		const float radius =
			particle.isFixed ? 0.12f : 0.1f;

		object.SetScale({
			radius,
			radius,
			radius
			});

		object.SetRotate({
			0.0f,
			0.0f,
			0.0f
			});

		object.SetTranslate(
			particle.position
		);
	}

	// 隣接する質点の位置をLineへ反映
	for (size_t i = 0; i < segmentLines_.size(); ++i) {
		if (segmentLines_[i] == nullptr) {
			continue;
		}

		segmentLines_[i]->SetStart(
			particles_[i].position
		);

		segmentLines_[i]->SetEnd(
			particles_[i + 1].position
		);
	}
}

/*-------------------- 描画 --------------------*/
void MultiPendulum::Draw() {
	// 軸を先に描画
	for (const auto& line : segmentLines_) {
		if (line == nullptr) {
			continue;
		}

		line->Draw();
	}

	// 質点を描画
	for (const auto& object : particleObjects_) {
		if (object == nullptr) {
			continue;
		}

		object->Draw();
	}
}

// 実装のため専用関数として
float MultiPendulum::Length(Vector3 vector) {
	return sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}