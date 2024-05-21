#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <source_location>
#include <unordered_map>
#include <ranges>
#include <functional>

namespace LogForge
{
	///
	/// Enum class for the severity of the log message
	///
	enum class Severity
	{
		Trace,		///< Trace level
		Debug,		///< Debug level
		Info,		///< Information level
		Warning,	///< Warning level
		Error,		///< Error level
		Fatal,		///< Fatal level
	};


	using Line = std::wstring;
	using Lines = std::vector<Line>;
	using Clock = std::chrono::system_clock;
	using TimePoint = std::chrono::time_point<Clock>;
	using SourceLocation = std::source_location;

	///
	/// A structure that represents a log event.
	///	It contains
	///		the severity of the event,
	///		the message,
	///		the time point when the event was created and
	///		the source location where the event was created.
	///
	struct LogEvent
	{
		/// The severity of the event. This is used to filter the events.
		Severity Severity;

		/// The message of the event. This is the actual log message.
		Line Message;

		/// The time point when the event was created.
		TimePoint TimePoint;

		/// The source location where the event was created.
		SourceLocation SourceLocation;
	};

	///
	/// A structure that represents an output event.
	///
	struct OutputEvent
	{
		/// The lines that are to be written to the output.
		Lines Lines;

		/// The original log event that was used to create this output event.
		LogEvent Origin;
	};

	/// Type interface for all log filters.
	class LogFilter
	{
	public:

		virtual ~LogFilter() = default;

		/// This method should return true if the log event should be logged.
		[[nodiscard]] virtual bool ShouldLog(const LogEvent& logEvent) const = 0;

	};

	///
	/// Forward declaration of LogEvent
	///
	template <typename T>
	concept log_filter = requires(T logFilter, const LogEvent& logEvent)
	{
		/// This requirement ensures that the log filter can be called with a severity and returns a bool.
		{ logFilter.ShouldLog(logEvent) } -> std::convertible_to<bool>;

		/// This requirement ensures that the log filter is derived from the LogFilter interface.
		std::derived_from<T, LogFilter>;
	};

	/// Type interface for all log outputs.
	class LogOutput
	{
	public:

		virtual ~LogOutput() = default;

		/// This method should write the output event to the output.
		virtual void Write(const OutputEvent& event) const = 0;

	};

	///
	/// A concept that defines the requirements for a log output.
	///
	template <typename T>
	concept log_output = requires(T logOutput, const OutputEvent& event)
	{
		/// This requirement ensures that the log output can be called with an output event.
		///	Most likely this will write the event to a file or the console.
		{ logOutput.Write(event) };

		/// This requirement ensures that the log output is derived from the LogOutput interface.
		std::derived_from<T, LogOutput>;
	};

	/// Type interface for all log printers.
	struct LogPrinter
	{
		virtual ~LogPrinter() = default;

		/// This method should return the lines that are to be written to the output for the given log event.
		[[nodiscard]] virtual Lines Print(const LogEvent& logEvent) const = 0;
	};

	///
	/// A concept that defines the requirements for a log printer.
	///
	template <typename T>
	concept log_printer = requires(T logPrinter, const LogEvent & event)
	{
		/// This requirement ensures that the log printer can be called with a log event.
		{ logPrinter.Print(event) } -> std::convertible_to<Lines>;

		/// This requirement ensures that the log printer is derived from the LogPrinter interface.
		std::derived_from<T, LogPrinter>;
	};


	/// Namespace for all log filters.
	namespace Filters
	{
		namespace Development
		{
			/// This log filter implementation produces different results in debug and release builds.
			///		- If debug mode is enabled, the filter will ignore all log events with a severity lower than the minimum severity.
			///		- If debug mode is disabled, the filter will ignore all log events.
			struct DevelopmentFilter final : LogFilter
			{
				/// Construct a new [DevelopmentFilter] with the specified minimum [Severity].
				constexpr explicit DevelopmentFilter(const Severity minimumSeverity) noexcept
					: MinimumSeverity(minimumSeverity)
				{
				}

