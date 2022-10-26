#include <cinolib/gl/key_bindings.hpp>

#include <cinolib/cino_inline.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <cctype>
#include <array>

namespace cinolib
{

	CINO_INLINE std::string KeyBinding::key_name(int key)
	{
		const char* key_name{ glfwGetKeyName(key, 0) };
		if (key_name)
		{
			std::string name{ key_name };
			for (auto& c : name) c = toupper(c);
			return name;
		}
		switch (key)
		{
			case GLFW_KEY_LEFT_SHIFT:
				return "LSHIFT";
			case GLFW_KEY_LEFT_ALT:
				return "LALT";
			case GLFW_KEY_LEFT_CONTROL:
				return "LCTRL";
			case GLFW_KEY_TAB:
				return "TAB";
			case GLFW_KEY_ESCAPE:
				return "ESC";
		}
		return "UNKNOWN";
	}

	CINO_INLINE std::string KeyBinding::mod_name(int modifier)
	{
		switch (modifier)
		{
			case GLFW_MOD_ALT:
				return "ALT";
			case GLFW_MOD_CAPS_LOCK:
				return "CAPS_LOCK";
			case GLFW_MOD_CONTROL:
				return "CTRL";
			case GLFW_MOD_NUM_LOCK:
				return "NUM_LOCK";
			case GLFW_MOD_SHIFT:
				return "SHIFT";
			case GLFW_MOD_SUPER:
				return "SUPER";
		}
		return "UNKNOWN";
	}

	CINO_INLINE std::string KeyBinding::mod_names(int modifiers)
	{
		static constexpr std::array<int, 6> flags{ GLFW_MOD_ALT, GLFW_MOD_CAPS_LOCK, GLFW_MOD_CONTROL, GLFW_MOD_NUM_LOCK, GLFW_MOD_SHIFT, GLFW_MOD_SUPER };
		std::string out{};
		for (int flag : flags)
		{
			if (modifiers & flag)
			{
				if (!out.empty())
				{
					out += '+';
				}
				out += mod_name(flag);
			}
		}
		return out;
	}

	CINO_INLINE std::string KeyBinding::name() const
	{
		if (modifiers)
		{
			return mod_names(modifiers) + '+' + key_name(key);
		}
		return key_name(key);
	}

	CINO_INLINE void KeyBindings::print(int key, const char* desc)
	{
		if (key != no_key)
		{
			print_binding(KeyBinding::key_name(key).c_str(), desc);
		}
	}

	CINO_INLINE void KeyBindings::print(KeyBinding binding, const char* desc)
	{
		if (binding != no_key_binding)
		{
			print_binding(binding.name().c_str(), desc);
		}
	}

	CINO_INLINE void KeyBindings::print() const
	{
		print(toggle_sidebar, "toggle sidebar");
		print(toggle_axes, "toggle axes");
		print(toggle_ortho, "toggle perspective/orthographic camera");
		print(reset_camera, "reset camera");
		print(look_at_center, "look at center");
		print(store_camera, "copy camera to clipboard");
		print(restore_camera, "restore camera from clipboard");
		print(camera_faster, "move camera faster (hold down)");
		print(camera_slower, "move camera slower (hold down)");
		print(camera_inplace_zoom, "change fov instead of moving forward when zooming (hold down)");
		print(camera_inplace_rotation, "rotate camera around itself instead of the center (hold down)");
		print(camera_look_at_minus_x, "look at -x");
		print(camera_look_at_minus_y, "look (almost) at -y");
		print(camera_look_at_minus_z, "look at -z");
		print(camera_look_at_plus_x, "look at +x");
		print(camera_look_at_plus_y, "look (almost) at +y");
		print(camera_look_at_plus_z, "look at +z");
		if (pan_with_arrow_keys)
		{
			print_binding("ARROWS", "pan");
		}
		if (pan_and_zoom_with_numpad_keys)
		{
			print_binding("NUMPAD", "pan and zoom");
		}
	}

	CINO_INLINE void MouseBindings::print(int button, const char* desc)
	{
		if (button != no_button)
		{
			print_binding(MouseBindings::button_name(button).c_str(), desc);
		}
	}

	CINO_INLINE void MouseBindings::print() const
	{

		print(camera_pan, "pan (drag)");
		print(camera_zoom, "zoom (drag)");
		print(camera_rotate, "rotate camera (drag)");
		if (zoom_with_wheel)
		{
			print_binding("MOUSE_WHEEL", "zoom");
		}
	}

	CINO_INLINE std::string MouseBindings::button_name(int button)
	{
		switch (button)
		{
			case GLFW_MOUSE_BUTTON_LEFT:
				return "MOUSE_LEFT";
			case GLFW_MOUSE_BUTTON_MIDDLE:
				return "MOUSE_MIDDLE";
			case GLFW_MOUSE_BUTTON_RIGHT:
				return "MOUSE_RIGHT";
			default:
				return "UNKNOWN";
		}
	}

	CINO_INLINE void print_binding(const char* binding, const char* desc)
	{
		static constexpr int bindingMaxLen{ 20 };
		std::cout << std::setw(bindingMaxLen) << binding << ": " << desc << '\n';
	}

}