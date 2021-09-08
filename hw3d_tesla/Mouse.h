#pragma once
#include <queue>
#include <optional>

class Mouse
{
	friend class Window;
public:
	struct RawDelta
	{
		long x;
		long y;
	};
	class Event
	{
	public:
		enum class Type
		{
			LPress,
			LRelease,
			RPress,
			RRelease,
			MiddlePress,
			MiddleRelease,
			WheelUp,
			WheelDown,
			Move,
			Enter,
			Leave,
			Invalid
		};
	private:
		Type type;
		bool leftIsPressed;
		bool rightIsPressed;
		bool middleIsPressed;
		int x;
		int y;
	public:
		Event() noexcept
			:
			type(Type::Invalid),
			leftIsPressed(false),
			rightIsPressed(false),
			middleIsPressed(false),
			x(0),
			y(0)
		{}
		Event(Type type, const Mouse& parent)
			:
			type(type),
			leftIsPressed(parent.leftIsPressed),
			rightIsPressed(parent.rightIsPressed),
			middleIsPressed(parent.middleIsPressed),
			x(parent.x),
			y(parent.y)
		{}
		bool IsValid() const noexcept
		{
			return type != Type::Invalid;
		}
		Type GetType() const noexcept
		{
			return type;
		}
		std::pair<int, int> GetPos() const noexcept
		{
			return { x,y };
		}
		int GetPosX() const noexcept
		{
			return x;
		}
		int GetPosY() const noexcept
		{
			return y;
		}
		bool LeftIsPressed() const noexcept
		{
			return leftIsPressed;
		}
		bool RightIsPressed() const noexcept
		{
			return rightIsPressed;
		}
		bool MiddleIsPressed() const noexcept
		{
			return middleIsPressed;
		}
	};
public:
	Mouse() = default;
	Mouse(const Mouse&) = delete;
	Mouse& operator = (const Mouse&) = delete;
	std::pair<int, int> GetPos() const noexcept;
	std::pair<float, float> GetPosF() const noexcept;
	std::optional<RawDelta> ReadRawDelta() noexcept;
	int GetPosX() const noexcept;
	int GetPosY() const noexcept;
	float GetPosXf() const noexcept;
	float GetPosYf() const noexcept;
	bool LeftIsPressed() const noexcept;
	bool RightIsPressed() const noexcept;
	bool IsInWindow() const noexcept;
	Mouse::Event Read() noexcept;
	bool IsEmpty() const noexcept;
	void Flush() noexcept;
	void EnableRawInput() noexcept;
	void DisableRawInput() noexcept;
	bool IsRawInputEnabled() const noexcept;
private:
	void OnMouseMove(int new_x, int new_y) noexcept;
	void OnMouseEnter() noexcept;
	void OnMouseLeave() noexcept;
	void OnLeftIsPressed(int x, int y) noexcept;
	void OnLeftIsReleased(int x, int y) noexcept;
	void OnRightIsPressed(int x, int y) noexcept;
	void OnRightIsReleased(int x, int y) noexcept;
	void OnWheelUp(int x, int y) noexcept;
	void OnWheelDown(int x, int y) noexcept;
	void OnMiddlePress(int x, int y) noexcept;
	void OnMiddleRelease(int x, int y) noexcept;
	void OnWheelDelta(int x, int y, int delta) noexcept;
	void OnRawInputDelta(long acc_x, long acc_y) noexcept;
	void TrimBuffer() noexcept;
	void TrimRawBuffer() noexcept;
private:
	static constexpr unsigned int bufferSize = 16u;
	int x = 0;
	int y = 0;
	bool leftIsPressed = false;
	bool rightIsPressed = false;
	bool middleIsPressed = false;
	bool rawInputEnabled = false;
	bool isInWindow = false;
	int wheelDeltaCarry = 0;
	std::queue<Event> buffer;
	std::queue<RawDelta> rawBuffer;
};