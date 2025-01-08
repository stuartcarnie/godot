//
// Created by Stuart Carnie on 4/8/2024.
//

#include "servers/display_server.h"

DisplayServer *DisplayServer::singleton = nullptr;

bool DisplayServer::hidpi_allowed = false;

bool DisplayServer::window_early_clear_override_enabled = false;
Color DisplayServer::window_early_clear_override_color = Color(0, 0, 0, 0);

DisplayServer::DisplayServerCreate DisplayServer::server_create_functions[DisplayServer::MAX_SERVERS] = {
};

int DisplayServer::server_create_count = 1;

void DisplayServer::help_set_search_callbacks(const Callable &p_search_callback, const Callable &p_action_callback) {
}

#ifndef DISABLE_DEPRECATED

RID DisplayServer::_get_rid_from_name(NativeMenu *p_nmenu, const String &p_menu_root) const {
	return RID();
}

int DisplayServer::global_menu_add_item(const String &p_menu_root, const String &p_label, const Callable &p_callback, const Callable &p_key_callback, const Variant &p_tag, Key p_accel, int p_index) {
	return 0;
}

int DisplayServer::global_menu_add_check_item(const String &p_menu_root, const String &p_label, const Callable &p_callback, const Callable &p_key_callback, const Variant &p_tag, Key p_accel, int p_index) {
	return 0;
}

int DisplayServer::global_menu_add_icon_item(const String &p_menu_root, const Ref<Texture2D> &p_icon, const String &p_label, const Callable &p_callback, const Callable &p_key_callback, const Variant &p_tag, Key p_accel, int p_index) {
	return 0;
}

int DisplayServer::global_menu_add_icon_check_item(const String &p_menu_root, const Ref<Texture2D> &p_icon, const String &p_label, const Callable &p_callback, const Callable &p_key_callback, const Variant &p_tag, Key p_accel, int p_index) {
	return 0;
}

int DisplayServer::global_menu_add_radio_check_item(const String &p_menu_root, const String &p_label, const Callable &p_callback, const Callable &p_key_callback, const Variant &p_tag, Key p_accel, int p_index) {
	return 0;
}

int DisplayServer::global_menu_add_icon_radio_check_item(const String &p_menu_root, const Ref<Texture2D> &p_icon, const String &p_label, const Callable &p_callback, const Callable &p_key_callback, const Variant &p_tag, Key p_accel, int p_index) {
	return 0;
}

int DisplayServer::global_menu_add_multistate_item(const String &p_menu_root, const String &p_label, int p_max_states, int p_default_state, const Callable &p_callback, const Callable &p_key_callback, const Variant &p_tag, Key p_accel, int p_index) {
	return 0;}

void DisplayServer::global_menu_set_popup_callbacks(const String &p_menu_root, const Callable &p_open_callback, const Callable &p_close_callback) {
}

int DisplayServer::global_menu_add_submenu_item(const String &p_menu_root, const String &p_label, const String &p_submenu, int p_index) {
	return 0;}

int DisplayServer::global_menu_add_separator(const String &p_menu_root, int p_index) {
	return 0;}

int DisplayServer::global_menu_get_item_index_from_text(const String &p_menu_root, const String &p_text) const {
	return 0;}

int DisplayServer::global_menu_get_item_index_from_tag(const String &p_menu_root, const Variant &p_tag) const {
	return 0;}

void DisplayServer::global_menu_set_item_callback(const String &p_menu_root, int p_idx, const Callable &p_callback) {
}

void DisplayServer::global_menu_set_item_hover_callbacks(const String &p_menu_root, int p_idx, const Callable &p_callback) {
}

void DisplayServer::global_menu_set_item_key_callback(const String &p_menu_root, int p_idx, const Callable &p_key_callback) {
}

bool DisplayServer::global_menu_is_item_checked(const String &p_menu_root, int p_idx) const {
	return false;
}

bool DisplayServer::global_menu_is_item_checkable(const String &p_menu_root, int p_idx) const {
	return false;
}

bool DisplayServer::global_menu_is_item_radio_checkable(const String &p_menu_root, int p_idx) const {
	return false;
}

Callable DisplayServer::global_menu_get_item_callback(const String &p_menu_root, int p_idx) const {
	return Callable();
}

Callable DisplayServer::global_menu_get_item_key_callback(const String &p_menu_root, int p_idx) const {
	return Callable();
}

Variant DisplayServer::global_menu_get_item_tag(const String &p_menu_root, int p_idx) const {
	return 0;
}

String DisplayServer::global_menu_get_item_text(const String &p_menu_root, int p_idx) const {
	return String();
}

String DisplayServer::global_menu_get_item_submenu(const String &p_menu_root, int p_idx) const {
	return String();
}