				/// Check if the log event should be logged.
				[[nodiscard]] constexpr bool ShouldLog(const LogEvent& logEvent) const override
				{
				#ifdef NDEBUG
					return false;
				#else
					return logEvent.Severity >= MinimumSeverity;
				#endif
				}

				/// The minimum severity that is required for a log event to be logged.
				Severity MinimumSeverity;
			};
		}
		namespace Production
		{

			/// This log filter implementation will only log events with a severity greater than or equal to the minimum severity.
			struct ProductionFilter final : LogFilter
			{
				/// Construct a new [ProductionFilter] with the specified minimum [Severity].
				constexpr explicit ProductionFilter(const Severity minimumSeverity) noexcept
					: MinimumSeverity(minimumSeverity)
				{
				}

				/// Check if the log event should be logged.
				[[nodiscard]] constexpr bool ShouldLog(const LogEvent& logEvent) const override
				{
					return logEvent.Severity >= MinimumSeverity;
				}

				/// The minimum severity that is required for a log event to be logged.
				Severity MinimumSeverity;
			};
		}
	}

	/// Namespace for all log outputs.
	namespace Output
	{
		namespace Stream
		{
			/// This log output implementation writes the log events to a given output stream.
			class StreamOutput final : public LogOutput
			{
			public:

				/// Construct a new [StreamOutput] with the specified output stream.
				explicit StreamOutput(std::wostream& stream) noexcept
					: m_Stream(&stream)
				{
				}

				/// Write all lines of the output event to the stream.
				void Write(const OutputEvent& event) const override
				{
					for (const auto& line : event.Lines)
					{
						*m_Stream << line << std::endl;
					}
				}

			private:

				/// A pointer to the output stream.
				std::wostream* m_Stream;

			};
		}
	}

	/// Namespace for all log printers.
	namespace Printers
	{
		namespace Messaged
		{
			/// This log printer implementation prints the message of the log event.
			class MessagePrinter final : public LogPrinter
			{
			public:

				constexpr MessagePrinter() noexcept = default;

				/// Convert the message of the log event to a single line.
				[[nodiscard]] Lines Print(const LogEvent& logEvent) const override
				{
					return { logEvent.Message };
				}

				template <typename PrinterBuilder>
				[[nodiscard]] constexpr auto operator >> (PrinterBuilder builder) const noexcept
				{
					return builder.Build(*this);
				}

			};
		}
		namespace Prefixed
		{
			/// Type definition for the severity prefixes.
			using SeverityPrefixes = std::unordered_map<Severity, Line>;

			/// The default severity prefixes.
			inline static const SeverityPrefixes DefaultSeverityPrefixes = {
				{ Severity::Trace, L"[TRACE]" },
				{ Severity::Debug, L"[DEBUG]" },
				{ Severity::Info, L"[INFO]" },
				{ Severity::Warning, L"[WARNING]" },
				{ Severity::Error, L"[ERROR]" },
				{ Severity::Fatal, L"[FATAL]" }
			};

			/// This log printer implementation prints each line with a given prefix for the specified severity.
			template <log_printer Printer>
			class PrefixedPrinter final : public LogPrinter
			{
			public:

				/// Construct a new [PrefixedPrinter] with the specified printer and severity prefixes.
				constexpr explicit PrefixedPrinter(Printer printer, SeverityPrefixes prefixes) noexcept :
					m_RealPrinter(std::move(printer)),
					m_SeverityPrefixes(std::move(prefixes)),
					m_LongestPrefixLength(GetLongestPrefixLength(m_SeverityPrefixes | std::ranges::views::values))
				{
				}

