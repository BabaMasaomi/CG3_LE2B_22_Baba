#pragma once

namespace Engine {

	// 継承用の基底クラス
    class GameObject {
    public:
		/* ==================== メンバ関数とデストラクタの宣言 ====================*/
        virtual ~GameObject() = default;

        /// <summary>
		/// 共通の初期化処理を行う
        /// </summary>
        virtual void Initialize() {}
        /// <summary>
		/// 更新処理を行う
        /// </summary>
        virtual void Update() {}
        /// <summary>
		/// 描画処理を行う
        /// </summary>
        virtual void Draw() {}

        /*==================== アクセッサ ====================*/
		// ゲッター
        bool IsActive() const { return isActive_; }

		// セッター
        void SetActive(bool active) { isActive_ = active; }

    protected:
        /*==================== メンバ変数 ====================*/
		// アクティブ状態を示すフラグ
        bool isActive_ = true;
    };
}