Key DisplayServer::global_menu_get_item_accelerator(const String &p_menu_root, int p_idx) const {
	return Key::A;
}

bool DisplayServer::global_menu_is_item_disabled(const String &p_menu_root, int p_idx) const {
	return false;
}

bool DisplayServer::global_menu_is_item_hidden(const String &p_menu_root, int p_idx) const {
	return false;}

String DisplayServer::global_menu_get_item_tooltip(const String &p_menu_root, int p_idx) const {
	return String();
}

int DisplayServer::global_menu_get_item_state(const String &p_menu_root, int p_idx) const {
	return 0;
}

int DisplayServer::global_menu_get_item_max_states(const String &p_menu_root, int p_idx) const {
	return 0;
}

class Texture2D {
public:
	void reference() {};
	bool unreference() { return true; };
};

Ref<Texture2D> DisplayServer::global_menu_get_item_icon(const String &p_menu_root, int p_idx) const {
	Ref<Texture2D> ref;
	return ref;
}

int DisplayServer::global_menu_get_item_indentation_level(const String &p_menu_root, int p_idx) const {
	return 0;
}

void DisplayServer::global_menu_set_item_checked(const String &p_menu_root, int p_idx, bool p_checked) {
}

void DisplayServer::global_menu_set_item_checkable(const String &p_menu_root, int p_idx, bool p_checkable) {
}

void DisplayServer::global_menu_set_item_radio_checkable(const String &p_menu_root, int p_idx, bool p_checkable) {
}

void DisplayServer::global_menu_set_item_tag(const String &p_menu_root, int p_idx, const Variant &p_tag) {
}

void DisplayServer::global_menu_set_item_text(const String &p_menu_root, int p_idx, const String &p_text) {
}

void DisplayServer::global_menu_set_item_submenu(const String &p_menu_root, int p_idx, const String &p_submenu) {
}

void DisplayServer::global_menu_set_item_accelerator(const String &p_menu_root, int p_idx, Key p_keycode) {
}

void DisplayServer::global_menu_set_item_disabled(const String &p_menu_root, int p_idx, bool p_disabled) {
}

void DisplayServer::global_menu_set_item_hidden(const String &p_menu_root, int p_idx, bool p_hidden) {
}

void DisplayServer::global_menu_set_item_tooltip(const String &p_menu_root, int p_idx, const String &p_tooltip) {
}

void DisplayServer::global_menu_set_item_state(const String &p_menu_root, int p_idx, int p_state) {
}

void DisplayServer::global_menu_set_item_max_states(const String &p_menu_root, int p_idx, int p_max_states) {
}

void DisplayServer::global_menu_set_item_icon(const String &p_menu_root, int p_idx, const Ref<Texture2D> &p_icon) {
}

void DisplayServer::global_menu_set_item_indentation_level(const String &p_menu_root, int p_idx, int p_level) {
}

int DisplayServer::global_menu_get_item_count(const String &p_menu_root) const {
	return 0;
}

void DisplayServer::global_menu_remove_item(const String &p_menu_root, int p_idx) {
}

void DisplayServer::global_menu_clear(const String &p_menu_root) {
}

Dictionary DisplayServer::global_menu_get_system_menu_roots() const {
	Dictionary out;
	return out;
}

#endif

bool DisplayServer::tts_is_speaking() const {
	return false;
}

bool DisplayServer::tts_is_paused() const {
	return false;
}

void DisplayServer::tts_pause() {
}

void DisplayServer::tts_resume() {
}

TypedArray<Dictionary> DisplayServer::tts_get_voices() const {
	return TypedArray<Dictionary>();
}

PackedStringArray DisplayServer::tts_get_voices_for_language(const String &p_language) const {
	PackedStringArray ret;
	return ret;
}

void DisplayServer::tts_speak(const String &p_text, const String &p_voice, int p_volume, float p_pitch, float p_rate, int p_utterance_id, bool p_interrupt) {
}

void DisplayServer::tts_stop() {
}

void DisplayServer::tts_set_utterance_callback(TTSUtteranceEvent p_event, const Callable &p_callable) {
}

void DisplayServer::tts_post_utterance_event(TTSUtteranceEvent p_event, int p_id, int p_pos) {
}

bool DisplayServer::_get_window_early_clear_override(Color &r_color) {
	return false;
}

void DisplayServer::set_early_window_clear_color_override(bool p_enabled, Color p_color) {
}

void DisplayServer::mouse_set_mode(MouseMode p_mode) {
}

DisplayServer::MouseMode DisplayServer::mouse_get_mode() const {
	return MOUSE_MODE_VISIBLE;
}

void DisplayServer::warp_mouse(const Point2i &p_position) {
}

Point2i DisplayServer::mouse_get_position() const {
	return Point2i();
}

