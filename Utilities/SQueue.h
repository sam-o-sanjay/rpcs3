#pragma once

static const volatile bool sq_no_wait = true;

template<typename T, u32 SQSize = 666>
class SQueue
{
	std::mutex m_mutex;
	u32 m_pos;
	u32 m_count;
	T m_data[SQSize];

public:
	SQueue()
		: m_pos(0)
		, m_count(0)
	{
	}

	const u32 GetSize() const
	{
		return SQSize;
	}

	bool Push(const T& data, const volatile bool* do_exit)
	{
		while (true)
		{
			if (m_count >= SQSize)
			{
				if (Emu.IsStopped() || do_exit && *do_exit)
				{
					return false;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

			{
				std::lock_guard<std::mutex> lock(m_mutex);

				if (m_count >= SQSize) continue;

				m_data[(m_pos + m_count++) % SQSize] = data;

				return true;
			}
		}
	}

	bool Pop(T& data, const volatile bool* do_exit)
	{
		while (true)
		{
			if (!m_count)
			{
				if (Emu.IsStopped() || do_exit && *do_exit)
				{
					return false;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

			{
				std::lock_guard<std::mutex> lock(m_mutex);

				if (!m_count) continue;

				data = m_data[m_pos];
				m_pos = (m_pos + 1) % SQSize;
				m_count--;

				return true;
			}
		}
	}

	u32 GetCount()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return m_count;
	}

	u32 GetCountUnsafe()
	{
		return m_count;
	}

	bool IsEmpty()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		return !m_count;
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_count = 0;
	}

	T& Peek(const volatile bool* do_exit, u32 pos = 0)
	{
		while (true)
		{
			if (m_count <= pos)
			{
				if (Emu.IsStopped() || do_exit && *do_exit)
				{
					break;
				}
				
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}

			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (m_count > pos)
				{
					break;
				}
			}
		}
		return m_data[(m_pos + pos) % SQSize];
	}
};