				/// Print the log event with the severity prefix in front.
				[[nodiscard]] Lines Print(const LogEvent& logEvent) const override
				{
					const auto printedLines = m_RealPrinter.Print(logEvent);
					const auto prefix = GetPrefixForSeverity(logEvent.Severity);
					if (prefix == std::nullopt) return printedLines;

					const auto spacing = m_LongestPrefixLength - prefix.value().length();
					const auto padding = Line(spacing, L' ');
					const auto leading = prefix.value() + padding + L' ';
					const auto lines = printedLines | std::views::transform([&leading](const Line& line)
					{
						return leading + line;
					});

					return { lines.begin(), lines.end() };
				}

				template <typename PrinterBuilder>
				[[nodiscard]] constexpr auto operator >> (PrinterBuilder builder) const noexcept
				{
					return builder.Build(*this);
				}

			private:

				/// Get the associated prefix for the given severity.
				[[nodiscard]] std::optional<Line> GetPrefixForSeverity(const Severity severity) const
				{
					const auto it = m_SeverityPrefixes.find(severity);
					if (it != m_SeverityPrefixes.end())
					{
						return it->second;
					}

					return std::nullopt;
				}

				[[nodiscard]] constexpr static std::size_t GetLongestPrefixLength(const auto prefixes)
				{
					return std::ranges::max(prefixes | std::ranges::views::transform([](const Line& prefix)
					{
						return prefix.length();
					}));
				}

				/// The real printer that is used to print the log event.
				Printer m_RealPrinter;

				/// The severity prefixes that are used to prefix the log messages.
				SeverityPrefixes m_SeverityPrefixes;

				/// The length of the longest prefix.
				std::size_t m_LongestPrefixLength;

			};

			class PrefixedPrinterBuilder
			{
			public:

				explicit PrefixedPrinterBuilder(SeverityPrefixes prefixes) :
					m_SeverityPrefixes(std::move(prefixes))
				{
				}

				template <log_printer PrevPrinter>
				[[nodiscard]] constexpr auto Build(PrevPrinter printer) const noexcept
				{
					return PrefixedPrinter(std::move(printer), m_SeverityPrefixes);
				}

			private:

				SeverityPrefixes m_SeverityPrefixes;

			};
		}
		namespace Colored
		{
			/// Type definition for the severity colors.
			using SeverityColors = std::unordered_map<Severity, Line>;

			/// Default severity colors.
			inline static const SeverityColors DefaultSeverityColors = {
				{ Severity::Trace, L"\033[37m" },
				{ Severity::Debug, L"\033[36m" },
				{ Severity::Info, L"\033[32m" },
				{ Severity::Warning, L"\033[33m" },
				{ Severity::Error, L"\033[31m" },
				{ Severity::Fatal, L"\033[35m" }
			};

			template <log_printer Printer>
			class ColoredPrinter final : public LogPrinter
			{
			public:

				/// Construct a new [ColoredPrinter] with the specified printer and severity colors.
				explicit ColoredPrinter(Printer printer, SeverityColors colors = DefaultSeverityColors) noexcept :
					m_RealPrinter(std::move(printer)),
					m_SeverityColors(std::move(colors))
				{
				}

				/// Print the log event with the color for the specified severity.
				[[nodiscard]] Lines Print(const LogEvent& logEvent) const override
				{
					const auto printedLines = m_RealPrinter.Print(logEvent);
					const auto color = GetColorForSeverity(logEvent.Severity);
					if (color == std::nullopt) return printedLines;

					const auto lines = printedLines | std::views::transform([color](const Line& line)
					{
						return Colorize(line, color.value());
					});

					return { lines.begin(), lines.end() };
				}

				template <typename PrinterBuilder>
				[[nodiscard]] constexpr auto operator >> (PrinterBuilder builder) const noexcept
				{
					return builder.Build(*this);
				}

			private:

				/// Get the associated color for the given severity.
				[[nodiscard]] std::optional<Line> GetColorForSeverity(const Severity severity) const
				{
					const auto it = m_SeverityColors.find(severity);
					if (it != m_SeverityColors.end())
					{
						return it->second;
					}

					return std::nullopt;
				}

