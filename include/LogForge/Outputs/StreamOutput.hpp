#pragma once

#include "../LogOutput.hpp"

#include <ostream>

namespace LogForge
{
	class StreamOutput final : public LogOutput
	{
	public:

		explicit StreamOutput(std::wostream& stream) noexcept :
			m_Stream(&stream)
		{}
		
		void Output(const OutputEvent& event) const override
		{
			for (const auto& line : event.Lines)
			{
				*m_Stream << line << std::endl;
			}
		}

	private:

		std::wostream* m_Stream;

	};
}
