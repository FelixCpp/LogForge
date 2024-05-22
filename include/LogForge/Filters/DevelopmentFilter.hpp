#pragma once

#include "../LogFilter.hpp"

namespace LogForge
{

	class DevelopmentFilter final : public LogFilter
	{
	public:

		constexpr explicit DevelopmentFilter(const Severity minSeverity = DefaultMinSeverity) noexcept :
			LogFilter(minSeverity)
		{}

		[[nodiscard]] constexpr bool Filter(const LogEvent& event) const override
		{
		#ifdef NDEBUG
			return false
		#else
			return event.Severity >= MinSeverity;
		#endif
		}

	};

}