BitField<MouseButtonMask> DisplayServer::mouse_get_button_state() const {
	ERR_FAIL_V_MSG(0, "Mouse is not supported by this display server.");
}

void DisplayServer::clipboard_set(const String &p_text) {
}

String DisplayServer::clipboard_get() const {
	ERR_FAIL_V_MSG(String(), "Clipboard is not supported by this display server.");
}

Ref<Image> DisplayServer::clipboard_get_image() const {
	ERR_FAIL_V_MSG(Ref<Image>(), "Clipboard is not supported by this display server.");
}

bool DisplayServer::clipboard_has() const {
	return false;
}

bool DisplayServer::clipboard_has_image() const {
	return false;
}

void DisplayServer::clipboard_set_primary(const String &p_text) {
}

String DisplayServer::clipboard_get_primary() const {
	ERR_FAIL_V_MSG(String(), "Primary clipboard is not supported by this display server.");
}

void DisplayServer::screen_set_orientation(ScreenOrientation p_orientation, int p_screen) {
}

DisplayServer::ScreenOrientation DisplayServer::screen_get_orientation(int p_screen) const {
	return SCREEN_LANDSCAPE;
}

float DisplayServer::screen_get_scale(int p_screen) const {
	return 1.0f;
};

bool DisplayServer::is_touchscreen_available() const {
	return false;
}

void DisplayServer::screen_set_keep_on(bool p_enable) {
}

bool DisplayServer::screen_is_kept_on() const {
	return false;
}

int DisplayServer::get_screen_from_rect(const Rect2 &p_rect) const {
	int pos_screen = -1;
	return pos_screen;
}

DisplayServer::WindowID DisplayServer::create_sub_window(WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Rect2i &p_rect, bool p_exclusive, WindowID p_transient_parent) {
	ERR_FAIL_V_MSG(INVALID_WINDOW_ID, "Sub-windows not supported by this display server.");
}

void DisplayServer::show_window(WindowID p_id) {
	ERR_FAIL_MSG("Sub-windows not supported by this display server.");
}

void DisplayServer::delete_sub_window(WindowID p_id) {
	ERR_FAIL_MSG("Sub-windows not supported by this display server.");
}

void DisplayServer::window_set_exclusive(WindowID p_window, bool p_exclusive) {
	// Do nothing, if not supported.
}

void DisplayServer::window_set_mouse_passthrough(const Vector<Vector2> &p_region, WindowID p_window) {
	ERR_FAIL_MSG("Mouse passthrough not supported by this display server.");
}

void DisplayServer::gl_window_make_current(DisplayServer::WindowID p_window_id) {
	// noop except in gles
}

void DisplayServer::window_set_ime_active(const bool p_active, WindowID p_window) {
}

void DisplayServer::window_set_ime_position(const Point2i &p_pos, WindowID p_window) {
}

Point2i DisplayServer::ime_get_selection() const {
	ERR_FAIL_V_MSG(Point2i(), "IME or NOTIFICATION_WM_IME_UPDATE not supported by this display server.");
}

String DisplayServer::ime_get_text() const {
	ERR_FAIL_V_MSG(String(), "IME or NOTIFICATION_WM_IME_UPDATEnot supported by this display server.");
}

void DisplayServer::virtual_keyboard_show(const String &p_existing_text, const Rect2 &p_screen_rect, VirtualKeyboardType p_type, int p_max_length, int p_cursor_start, int p_cursor_end) {
}

void DisplayServer::virtual_keyboard_hide() {
}

// returns height of the currently shown keyboard (0 if keyboard is hidden)
int DisplayServer::virtual_keyboard_get_height() const {
	ERR_FAIL_V_MSG(0, "Virtual keyboard not supported by this display server.");
}

void DisplayServer::cursor_set_shape(CursorShape p_shape) {
}

DisplayServer::CursorShape DisplayServer::cursor_get_shape() const {
	return CURSOR_ARROW;
}

void DisplayServer::cursor_set_custom_image(const Ref<Resource> &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot) {
}

bool DisplayServer::get_swap_cancel_ok() {
	return false;
}

void DisplayServer::enable_for_stealing_focus(OS::ProcessID pid) {
}

Error DisplayServer::dialog_show(String p_title, String p_description, Vector<String> p_buttons, const Callable &p_callback) {
	return ERR_UNAVAILABLE;
}

Error DisplayServer::dialog_input_text(String p_title, String p_description, String p_partial, const Callable &p_callback) {
	return ERR_UNAVAILABLE;
}

Error DisplayServer::file_dialog_show(const String &p_title, const String &p_current_directory, const String &p_filename, bool p_show_hidden, FileDialogMode p_mode, const Vector<String> &p_filters, const Callable &p_callback) {
	return ERR_UNAVAILABLE;
}