				/// Colorize the given line with the given color by surrounding it with the color codes.
				///	The color mode will be reset after each line.
				[[nodiscard]] static Line Colorize(const Line& line, const Line& color)
				{
					static constexpr auto Reset = L"\033[0m";
					return color + line + Reset;
				}

				/// The real printer that is used to print the log event.
				Printer m_RealPrinter;

				/// The severity colors that are used to colorize the log messages.
				SeverityColors m_SeverityColors;

			};

			class ColoredPrinterBuilder
			{
			public:

				explicit ColoredPrinterBuilder(SeverityColors prefixes) :
					m_SeverityColors(std::move(prefixes))
				{
				}

				template <log_printer PrevPrinter>
				[[nodiscard]] constexpr auto Build(PrevPrinter printer) const noexcept
				{
					return ColoredPrinter(std::move(printer), m_SeverityColors);
				}

			private:

				SeverityColors m_SeverityColors;

			};
		}
		namespace Timestamped
		{
			inline static constexpr auto DefaultTimePrefix = L"Timestamp:";
			inline static constexpr auto DefaultTimeFormat = L"%Y-%m-%d %H:%M:%S";

			template <log_printer Printer>
			class TimestampPrinter final : public LogPrinter
			{
			public:

				constexpr explicit TimestampPrinter(Printer printer, Line format = DefaultTimeFormat, Line prefix = DefaultTimePrefix) noexcept :
					m_RealPrinter(printer),
					m_Format(std::move(format)),
					m_Prefix(std::move(prefix))
				{}

				[[nodiscard]] Lines Print(const LogEvent& logEvent) const override
				{
					const auto printedLines = m_RealPrinter.Print(logEvent);
					const auto timestamp = FormatTime(logEvent.TimePoint).value_or(L"<Invalid Time>");
					const auto timeline = m_Prefix + L' ' + timestamp;

					Lines output;
					output.reserve(printedLines.size() + 1);
					output.insert(output.begin(), timeline);
					output.insert(output.end(), printedLines.cbegin(), printedLines.cend());
					return output;
				}

				template <typename PrinterBuilder>
				[[nodiscard]] constexpr auto operator >> (PrinterBuilder builder) const noexcept
				{
					return builder.Build(*this);
				}

			private:

				[[nodiscard]] std::optional<Line> FormatTime(const TimePoint& timePoint) const
				{
					const std::time_t time = Clock::to_time_t(timePoint);
					tm timeInfo = {};
					const auto error = localtime_s(&timeInfo, &time);
					if (error != 0) return std::nullopt;

					std::wostringstream wss;
					wss << std::put_time(&timeInfo, m_Format.data());
					return wss.str();
				}

				Printer m_RealPrinter;
				Line m_Format;
				Line m_Prefix;

			};

			class TimestampPrinterBuilder
			{
			public:

				explicit TimestampPrinterBuilder(Line format, Line prefix) :
					m_Format(std::move(format)),
					m_Prefix(std::move(prefix))
				{
				}

				template <log_printer PrevPrinter>
				[[nodiscard]] constexpr auto Build(PrevPrinter printer) const noexcept
				{
					return TimestampPrinter(std::move(printer), m_Format, m_Prefix);
				}

			private:

				Line m_Format;
				Line m_Prefix;

			};
		}
		namespace Located
		{
			inline static constexpr auto DefaultLocationPrefix = L"Location:";
			using SourceLocationFormatter = std::function<Line(SourceLocation)>;
			[[nodiscard]] inline Line DefaultSourceLocationFormatter(const SourceLocation& location)
			{
				std::wostringstream wss;
				wss << location.file_name() << L':' << location.line() << L' ' << location.function_name();
				return wss.str();
			}

			template <log_printer Printer>
			class LocationPrinter final : public LogPrinter
			{
			public:

				constexpr explicit LocationPrinter(Printer printer, SourceLocationFormatter formatter = &DefaultSourceLocationFormatter, Line prefix = DefaultLocationPrefix) noexcept :
					m_RealPrinter(printer),
					m_Formatter(std::move(formatter)),
					m_Prefix(std::move(prefix))
				{}

