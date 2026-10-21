#pragma once

namespace apn::timeline_scroller
{
	//
	// このクラスはメインウィンドウです。
	//
	inline struct main_window_t : my::layout::Window<config_dialog_base_t>
	{
		inline static constexpr struct message_t {
			inline static constexpr int32_t k_update_focus_object = WM_APP + 2026;
		} k_message;

		//
		// フォーカスオブジェクトの位置です。
		//
		struct focus_object_t {
			int layer;
			int frame;
		} focus_object = {};

		//
		// フォーカスオブジェクトを変更します。
		//
		inline void change_focus_object(int layer, int frame)
		{
			MY_TRACE_FUNC("");

			// 一旦メンバー変数にコピーします。
			focus_object = { layer, frame };

			// ここはサブスレッドかもしれないので、メインスレッドにポストします。
			::PostMessage(*this, k_message.k_update_focus_object, 0, 0);
		}

		//
		// コンフィグでコントロールを更新します。
		//
		virtual BOOL on_to_ui() override
		{
			MY_TRACE_FUNC("");

			for (UINT i = 0; i < std::size(hive.move_layers); i++) set_int(idc_move_layer_0 + i * 2, hive.move_layers[i]);
			for (UINT i = 0; i < std::size(hive.move_frames); i++) set_int(idc_move_frame_0 + i * 2, hive.move_frames[i]);
			set_check(idc_flag_move_cursor, hive.flag_move_cursor);

			return TRUE;
		}

		//
		// コントロールでコンフィグを更新します。
		//
		virtual BOOL on_from_ui() override
		{
			MY_TRACE_FUNC("");

			for (UINT i = 0; i < std::size(hive.move_layers); i++) get_int(idc_move_layer_0 + i * 2, hive.move_layers[i]);
			for (UINT i = 0; i < std::size(hive.move_frames); i++) get_int(idc_move_frame_0 + i * 2, hive.move_frames[i]);
			get_check(idc_flag_move_cursor, hive.flag_move_cursor);

			return TRUE;
		}

		//
		// WM_INITDIALOGを処理します。
		//
		virtual void on_init_dialog() override
		{
			MY_TRACE_FUNC("");

			// vs2022のリソースエディタにはスピンコントロールのサイズを
			// 正常に指定できないバグがあるので、ここで手動で修正しています。

			// 基準サイズを取得します。
			auto base_rc = my::get_window_rect(ctrl(idc_move_frame_spin_0));
			auto base_size = my::get_height(base_rc) - 2;

			//
			// この関数は垂直スピンコントロールをリサイズします。
			//
			const auto resize_vert_spin = [base_size](HWND spin)
			{
				auto rc = my::get_window_rect(spin);
				auto h = my::get_height(rc);

				::SetWindowPos(spin, nullptr, 0, 0, base_size, h, SWP_NOMOVE | SWP_NOZORDER);
			};

			// 垂直スピンコントロールをリサイズします。
			for (UINT i = 0; i < std::size(hive.move_layers); i++)
				resize_vert_spin(ctrl(idc_move_layer_spin_0 + i * 2));
			resize_vert_spin(ctrl(idc_focus_layer_spin));

			//
			// この関数は水平スピンコントロールをリサイズします。
			//
			const auto resize_horz_spin = [base_size](HWND spin)
			{
				auto rc = my::get_window_rect(spin);
				auto w = my::get_width(rc);

				::SetWindowPos(spin, nullptr, 0, 0, w, base_size, SWP_NOMOVE | SWP_NOZORDER);
			};

			// 水平スピンコントロールをリサイズします。
			for (UINT i = 0; i < std::size(hive.move_frames); i++)
				resize_horz_spin(ctrl(idc_move_frame_spin_0 + i * 2));
			resize_horz_spin(ctrl(idc_focus_frame_spin));
		}

		//
		// WM_COMMANDを処理します。
		//
		virtual void on_command(UINT code, UINT control_id, HWND control) override
		{
			switch (control_id)
			{
			// エディットボックス
			case idc_move_layer_0:
			case idc_move_layer_1:
			case idc_move_layer_2:
			case idc_move_layer_3:
			case idc_move_frame_0:
			case idc_move_frame_1:
			case idc_move_frame_2:
			case idc_move_frame_3:
				if (code == EN_CHANGE) from_ui(); break;
			// チェックボックス
			case idc_flag_move_cursor:
				from_ui(); break;
			}
		}

		//
		// WM_NOTIFYを処理します。
		//
		virtual void on_notify(NMHDR* nmhdr) override
		{
			switch(nmhdr->code)
			{
			case UDN_DELTAPOS:
				{
					auto nm = (NMUPDOWN*)nmhdr;
					auto edit_id = (UINT)nmhdr->idFrom - 1;

					switch (edit_id)
					{
					case idc_move_layer_0:
					case idc_move_layer_1:
					case idc_move_layer_2:
					case idc_move_layer_3:
						app->move_layer(get_int(edit_id) * +nm->iDelta); break;
					case idc_move_frame_0:
					case idc_move_frame_1:
					case idc_move_frame_2:
					case idc_move_frame_3:
						app->move_frame(get_int(edit_id) * -nm->iDelta); break;
					case idc_focus_layer: app->change_focus_object_layer(+nm->iDelta); break;
					case idc_focus_frame: app->change_focus_object_frame(-nm->iDelta); break;
					}

					break;
				}
			}
		}

		//
		// ウィンドウプロシージャです。
		//
		virtual LRESULT on_wnd_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) override
		{
			switch (message)
			{
			case k_message.k_update_focus_object:
				{
					MY_TRACE("k_message.k_update_focus_object\n");

					my::locker_t lock(this);

					if (focus_object.layer < 0)
						set_text(idc_focus_layer, L"");
					else
						set_int(idc_focus_layer, focus_object.layer + 1);

					if (focus_object.frame < 0)
						set_text(idc_focus_frame, L"");
					else
						set_int(idc_focus_frame, focus_object.frame);

					break;
				}
			}

			return __super::on_wnd_proc(hwnd, message, w_param, l_param);
		}
	} main_window;
}