Error DisplayServer::file_dialog_with_options_show(const String &p_title, const String &p_current_directory, const String &p_root, const String &p_filename, bool p_show_hidden, FileDialogMode p_mode, const Vector<String> &p_filters, const TypedArray<Dictionary> &p_options, const Callable &p_callback) {
	return ERR_UNAVAILABLE;
}

int DisplayServer::keyboard_get_layout_count() const {
	return 0;
}

int DisplayServer::keyboard_get_current_layout() const {
	return -1;
}

void DisplayServer::keyboard_set_current_layout(int p_index) {
}

String DisplayServer::keyboard_get_layout_language(int p_index) const {
	return "";
}

String DisplayServer::keyboard_get_layout_name(int p_index) const {
	return "Not supported";
}

Key DisplayServer::keyboard_get_keycode_from_physical(Key p_keycode) const {
	ERR_FAIL_V_MSG(p_keycode, "Not supported by this display server.");
}

Key DisplayServer::keyboard_get_label_from_physical(Key p_keycode) const {
	ERR_FAIL_V_MSG(p_keycode, "Not supported by this display server.");
}

void DisplayServer::force_process_and_drop_events() {
}

void DisplayServer::release_rendering_thread() {
}

void DisplayServer::swap_buffers() {
}

void DisplayServer::set_native_icon(const String &p_filename) {
}

void DisplayServer::set_icon(const Ref<Image> &p_icon) {
}

DisplayServer::IndicatorID DisplayServer::create_status_indicator(const Ref<Texture2D> &p_icon, const String &p_tooltip, const Callable &p_callback) {
	return INVALID_INDICATOR_ID;
}

void DisplayServer::status_indicator_set_icon(IndicatorID p_id, const Ref<Texture2D> &p_icon) {
}

void DisplayServer::status_indicator_set_tooltip(IndicatorID p_id, const String &p_tooltip) {
}

void DisplayServer::status_indicator_set_menu(IndicatorID p_id, const RID &p_menu_rid) {
}

void DisplayServer::status_indicator_set_callback(IndicatorID p_id, const Callable &p_callback) {
}

Rect2 DisplayServer::status_indicator_get_rect(IndicatorID p_id) const {
	return Rect2();
}

void DisplayServer::delete_status_indicator(IndicatorID p_id) {
}

int64_t DisplayServer::window_get_native_handle(HandleType p_handle_type, WindowID p_window) const {
	return 0;
}

void DisplayServer::window_set_vsync_mode(DisplayServer::VSyncMode p_vsync_mode, WindowID p_window) {
}

DisplayServer::VSyncMode DisplayServer::window_get_vsync_mode(WindowID p_window) const {
	return VSyncMode::VSYNC_ENABLED;
}

DisplayServer::WindowID DisplayServer::get_focused_window() const {
	return MAIN_WINDOW_ID; // Proper value for single windows.
}

void DisplayServer::set_context(Context p_context) {
}

void DisplayServer::register_additional_output(Object *p_object) {
}

void DisplayServer::unregister_additional_output(Object *p_object) {
}

void DisplayServer::_bind_methods() {
}

Ref<Image> DisplayServer::_get_cursor_image_from_resource(const Ref<Resource> &p_cursor, const Vector2 &p_hotspot) {
	Ref<Image> image;
	return image;
}

void DisplayServer::register_create_function(const char *p_name, CreateFunction p_function, GetRenderingDriversFunction p_get_drivers) {

}

int DisplayServer::get_create_function_count() {
	return server_create_count;
}

const char *DisplayServer::get_create_function_name(int p_index) {
	return nullptr;
}

Vector<String> DisplayServer::get_create_function_rendering_drivers(int p_index) {
	return {};
}

DisplayServer *DisplayServer::create(int p_index, const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Context p_context, Error &r_error) {
	return nullptr;
}

void DisplayServer::_input_set_mouse_mode(Input::MouseMode p_mode) {
}

Input::MouseMode DisplayServer::_input_get_mouse_mode() {
	return Input::MouseMode::MOUSE_MODE_CAPTURED;
}

void DisplayServer::_input_warp(const Vector2 &p_to_pos) {
}

Input::CursorShape DisplayServer::_input_get_current_cursor_shape() {
	return Input::CursorShape::CURSOR_ARROW;
}

void DisplayServer::_input_set_custom_mouse_cursor_func(const Ref<Resource> &p_image, Input::CursorShape p_shape, const Vector2 &p_hostspot) {
}

DisplayServer::DisplayServer() {
	singleton = this;
}

DisplayServer::~DisplayServer() {
	singleton = nullptr;
}


#include "servers/rendering_server.h"

RenderingServer *RenderingServer::singleton = nullptr;

RenderingServer *RenderingServer::get_singleton() {
	return singleton;
}