				[[nodiscard]] Lines Print(const LogEvent& logEvent) const override
				{
					const auto printedLines = m_RealPrinter.Print(logEvent);
					if (m_Formatter == nullptr) return printedLines;

					const auto location = m_Formatter(logEvent.SourceLocation);
					const auto timeline = m_Prefix + L' ' + location;

					Lines output;
					output.reserve(printedLines.size() + 1);
					output.insert(output.begin(), timeline);
					output.insert(output.end(), printedLines.cbegin(), printedLines.cend());
					return output;
				}

				template <typename PrinterBuilder>
				[[nodiscard]] constexpr auto operator >> (PrinterBuilder builder) const noexcept
				{
					return builder.Build(*this);
				}

			private:

				Printer m_RealPrinter;
				SourceLocationFormatter m_Formatter;
				Line m_Prefix;

			};

			class LocationPrinterBuilder
			{
			public:

				explicit LocationPrinterBuilder(SourceLocationFormatter format, Line prefix) :
					m_Formatter(std::move(format)),
					m_Prefix(std::move(prefix))
				{
				}

				template <log_printer PrevPrinter>
				[[nodiscard]] constexpr auto Build(PrevPrinter printer) const noexcept
				{
					return LocationPrinter(std::move(printer), m_Formatter, m_Prefix);
				}

			private:

				SourceLocationFormatter m_Formatter;
				Line m_Prefix;

			};
		}
		namespace Boxed
		{
			template <log_printer Printer>
			class BoxedPrinter final : public LogPrinter
			{
			public:
				
				static constexpr auto TopLeft = L'┌';
				static constexpr auto TopRight = L'┐';
				static constexpr auto BottomLeft = L'└';
				static constexpr auto BottomRight = L'┘';
				static constexpr auto Vertical = L'│';
				static constexpr auto Horizontal = L'─';

				constexpr explicit BoxedPrinter(Printer printer) noexcept :
					m_RealPrinter(printer)
				{}

				[[nodiscard]] Lines Print(const LogEvent& logEvent) const override
				{
					const auto printedLines = m_RealPrinter.Print(logEvent);
					const auto longestLineLength = GetLongestLineLength(printedLines);

					const auto horizontalLine = Line(longestLineLength, Horizontal);
					const auto upperLine = TopLeft + horizontalLine + TopRight;
					const auto lowerLine = BottomLeft + horizontalLine + BottomRight;

					const auto boxedLines = printedLines | std::views::transform([longestLineLength](const Line& line)
					{
						const auto padding = longestLineLength - line.size();
						return Vertical + line + Line(padding, L' ') + Vertical;
					});

					Lines output;
					output.reserve(printedLines.size() + 2);
					output.insert(output.begin(), upperLine);
					output.insert(output.end(), boxedLines.begin(), boxedLines.end());
					output.insert(output.end(), lowerLine);
					return output;
				}

				template <typename PrinterBuilder>
				[[nodiscard]] constexpr auto operator >> (PrinterBuilder builder) const noexcept
				{
					return builder.Build(*this);
				}

			private:

				[[nodiscard]] static std::size_t GetLongestLineLength(const Lines& lines)
				{
					return std::ranges::max(lines | std::ranges::views::transform([](const Line& line)
					{
						return line.length();
					}));
				}

				Printer m_RealPrinter;

			};

			class BoxedPrinterBuilder
			{
			public:

				constexpr explicit BoxedPrinterBuilder() noexcept = default;

				template <log_printer PrevPrinter>
				[[nodiscard]] constexpr auto Build(PrevPrinter printer) const noexcept
				{
					return BoxedPrinter(std::move(printer));
				}

			};
		}
	}

	constexpr auto Messages()
	{
		return Printers::Messaged::MessagePrinter();
	}

