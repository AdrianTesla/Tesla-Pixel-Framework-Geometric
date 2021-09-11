#include "TeslaWin.h"
#include "Mouse.h"
#include "Graphics.h"

std::optional<Mouse::RawDelta> Mouse::ReadRawDelta() noexcept
{
	if (rawBuffer.empty())
	{
		return std::nullopt;
	}
	const RawDelta delta = rawBuffer.front();
	rawBuffer.pop();
	return delta;
}

int Mouse::GetPosX() const noexcept
{
	return x / Graphics::PixelSize;
}

int Mouse::GetPosY() const noexcept
{
	return y / Graphics::PixelSize;
}

float Mouse::GetPosXf() const noexcept
{
	return float(x) / float(Graphics::PixelSize);
}

float Mouse::GetPosYf() const noexcept
{
	return float(y) / float(Graphics::PixelSize);
}

Tesla::Vec2 Mouse::GetPosF() const noexcept
{
	return { GetPosXf(),GetPosYf() };
}

Tesla::Vei2 Mouse::GetPos() const noexcept
{
	return { GetPosX(),GetPosY() };
}

bool Mouse::LeftIsPressed() const noexcept
{
	return leftIsPressed;
}

bool Mouse::RightIsPressed() const noexcept
{
	return rightIsPressed;
}

bool Mouse::IsInWindow() const noexcept
{
	return isInWindow;
}

Mouse::Event Mouse::Read() noexcept
{
	if (buffer.size() > 0u)
	{
		Mouse::Event e = buffer.front();
		buffer.pop();
		return e;
	}
	else
	{
		return Mouse::Event();
	}
}

bool Mouse::IsEmpty() const noexcept
{
	return buffer.empty();
}

void Mouse::Flush() noexcept
{
	buffer = std::queue<Event>();
}

void Mouse::EnableRawInput() noexcept
{
	rawInputEnabled = true;
}

void Mouse::DisableRawInput() noexcept
{
	rawInputEnabled = false;
}

bool Mouse::IsRawInputEnabled() const noexcept
{
	return rawInputEnabled;
}

void Mouse::OnMouseMove(int new_x, int new_y) noexcept
{
	x = new_x;
	y = new_y;

	buffer.push(Mouse::Event(Mouse::Event::Type::Move, *this));
	TrimBuffer();
}

void Mouse::OnMouseEnter() noexcept
{
	isInWindow = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::Enter, *this));
	TrimBuffer();
}

void Mouse::OnMouseLeave() noexcept
{
	isInWindow = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::Leave, *this));
	TrimBuffer();
}

void Mouse::OnLeftIsPressed(int x, int y) noexcept
{
	leftIsPressed = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::LPress, *this));
	TrimBuffer();
}

void Mouse::OnLeftIsReleased(int x, int y) noexcept
{
	leftIsPressed = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::LRelease, *this));
	TrimBuffer();
}

void Mouse::OnRightIsPressed(int x, int y) noexcept
{
	rightIsPressed = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::RPress, *this));
	TrimBuffer();
}

void Mouse::OnRightIsReleased(int x, int y) noexcept
{
	rightIsPressed = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::RRelease, *this));
	TrimBuffer();
}

void Mouse::OnWheelUp(int x, int y) noexcept
{
	buffer.push(Mouse::Event(Mouse::Event::Type::WheelUp, *this));
	TrimBuffer();
}

void Mouse::OnWheelDown(int x, int y) noexcept
{
	buffer.push(Mouse::Event(Mouse::Event::Type::WheelDown, *this));
	TrimBuffer();
}

void Mouse::OnMiddlePress(int x, int y) noexcept
{
	middleIsPressed = true;
	buffer.push(Mouse::Event(Mouse::Event::Type::MiddlePress, *this));
	TrimBuffer();
}

void Mouse::OnMiddleRelease(int x, int y) noexcept
{
	middleIsPressed = false;
	buffer.push(Mouse::Event(Mouse::Event::Type::MiddleRelease, *this));
	TrimBuffer();
}

void Mouse::OnWheelDelta(int x, int y, int delta) noexcept
{
	wheelDeltaCarry += delta;
	while (wheelDeltaCarry >= WHEEL_DELTA)
	{
		wheelDeltaCarry -= WHEEL_DELTA;
		OnWheelUp(x, y);
	}
	while (wheelDeltaCarry <= -WHEEL_DELTA)
	{
		wheelDeltaCarry += WHEEL_DELTA;
		OnWheelDown(x, y);
	}
}

void Mouse::OnRawInputDelta(long acc_x, long acc_y) noexcept
{
	rawBuffer.push({ acc_x,acc_y });
	TrimRawBuffer();
}

void Mouse::TrimBuffer() noexcept
{
	while (buffer.size() > bufferSize)
	{
		buffer.pop();
	}
}

void Mouse::TrimRawBuffer() noexcept
{
	while (rawBuffer.size() > bufferSize)
	{
		rawBuffer.pop();
	}
}
