#pragma once

namespace apn::timeline_scroller
{
	//
	// このクラスはこのプログラムのバージョンを管理します。
	//
	inline struct version_t : version_base_t {
		//
		// コンストラクタです。
		//
		version_t() : version_base_t(L"タイムラインスクロール", L"🐍タイムラインスクロール🔖", L"r1") {}
	} version;
}
