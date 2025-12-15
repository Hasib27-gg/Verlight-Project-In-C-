#ifndef ___VERLIGHT___VM___H___
#define ___VERLIGHT___VM___H___ 0x988cab

#ifndef HAS_INCLUDED_ALL
#    define HAS_INCLUDED_ALL 1
#         include "VerlightCompiler.h"
#         include "MemoryModel.h"
#         include "DeclaratorEngine.h"
#         include "IOEngine.h"
#         include "ArithematicEngine.h"
#         include "RelationalEngine.h"
#         include "LoopEngine.h"
#         include "ListEngine.h"
#endif


class  VerlightVM {
private:
	robin_hood::unordered_map
		<str, std::function<void(std::vector<str>&, str&, SectionMemory&)>>
		all_utilities =
	{
		{"new_i8" , DeclaratorEngine::new_i8},
		{"new_i16" , DeclaratorEngine::new_i16},
		{"new_i32" , DeclaratorEngine::new_i32},
		{"new_i64" , DeclaratorEngine::new_i64},
		{"new_f32" , DeclaratorEngine::new_f32},
		{"new_f64" , DeclaratorEngine::new_f64},
		{"new_fmax" , DeclaratorEngine::new_fmax},
		{"new_str" , DeclaratorEngine::new_str},
		{"new_char" , DeclaratorEngine::new_char},
		{"new_bool" , DeclaratorEngine::new_bool},

		{"reAssign_i8" , DeclaratorEngine::reAssign_i8},
		{"reAssign_i16" , DeclaratorEngine::reAssign_i16},
		{"reAssign_i32" , DeclaratorEngine::reAssign_i32},
		{"reAssign_i64" , DeclaratorEngine::reAssign_i64},
		{"reAssign_f32" , DeclaratorEngine::reAssign_f32},
		{"reAssign_f64" , DeclaratorEngine::reAssign_f64},
		{"reAssign_fmax" , DeclaratorEngine::reAssign_fmax},
		{"reAssign_str" , DeclaratorEngine::reAssign_str},
		{"reAssign_char" , DeclaratorEngine::reAssign_char},
		{"reAssign_bool" , DeclaratorEngine::reAssign_bool},

		{"delete_var" , DeclaratorEngine::delete_var},

		{"print" , IOEngine::print},
		{"println" , IOEngine::println},
		{"flush" , IOEngine::flush},
		{"input" , IOEngine::input},

		{"add" , ArithmeticEngine::sum},
		{"sum" , ArithmeticEngine::sum},
		{"multiply", ArithmeticEngine::product},
		{"product" , ArithmeticEngine::product},
		{"subtract" , ArithmeticEngine::subtract},
		{"divide" , ArithmeticEngine::divide},
		{"floor" , ArithmeticEngine::floor},
		{"ceiling" , ArithmeticEngine::ceiling},
        {"mod" , ArithmeticEngine::mod},
		{"abs" , ArithmeticEngine::abs},
		{"pow" , ArithmeticEngine::pow},

		{"isEqual" , RelationalEngine::isEqual},
		{"isCharsEqual" , RelationalEngine::isCharsEqual},
		{"isGreater" , RelationalEngine::isGreater},
		{"isLess" , RelationalEngine::isLess},
		{"isGreaterEqual" , RelationalEngine::isGreaterEqual},
		{"isLessEqual" , RelationalEngine::isLessEqual},
		{"isNotEqual" , RelationalEngine::isNotEqual},
		{"isNotCharsEqual" , RelationalEngine::isCharsNotEqual},

		{"loop" , LoopEngine::loop },

		{"new_list" , ListEngine::new_list},
		{"reAssign_list" , ListEngine::reAssign_list },
		{"delete_list" , ListEngine::delete_list },
		{"get" , ListEngine::get},
		{"push" , ListEngine::push},
		{"pop" , ListEngine::pop},
		{"print_list" , ListEngine::printList }
	};


	std::vector<___INSTRUCTION___T___>
		pm_selectStartEndCode
		(std::vector<___INSTRUCTION___T___> code, int64_t& endIdx)
	{
		std::vector<___INSTRUCTION___T___> body;
		str identifier = code[endIdx].___PARAMETERS___[0];

		for (int64_t idx = endIdx + 1;
			idx < static_cast<int64_t>(code.size());
			++idx) {
			const auto& ins = code[idx];
			if (ins.___TODO___ == "end" &&
				!ins.___PARAMETERS___.empty() &&
				ins.___PARAMETERS___[0] == identifier) {
				endIdx = idx;
				return body;
			}
			body.push_back(ins);
		}

		throw std::runtime_error("missing matching 'end' for @start " + identifier);
	}

public:
	___UMAP___STRING___VECTOR___INSTRUCTIONS___T___ compiledCode;

	robin_hood::unordered_map<str, SectionMemory> memoryUnion;

