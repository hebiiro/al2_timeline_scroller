#pragma once

namespace apn::timeline_scroller
{
	//
	// このクラスはコンフィグの入出力を担当します。
	//
	inline struct config_io_t : config_io_base2_t
	{
		//
		// ノードからコンフィグを読み込みます。
		//
		virtual BOOL read_node(n_json& root)
		{
			MY_TRACE_FUNC("");

			for (UINT i = 0; i < std::size(hive.move_layers); i++) read_int(root, my::format("move_layer_{/}", i), hive.move_layers[i]);
			for (UINT i = 0; i < std::size(hive.move_frames); i++) read_int(root, my::format("move_frame_{/}", i), hive.move_frames[i]);
			read_bool(root, "flag_move_cursor", hive.flag_move_cursor);

			return __super::read_node(root);
		}

		//
		// ノードにコンフィグを書き込みます。
		//
		virtual BOOL write_node(n_json& root)
		{
			MY_TRACE_FUNC("");

			for (UINT i = 0; i < std::size(hive.move_layers); i++) write_int(root, my::format("move_layer_{/}", i), hive.move_layers[i]);
			for (UINT i = 0; i < std::size(hive.move_frames); i++) write_int(root, my::format("move_frame_{/}", i), hive.move_frames[i]);
			write_bool(root, "flag_move_cursor", hive.flag_move_cursor);

			return __super::write_node(root);
		}
	} config_io;
}
