#ifndef CINO_KEYBINDINGS_H
#define CINO_KEYBINDINGS_H

#include <cinolib/gl/gl_glfw.h>
#include <string>

namespace cinolib
{

	class KeyBinding final
	{

	public:

		static std::string key_name(int key);
		static std::string mod_name(int modifier);
		static std::string mod_names(int modifiers);

		int key;
		int modifiers;

		constexpr KeyBinding(int key, int modifiers = 0) : key{ key }, modifiers{ modifiers } {}

		constexpr KeyBinding operator|(int additional_modifiers) const
		{
			return { key, modifiers | additional_modifiers };
		}

		constexpr bool operator==(const KeyBinding& other) const
		{
			return key == other.key && modifiers == other.modifiers;
		}

		constexpr bool operator!=(const KeyBinding& other) const 
		{
			return !(*this == other);
		}

		std::string name() const;

	};

	class KeyBindings final
	{

	private:

		static void print(int key, const char* desc);
		static void print(KeyBinding key, const char* desc);

	public:

		static constexpr int no_key() { return -1; };
		static constexpr KeyBinding no_key_binding(){ return -1; };

		KeyBinding toggle_ortho{ GLFW_KEY_O };
		int camera_faster{ GLFW_KEY_LEFT_SHIFT };
		int camera_slower{ GLFW_KEY_LEFT_CONTROL };
		int camera_inplace_zoom{ GLFW_KEY_LEFT_ALT };
		int camera_inplace_rotation{ GLFW_KEY_LEFT_ALT };
		KeyBinding reset_camera{ GLFW_KEY_R };
		KeyBinding look_at_center{ GLFW_KEY_L };
		KeyBinding toggle_axes{ GLFW_KEY_A };
		KeyBinding toggle_sidebar{ GLFW_KEY_TAB };
		KeyBinding store_camera{ GLFW_KEY_C };
		KeyBinding restore_camera{ GLFW_KEY_V };
		KeyBinding camera_look_at_plus_x{ GLFW_KEY_X, GLFW_MOD_ALT };
		KeyBinding camera_look_at_plus_y{ GLFW_KEY_Y, GLFW_MOD_ALT };
		KeyBinding camera_look_at_plus_z{ GLFW_KEY_Z, GLFW_MOD_ALT };
		KeyBinding camera_look_at_minus_x{ GLFW_KEY_X, GLFW_MOD_ALT | GLFW_MOD_SHIFT };
		KeyBinding camera_look_at_minus_y{ GLFW_KEY_Y, GLFW_MOD_ALT | GLFW_MOD_SHIFT };
		KeyBinding camera_look_at_minus_z{ GLFW_KEY_Z, GLFW_MOD_ALT | GLFW_MOD_SHIFT };
		bool pan_with_arrow_keys{ true };
		bool pan_and_zoom_with_numpad_keys{ true };

		void print() const;

	};

	class MouseBindings final
	{

	private:

		static void print(int button, const char* desc);

	public:

		static std::string button_name(int button);

		static constexpr int no_button() { return -1; };

		int camera_pan{ GLFW_MOUSE_BUTTON_RIGHT };
		int camera_zoom{ GLFW_MOUSE_BUTTON_MIDDLE };
		int camera_rotate{ GLFW_MOUSE_BUTTON_LEFT };
		bool zoom_with_wheel{ true };

		void print() const;

	};

	void print_binding(const char* binding, const char* desc);

}

#ifndef  CINO_STATIC_LIB
#include "key_bindings.cpp"
#endif

#endif // CINO_KEYBINDINGS_H
