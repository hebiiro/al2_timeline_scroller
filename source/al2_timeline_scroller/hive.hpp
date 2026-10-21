#pragma once

namespace apn::timeline_scroller
{
	//
	// このクラスは他クラスから共通して使用される変数を保持します。
	//
	inline struct hive_t : hive_base_t
	{
		//
		// TRUEの場合はカーソルを移動します。
		//
		BOOL flag_move_cursor = TRUE;

		//
		// 各配列の最大数です。
		//
		inline static constexpr int32_t k_max_move_frames = 4;
		inline static constexpr int32_t k_max_move_layers = 4;

		//
		// フレームの移動量です。
		//
		int32_t move_frames[k_max_move_frames] = { 1, 60, 600, 999999 };

		//
		// レイヤーの移動量です。
		//
		int32_t move_layers[k_max_move_layers] = { 1, 4, 10, 999 };
	} hive;
}
