#pragma once

#include "LogPrinter.hpp"

namespace LogForge
{

	/// Base class for all log filters
	class LogFilter
	{
	public:

		/// Default minimum severity
		static constexpr auto DefaultMinSeverity = Severity::Trace;

	public:

		/// Constructor that initializes the minimum severity
		constexpr explicit LogFilter(const Severity minSeverity = DefaultMinSeverity) noexcept :
			MinSeverity(minSeverity)
		{}

		virtual ~LogFilter() = default;
		[[nodiscard]] virtual bool Filter(const LogEvent& event) const = 0;

	public:

		Severity MinSeverity;

	};

}