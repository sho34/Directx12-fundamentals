#include "kbd.h"

bool Keyboard::KeyIsPressed(unsigned char Keycode) const noexcept
{
	return Keystates[Keycode];
}

std::optional<Keyboard::Event> Keyboard::ReadKey() noexcept
{
	if (Keybuffer.size() > 0u)
	{
		Keyboard::Event e = Keybuffer.front();
		Keybuffer.pop();
		return e;
	}

	return {};
}

bool Keyboard::KeyIsEmpty() const noexcept
{
	return Keybuffer.empty();
}

void Keyboard::FlushKey() noexcept
{
	Keybuffer = std::queue<Event>();
}

char Keyboard::ReadChar() noexcept
{
	if (charBuffer.size() > 0u)
	{
		unsigned char charcode = charBuffer.front();
		charBuffer.pop();
		return charcode;
	}
	else
	{
		return 0;
	}
}

bool Keyboard::CharIsEmpty() const noexcept
{
	return charBuffer.empty();
}

void Keyboard::FlushChar() noexcept
{
	// this deconstructs the old queue and replaces it with a new queue.
	charBuffer = std::queue<char>();
}

void Keyboard::Flush() noexcept
{
	FlushChar();
	FlushKey();
}

void Keyboard::EnableAutoRepeat() noexcept
{
	autorepeatEnabled = true;
}

void Keyboard::DisableAutoRepeat() noexcept
{
	autorepeatEnabled = false;
}

bool Keyboard::AutoRepeatIsEnabled() const noexcept
{
	return autorepeatEnabled;
}

void Keyboard::OnKeyPressed(unsigned char Keycode) noexcept
{
	Keystates[Keycode] = true;
	Keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, Keycode));
	TrimBuffer(Keybuffer);
}

void Keyboard::OnKeyReleased(unsigned char Keycode) noexcept
{
	Keystates[Keycode] = false;
	Keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, Keycode));
	TrimBuffer(Keybuffer);
}

void Keyboard::OnChar(char Character) noexcept
{
	charBuffer.push(Character);
	TrimBuffer(charBuffer);
}

void Keyboard::ClearState() noexcept
{
	Keystates.reset();
}

template<typename T>
void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept
{
	while (buffer.size() > bufferSize)
	{
		buffer.pop();
	}
}