	VerlightVM(___UMAP___STRING___VECTOR___INSTRUCTIONS___T___& compiledCode) :
		compiledCode(compiledCode) {
	};

	void buildMemory() {
		for (auto name_instructions : compiledCode) {
			SectionMemory newMemory;
			memoryUnion.insert({ name_instructions.first , newMemory });
		}
	}

	void execute(const str& programName = "main",
		int64_t startIdx = 0,
		int64_t endIdx = -1)
	{
		if (!compiledCode.contains(programName))
		{
			std::cerr << "\n[ERROR_ENGINE] The programName that is causing exception is: ("
				<< programName << ")\n";
			throw std::runtime_error("Can't find the program name!");
		}

		if (!memoryUnion.contains(programName))
		{
			std::cerr << "\n[ERROR_ENGINE] The programName that is causing exception is: ("
				<< programName << ")\n";
			throw std::runtime_error
			("Can't find the program name in memoryUnion. Use buildMemory() if you haven't drawn the blueprint");
		}

		auto& base_instructions = compiledCode[programName];
		SectionMemory& base_memory = memoryUnion[programName];
		int64_t size = static_cast<int64_t>(base_instructions.size());

		if (endIdx == -1) endIdx = size - 1;

		for (int64_t idx = startIdx; idx <= endIdx; ++idx) {

			auto& currentIns = base_instructions[idx];
			str will_this_execute = currentIns.___VM___WILL___EXECUTE___;
			bool target = true;

			if (!will_this_execute.empty() && will_this_execute.front() == '!') {
				target = false;
				will_this_execute = will_this_execute.substr(1);
			}

			if (!will_this_execute.empty() && will_this_execute.front() == '$')
				will_this_execute = base_memory.m_fetchStringalizedValue(will_this_execute);

			if (will_this_execute != "true" && will_this_execute != "false")
				throw std::runtime_error("Unknown boolean type!");

			bool raw_result = (will_this_execute == "true");
			bool actual_result = !(raw_result ^ target);
			if (!actual_result) continue;

			auto& return_address = currentIns.___RETURN___ADDRESS___;

			if (!base_memory.m_containsVariable(return_address) && return_address != "nullptr")
			{
				std::cerr << "\n[ERROR_ENGINE] The return_address that is causing exception is: ("
					<< return_address << ")\n";
				throw std::runtime_error
				("Can't find the return address!");
			}

			if (currentIns.___TODO___ == "start") {

				if (currentIns.___PARAMETERS___.size() != 1)
					throw std::runtime_error("Parameters for @start : () requires a single parameter (iterator_name)!");

				str identifier = currentIns.___PARAMETERS___[0];

				uint64_t loop_start_idx = idx + 1;
				std::vector<___INSTRUCTION___T___> temp = pm_selectStartEndCode(base_instructions, idx);
				uint64_t loop_end_idx = idx - 1;

				str start_name = str("___LOOP___ENGINE___") + identifier + str("___start___");
				str stop_name = str("___LOOP___ENGINE___") + identifier + str("___stop___");
				str step_name = str("___LOOP___ENGINE___") + identifier + str("___step___");

				if (!base_memory.m_containsVariable(start_name) ||
					!base_memory.m_containsVariable(stop_name) ||
					!base_memory.m_containsVariable(step_name))
					throw std::runtime_error("Can't find the value of start or stop or step in the section buffer!");

				int64_t loop_start = std::stoll(makeValueToString(base_memory.m_getVariable(start_name)));
				int64_t loop_stop = std::stoll(makeValueToString(base_memory.m_getVariable(stop_name)));
				int64_t loop_step = std::stoll(makeValueToString(base_memory.m_getVariable(step_name)));

				if (loop_step == 0)
					throw std::runtime_error("Loop step cannot be zero");

				for (int64_t iterator = loop_start; iterator <= loop_stop; iterator += loop_step) {
					base_memory.m_reInsertVariable(identifier, { iterator , ENUM_TYPE_I64 });
					execute(programName, loop_start_idx, loop_end_idx);
				}
				idx = static_cast<int64_t>(loop_end_idx + 1);
				continue;
			}

			else if (currentIns.___TODO___ == "import")
			{
				auto& parameters = currentIns.___PARAMETERS___;

				if (parameters.size() < 2)
					throw std::runtime_error(
						"@import requires at least 2 parameters.\n"
						"Syntax: <state> @import : (source_section, var1, var2, ...)\n"
						"Example:        @import : (#main, x) ~ nullptr;"
					);

				str& from_address = parameters[0];

				if (!memoryUnion.contains(from_address))
				{
					std::cerr << "\n[ERROR_ENGINE] The from_address that is causing exception is: ("
						<< from_address << ")\n";
					throw std::runtime_error
					("Can't find the memory address in the memory union! buildMemory() before execute() might fix this issue");
				}

				SectionMemory& fromMemory = memoryUnion[from_address];

				for (size_t index = 1; index < parameters.size(); index++) {

					str& currentVar_name = parameters[index];

					if (!fromMemory.m_containsVariable(currentVar_name))
					{
						std::cerr << "\n[ERROR_ENGINE] The currentVar_name that is causing exception is: ("
							<< currentVar_name << ")\n";
						throw std::runtime_error
						("Can't find the variable!");
					}
					if (base_memory.m_containsVariable(currentVar_name))
					{
						std::cerr << "\n[ERROR_ENGINE] The currentVar_name that is causing exception is: ("
							<< currentVar_name << ")\n";
						throw std::runtime_error
						("Variable already exists!");
					}
					fromMemory.m_passVariabe(currentVar_name, base_memory);
					continue;
				}

			}
			else if (currentIns.___TODO___ == "export")
			{
				auto& parameters = currentIns.___PARAMETERS___;

				if (parameters.size() < 2)
					throw std::runtime_error(
						"@export requires at least 2 parameters.\n"
						"Syntax: <state> @export : (destination_section, var1, var2, ...)\n"
						"Example:          @export : (#main, x) ~ nullptr;"
					);

				str& to_address = parameters[0];
				if (!memoryUnion.contains(to_address))
				{
					std::cerr << "\n[ERROR_ENGINE] The to_address that is causing exception is: ("
						<< to_address << ")\n";
					throw std::runtime_error(
						"Can't find the memory address in the memory union! "
						"buildMemory() before execute() might fix this issue"
					);
				}

				SectionMemory& toMemory = memoryUnion[to_address];
				for (size_t index = 1; index < parameters.size(); index++)
				{
					str& currentVar_name = parameters[index];
					if (!base_memory.m_containsVariable(currentVar_name))
					{
						std::cerr << "\n[ERROR_ENGINE] The currentVar_name that is causing exception is: ("
							<< currentVar_name << ")\n";
						throw std::runtime_error("Can't find the variable!");
					}
					if (toMemory.m_containsVariable(currentVar_name))
					{
						std::cerr << "\n[ERROR_ENGINE] The currentVar_name that is causing exception is: ("
							<< currentVar_name << ")\n";
						throw std::runtime_error("Variable already exists!");
					}
					base_memory.m_passVariabe(currentVar_name, toMemory);
				}
			}

			else if (currentIns.___TODO___ == "execute") {

				std::vector<str>& parameters = currentIns.___PARAMETERS___;
				for (auto sectionName : parameters) {

					if (!compiledCode.contains(sectionName))
					{
						std::cerr << "\n[ERROR_ENGINE] The sectionName that is causing exception is: ("
							<< sectionName << ")\n";
						throw std::runtime_error("Can't find the section!");
					}

					if (!memoryUnion.contains(sectionName))
					{
						std::cerr << "\n[ERROR_ENGINE] The sectionName that is causing exception is: ("
							<< sectionName << ")\n";
						throw std::runtime_error("Can't find the section in the memory! Use buildMemory() before execute()");
					}
					execute(sectionName);
					continue;
				}
			}
			else if (currentIns.___TODO___ == "goto") {

				std::vector<str>& parameters = currentIns.___PARAMETERS___;
				if (parameters.size() != 1)
					throw std::runtime_error("@goto : () requires a single parameter (the destination name)");

				str destination_name = parameters[0];
				auto is_target = [&destination_name](const ___INSTRUCTION___T___& ins) -> bool {
					return ins.___TODO___ == "destination"
						&& !ins.___PARAMETERS___.empty()
						&& ins.___PARAMETERS___[0] == destination_name;
					};
				int64_t left = idx - 1;
				int64_t right = idx + 1;
				bool found = false;

				while (left >= 0 || right < size) {
					if (left >= 0) {
						const auto& ins = base_instructions[left];
						if (is_target(ins)) {
							idx = left;
							found = true;
							break;
						} left--;
					}
					if (right < size) {
						const auto& ins = base_instructions[right];
						if (is_target(ins)) {
							idx = right;
							found = true;
							break;
						}   right++;
					}
				}

				if (!found)
					throw std::runtime_error("Couldn't find the goto destination!");
				continue;
			}
			else if (currentIns.___TODO___ == "destination")
				continue;
			else {

				if (!all_utilities.contains(currentIns.___TODO___)) {
					std::cerr << "\n[ERROR_ENGINE] The ___TODO___ that is causing exception is: ("
						<< currentIns.___TODO___ << ")\n";
					throw std::runtime_error("The function is not provided by the utility!");
				}
				all_utilities[currentIns.___TODO___]
				(currentIns.___PARAMETERS___, currentIns.___RETURN___ADDRESS___, base_memory);
			}
		}
	}
};

#endif