	inline auto Prefixed(
		Printers::Prefixed::SeverityPrefixes prefixes = Printers::Prefixed::DefaultSeverityPrefixes
	) -> decltype(Printers::Prefixed::PrefixedPrinterBuilder(std::move(prefixes)))
	{
		return Printers::Prefixed::PrefixedPrinterBuilder(std::move(prefixes));
	}

	inline auto Colorized(
		Printers::Colored::SeverityColors colors = Printers::Colored::DefaultSeverityColors
	) -> decltype(Printers::Colored::ColoredPrinterBuilder(std::move(colors)))
	{
		return Printers::Colored::ColoredPrinterBuilder(std::move(colors));
	}

	inline auto Timestamped(
		Line format = Printers::Timestamped::DefaultTimeFormat,
		Line prefix = Printers::Timestamped::DefaultTimePrefix
	) -> decltype(Printers::Timestamped::TimestampPrinterBuilder(std::move(format), std::move(prefix)))
	{
		return Printers::Timestamped::TimestampPrinterBuilder(std::move(format), std::move(prefix));
	}

	inline auto Located(
		Printers::Located::SourceLocationFormatter formatter = Printers::Located::DefaultSourceLocationFormatter,
		Line prefix = Printers::Located::DefaultLocationPrefix
	) -> decltype(Printers::Located::LocationPrinterBuilder(std::move(formatter), std::move(prefix)))
	{
		return Printers::Located::LocationPrinterBuilder(std::move(formatter), std::move(prefix));
	}

	inline auto Boxed() -> decltype(Printers::Boxed::BoxedPrinterBuilder())
	{
		return Printers::Boxed::BoxedPrinterBuilder();
	}

	/// Type interface for all loggers.
	class Logger
	{
	public:

		virtual ~Logger() = default;

		/// This method should log the given log event.
		virtual void Log(const LogEvent& logEvent) const = 0;

		/// Construct a log event using the given parameters as attributes.
		void Log(const Severity severity, const Line& message, const TimePoint& timePoint = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log(LogEvent { severity, message, timePoint, location });
		}

		/// Log a message with the severity [Severity::Trace].
		void Trace(const Line& message, const TimePoint& timePoint = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log(Severity::Trace, message, timePoint, location);
		}

		/// Log a message with the severity [Severity::Debug].
		void Debug(const Line& message, const TimePoint& timePoint = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log(Severity::Debug, message, timePoint, location);
		}

		/// Log a message with the severity [Severity::Info].
		void Info(const Line& message, const TimePoint& timePoint = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log(Severity::Info, message, timePoint, location);
		}

		/// Log a message with the severity [Severity::Warning].
		void Warning(const Line& message, const TimePoint& timePoint = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log(Severity::Warning, message, timePoint, location);
		}

		/// Log a message with the severity [Severity::Error].
		void Error(const Line& message, const TimePoint& timePoint = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log(Severity::Error, message, timePoint, location);
		}

		/// Log a message with the severity [Severity::Fatal].
		void Fatal(const Line& message, const TimePoint& timePoint = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log(Severity::Fatal, message, timePoint, location);
		}

	};

	/// Default logger implementation that logs the events to the output if the filter allows it.
	template <log_filter Filter, log_output Output, log_printer Printer>
	class DefaultLogger final : public Logger
	{
	public:

		/// Construct a new [DefaultLogger] with the specified filter, output and printer.
		explicit DefaultLogger(
			Filter filter,
			Output output,
			Printer printer
		) noexcept :
			m_Filter(std::move(filter)),
			m_Output(std::move(output)),
			m_Printer(std::move(printer))
		{
		}

		void Log(const LogEvent& logEvent) const override
		{
			if (m_Filter.ShouldLog(logEvent))
			{
				const auto lines = m_Printer.Print(logEvent);
				const auto outputEvent = OutputEvent { lines, logEvent };
				m_Output.Write(outputEvent);
			}
		}

	private:

		Filter m_Filter;
		Output m_Output;
		Printer m_Printer;

	};
}
