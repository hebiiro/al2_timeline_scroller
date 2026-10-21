#pragma once

namespace apn::timeline_scroller
{
	//
	// このクラスはアプリケーションです。
	//
	inline struct app_t : app_base_t, app_interface_t
	{
		//
		// プラグインの初期化処理を実行します。
		//
		virtual BOOL on_initialize_plugin(DWORD version) override
		{
			MY_TRACE_FUNC("");

			// 各種開始処理を実行します。
			debug.init();
			config_io.init();
			main_window.init(idd_main);

			// コンフィグをファイルから読み込みます。
			config_io.read();

			return TRUE;
		}

		//
		// プラグインの後始末処理を実行します。
		//
		virtual BOOL on_uninitialize_plugin() override
		{
			MY_TRACE_FUNC("");

			// 各種終了処理を実行します。
			main_window.exit();
			config_io.exit();
			debug.exit();

			return TRUE;
		}

		//
		// プラグインを登録します。
		//
		virtual BOOL on_register_plugin(HOST_APP_TABLE* host) override
		{
			MY_TRACE_FUNC("");

			// メインウィンドウを登録します。
			host->register_window_client(version_base->get_name().c_str(), main_window);

			// フォーカスオブジェクトが変更された時のリスナーです。
			const auto listener = [](void* param)
			{
				// 読み込み専用の編集セクションを開始します。
				editor.handle->call_read_section([](EDIT_SECTION* edit)
				{
					// フォーカスオブジェクトの変更をハンドルします。
					app->on_change_focus_object(edit);
				});
			};

			// イベントリスナーを登録します。
			// UPDATE_OBJECTはオブジェクトが移動したことを検出します。
			// CHANGE_EDIT_SCENEはシーンが切り替わったことを検出します。
			// CHANGE_FOCUS_OBJECTはフォーカスオブジェクトが切り替わったことを検出します。
			host->register_event_listener(EVENT_TYPE::UPDATE_OBJECT, nullptr, listener);
			host->register_event_listener(EVENT_TYPE::CHANGE_EDIT_SCENE, nullptr, listener);
			host->register_event_listener(EVENT_TYPE::CHANGE_FOCUS_OBJECT, nullptr, listener);

			return TRUE;
		}

		//
		// フォーカスオブジェクトが変更された時の処理です。
		//
		virtual void on_change_focus_object(EDIT_SECTION* edit) override
		{
			MY_TRACE_FUNC("");

			// フォーカスオブジェクトを取得します。
			if (auto object = edit->get_focus_object())
			{
				// フォーカスオブジェクトのレイヤーとフレーム位置を取得します。
				auto object_layer_frame = edit->get_object_layer_frame(object);

				// ダイアログのフォーカスオブジェクトを更新します。
				main_window.change_focus_object(object_layer_frame.layer, object_layer_frame.start);
			}
			else
			{
				// ダイアログのフォーカスオブジェクトをリセットします。
				main_window.change_focus_object(-1, -1);
			}
		}

		//
		// 表示レイヤー位置を変更します。
		//
		virtual void move_layer(int layer_offset) override
		{
			// 引数を構造体にします。
			struct param_t {
				int layer_offset;
			} param = { layer_offset };

			// 編集セクションを開始します。
			editor.handle->call_edit_section_param(
				&param, [](void* p, EDIT_SECTION* edit)
			{
				// 引数の構造体を取得します。
				auto param = (param_t*)p;

				// カーソルを移動させる場合は
				if (hive.flag_move_cursor)
				{
					// カーソル位置を変更します。
					edit->set_cursor_layer_frame(
						edit->info->layer + param->layer_offset,
						edit->info->frame);
				}
//				// それ以外の場合は
//				else
				{
					// 表示位置を変更します。
					edit->set_display_layer_frame(
						edit->info->display_layer_start + param->layer_offset,
						edit->info->display_frame_start);
				}
			});
		}

		//
		// 表示フレーム位置を変更します。
		//
		virtual void move_frame(int frame_offset) override
		{
			// 引数を構造体にします。
			struct param_t {
				int frame_offset;
			} param = { frame_offset };

			// 編集セクションを開始します。
			editor.handle->call_edit_section_param(
				&param, [](void* p, EDIT_SECTION* edit)
			{
				// 引数の構造体を取得します。
				auto param = (param_t*)p;

				// カーソルを移動させる場合は
				if (hive.flag_move_cursor)
				{
					// カーソル位置を変更します。
					edit->set_cursor_layer_frame(
						edit->info->layer,
						edit->info->frame + param->frame_offset);
				}
//				// それ以外の場合は
//				else
				{
					// 表示位置を変更します。
					edit->set_display_layer_frame(
						edit->info->display_layer_start,
						edit->info->display_frame_start + param->frame_offset);
				}
			});
		}

		//
		// フォーカスオブジェクトをセットします。
		// 更に、フォーカスオブジェクトが表示されるようにタイムラインをスクロールします。
		//
		inline static void set_focus_object(EDIT_SECTION* edit, OBJECT_HANDLE object)
		{
			// フォーカスオブジェクトを変更します。
			edit->set_focus_object(object);

			// 新しいフォーカスオブジェクトのレイヤーとフレーム位置を取得します。
			auto object_layer_frame = edit->get_object_layer_frame(object);

			// 新しい表示位置です。
			auto layer = edit->info->display_layer_start;
			auto frame = edit->info->display_frame_start;

			// 新しいフォーカスオブジェクトが表示位置外の場合は
			if (object_layer_frame.layer < edit->info->display_layer_start ||
				object_layer_frame.layer >= (edit->info->display_layer_start + edit->info->display_layer_num - 1))
			{
				// 新しいフォーカスオブジェクトが表示されるように表示位置を調整します。
				layer = object_layer_frame.layer;
			}

			// 新しいフォーカスオブジェクトが表示位置外の場合は
			if (object_layer_frame.start < edit->info->display_frame_start ||
				object_layer_frame.start >= (edit->info->display_frame_start + edit->info->display_frame_num - 1))
			{
				// 新しいフォーカスオブジェクトが表示されるように表示位置を調整します。
				frame = object_layer_frame.start;
			}

			// カーソルを移動させる場合は
			if (hive.flag_move_cursor)
			{
				// カーソル位置を変更します。
				edit->set_cursor_layer_frame(layer, frame);
			}

			// 表示位置を変更します。
			edit->set_display_layer_frame(layer, frame);
		}

		//
		// フォーカスオブジェクトを次のレイヤー位置に変更します。
		//
		virtual void change_focus_object_layer(int layer) override
		{
			MY_TRACE_FUNC("");

			// 引数が無効の場合は何もしません。
			if (layer == 0) return;

			// 引数を構造体にします。
			struct param_t {
				int layer;
			} param = { layer };

			// 編集セクションを開始します。
			editor.handle->call_edit_section_param(
				&param, [](void* p, EDIT_SECTION* edit)
			{
				// 引数の構造体を取得します。
				auto param = (param_t*)p;

				// フォーカスオブジェクトを取得します。
				auto focus_object = edit->get_focus_object();
				if (!focus_object) return;

				// フォーカスオブジェクトのレイヤーとフレーム位置を取得します。
				auto focus_object_layer_frame = edit->get_object_layer_frame(focus_object);

				// 最も近いオブジェクトです。
				auto nearest_object = focus_object;
				auto min_layer_distance = INT_MAX;
				auto min_frame_distance = INT_MAX;

				// 検索位置(レイヤー)の初期値です。
				auto layer = focus_object_layer_frame.layer;

				// すぐ隣のレイヤーに移動できるとは限らないので、
				// ループで処理します。
				while (1)
				{
					// 次の検索位置(レイヤー)に進めます。
					layer += param->layer;

					// 検索位置が無効の場合はループを終了します。
					if (layer < 0 || layer > edit->info->layer_max) break;

					// 検索位置(フレーム)の初期値です。
					auto frame = 0;

					// フォーカスオブジェクトの最も近くにあるオブジェクトを検索します。
					while (1)
					{
						// オブジェクトを検索します。
						auto object = edit->find_object(layer, frame);

						// オブジェクトが無効の場合はループを終了します。
						if (!object) break;

						// オブジェクトのレイヤーとフレーム位置を取得します。
						auto object_layer_frame = edit->get_object_layer_frame(object);

						// フォーカスオブジェクトとのレイヤー距離を算出します。
						auto layer_distance = std::abs(object_layer_frame.layer - focus_object_layer_frame.layer);

						// フォーカスオブジェクトとのフレーム距離を算出します。
						auto frame_distance = std::abs(object_layer_frame.start - focus_object_layer_frame.start);

						// フォーカスオブジェクトにより近い場合は
						if (frame_distance < min_frame_distance)
						if (layer_distance <= min_layer_distance)
						{
							// 最も近いオブジェクトにセットします。
							nearest_object = object;
							min_layer_distance = layer_distance;
							min_frame_distance = frame_distance;
						}

						// 次の検索位置(フレーム)に進めます。
						frame = object_layer_frame.end + 1;
					}
				}

				// 最も近いオブジェクトが有効の場合は
				if (nearest_object)
				{
					// フォーカスオブジェクトを変更します。
					set_focus_object(edit, nearest_object);
				}
			});
		}

		//
		// フォーカスオブジェクトを次のフレーム位置に変更します。
		//
		virtual void change_focus_object_frame(int frame) override
		{
			MY_TRACE_FUNC("");

			// 引数を構造体にします。
			struct param_t {
				int frame;
			} param = { frame };

			// 編集セクションを開始します。
			editor.handle->call_edit_section_param(
				&param, [](void* p, EDIT_SECTION* edit)
			{
				// 引数の構造体を取得します。
				auto param = (param_t*)p;

				// フォーカスオブジェクトを取得します。
				if (auto focus_object = edit->get_focus_object())
				{
					// フォーカスオブジェクトのレイヤーとフレーム位置を取得します。
					auto focus_object_layer_frame = edit->get_object_layer_frame(focus_object);

					// 右のフレームに変更する場合は
					if (param->frame > 0)
					{
						// フォーカスオブジェクトのすぐ右隣にあるオブジェクトを検索します。

						// 次のオブジェクトのフレーム位置を算出します。
						auto frame = focus_object_layer_frame.end + param->frame;
						MY_TRACE_INT(frame);

						// 次のオブジェクトを検索します。
						if (auto next_object = edit->find_object(focus_object_layer_frame.layer, frame))
						{
							// フォーカスオブジェクトを変更します。
							set_focus_object(edit, next_object);
						}
					}
					// 左のフレームに変更する場合は
					else
					{
						// 次のオブジェクトです。
						auto next_object = focus_object;
						auto frame = 0;

						// フォーカスオブジェクトのすぐ左隣にあるオブジェクトを検索します。
						while (1)
						{
							// オブジェクトを検索します。
							auto object = edit->find_object(focus_object_layer_frame.layer, frame);

							// オブジェクトが無効の場合はループを終了します。
							if (!object || object == focus_object) break;

							// オブジェクトのレイヤーとフレーム位置を取得します。
							auto object_layer_frame = edit->get_object_layer_frame(object);

							// オブジェクトがフォーカスオブジェクトより右にある場合はループを終了します。
							if (object_layer_frame.start >= focus_object_layer_frame.start) break;

							// 次のオブジェクトにセットします。
							next_object = object;
							frame = object_layer_frame.end + 1;
						}

						// 次のオブジェクトが有効の場合は
						if (next_object)
						{
							// フォーカスオブジェクトを変更します。
							set_focus_object(edit, next_object);
						}
					}
				}
			});
		}
	} app_impl;
}
