#pragma once

#include "../LogFilter.hpp"

namespace LogForge
{

	class ProductionFilter final : public LogFilter
	{
	public:

		constexpr explicit ProductionFilter(const Severity minSeverity = DefaultMinSeverity) noexcept :
			LogFilter(minSeverity)
		{}

		[[nodiscard]] constexpr bool Filter(const LogEvent& event) const override
		{
			return event.Severity >= MinSeverity;
		}

	};

}