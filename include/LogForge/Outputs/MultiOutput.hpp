#pragma once

#include "../LogOutput.hpp"

#include <vector>
#include <memory>

namespace LogForge
{
	class MultiOutput final : public LogOutput
	{
	public:

		explicit MultiOutput(std::vector<std::unique_ptr<LogOutput>> outputs) noexcept :
			m_Outputs(NormalizeOutputs(std::move(outputs)))
		{}

		void Output(const OutputEvent& event) const override
		{
			for (const auto& output : m_Outputs)
			{
				output->Output(event);
			}
		}

	private:

		static std::vector<std::unique_ptr<LogOutput>> NormalizeOutputs(std::vector<std::unique_ptr<LogOutput>> outputs)
		{
			std::vector<std::unique_ptr<LogOutput>> normalizedOutputs;
			normalizedOutputs.reserve(outputs.size());

			for (auto& output : outputs)
			{
				if (output)
				{
					normalizedOutputs.push_back(std::move(output));
				}
			}

			return normalizedOutputs;
		}

		std::vector<std::unique_ptr<LogOutput>> m_Outputs;

	};
}