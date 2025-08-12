#pragma once
#ifndef __Keyboard__
#define __Keyboard__

#include <queue>
#include <bitset>
#include <optional>

class Keyboard
{
	friend class Window;
public:
	class Event
	{
		// internal class thingy
	public:
		enum class Type
		{
			Press,
			Release,
			Invalid
		};
	private:
		Type type;
		unsigned char code;
	public:
		Event()
			: type(Type::Invalid), code(0u)
		{}
		Event(Type type, unsigned char code) noexcept
			: type(type), code(code)
		{}
		bool IsPress() const noexcept
		{
			return type == Type::Press;
		}
		bool IsRelease() const noexcept
		{
			return type == Type::Release;
		}
		bool IsValid() const noexcept
		{
			return type != Type::Invalid;
		}
		unsigned char GetCode() const noexcept
		{
			return code;
		}
	};

	Keyboard() = default;
	// - Delete the copy of the object.
	Keyboard(const Keyboard&) = delete;
	// - Delete the copy assignment
	Keyboard& operator=(const Keyboard&) = delete;
	// key event stuff
	bool KeyIsPressed(unsigned char Keycode) const noexcept;
	std::optional<Keyboard::Event> ReadKey() noexcept; // pull an event out of the event queue.
	bool KeyIsEmpty() const noexcept; // check if there is any even tin the event queue.
	void FlushKey() noexcept; // flush the event queue.
	// char event stuff
	char ReadChar() noexcept;
	bool CharIsEmpty() const noexcept;
	void FlushChar() noexcept;
	void Flush() noexcept; // will flush the char and the key queues
	// autorepeat control 
	void EnableAutoRepeat() noexcept;
	void DisableAutoRepeat() noexcept;
	bool AutoRepeatIsEnabled() const noexcept;

private:
	// used by the window 
	void OnKeyPressed(unsigned char Keycode) noexcept;
	void OnKeyReleased(unsigned char Keycode) noexcept;
	void OnChar(char Character) noexcept;
	void ClearState() noexcept; // will clear Keystates bitset
	template<typename T>
	/*
	*  - Will trim the buffer back to 16u, if it overflows.
	*/
	static void TrimBuffer(std::queue<T>& buffer) noexcept;

private:
	static constexpr unsigned int nKeys = 256u;
	static constexpr unsigned int bufferSize = 16u;
	bool autorepeatEnabled = false;

private:
	std::bitset<nKeys> Keystates;
	std::queue<Event> Keybuffer;
	std::queue<char> charBuffer;
};


// template<typename T>
// inline void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept;

#endif // !__Keyboard__
