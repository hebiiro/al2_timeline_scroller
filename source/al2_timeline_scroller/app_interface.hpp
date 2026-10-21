#pragma once

namespace apn::timeline_scroller
{
	//
	// このクラスはアプリケーションのインターフェイスです。
	//
	inline struct app_interface_t {
		//
		// コンストラクタです。
		//
		app_interface_t() { app = this; }

		virtual void on_change_focus_object(EDIT_SECTION* edit) = 0;
		virtual void move_layer(int layer_offset) = 0;
		virtual void move_frame(int frame_offset) = 0;
		virtual void change_focus_object_layer(int layer) = 0;
		virtual void change_focus_object_frame(int frame) = 0;
	} *app = nullptr;
}
