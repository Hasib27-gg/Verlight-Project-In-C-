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


/*
Order of member declarations and functions inside VerlightVM (for quick reference):

1) private:
   - all_utilities (map of string -> function)
   - pm_selectStartEndCode(...)  (helper to extract code block between start/end)

2) public:
   - compiledCode (external compiled program store)
   - memoryUnion (map of section name -> SectionMemory)
   - VerlightVM(constructor)
   - void buildMemory()
   - void execute(const str& programName = "main", int64_t startIdx = 0, int64_t endIdx = -1)

Rationale:
- Utilities map is private because it's an implementation detail.
- pm_selectStartEndCode is a private helper used by execute to find the matching 'end'.
- Public API consists of the compiled code and memory union, constructor, builder, and executor.
*/


class  VerlightVM {
private:
	/*
	 all_utilities:
	 Parameters:
	   - none (this is a data member).
	 Description:
	   - A lookup table (hash map) mapping textual VM operation names (like "new_i8", "print",
		 "add", etc.) to the corresponding C++ handler functions in various Engine classes.
	   - Each handler has the signature: void(std::vector<str>& params, str& returnAddress, SectionMemory& memory)
	 Visualization:
				+-----------------------+
				| "new_i8" -> function  |
				| "print" -> function   |
				| "add" -> function     |
				+-----------------------+
	 Per-line notes:
	   - The initializer-list populates the robin_hood::unordered_map; each entry is a pair
		 { "name", Class::function } where the function is compatible with std::function signature.
	   - This is used in execute() to dispatch builtin utilities by name.
	*/
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

		{"delete_i8" , DeclaratorEngine::delete_var},

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


	/*
	 pm_selectStartEndCode
	 Parameters:
	   - std::vector<___INSTRUCTION___T___> code
		 (a copy of the instruction vector for the current section)
	   - int64_t& endIdx
		 (index where the '@start' was found; this will be updated to the index of the matching @end)
	 Description:
	   - Given the compiled instruction list and the index of a 'start' instruction, this helper
		 walks forward and collects all instructions until it finds the matching 'end' with the same identifier.
	   - On success it returns the vector of instructions that form the body between start and the matching end.
	   - It updates endIdx to point to the index of the matching 'end' instruction.
	   - If no matching end is found, it throws a runtime_error.
	 Visualization:
				(start @foo)  idx = endIdx
				  |
				  v
			 [ body instruction 1 ]
			 [ body instruction 2 ]
			 [ ...               ]
			 (end @foo)  <-- endIdx updated to this index
	 Per-line notes:
	   - identifier := name after @start (used to match the @end)
	   - loop from the next instruction (endIdx + 1) to end of code
	   - if we find ins.___TODO___ == "end" and same identifier, update endIdx and return collected body
	   - otherwise push the instruction to body
	   - if loop finishes without finding matching end, throw an exception mentioning missing match
	*/
	std::vector<___INSTRUCTION___T___>
		pm_selectStartEndCode
		(std::vector<___INSTRUCTION___T___> code, int64_t& endIdx)
	{
		std::vector<___INSTRUCTION___T___> body; // collected instructions between start and end
		str identifier = code[endIdx].___PARAMETERS___[0]; // identifier specified on the @start line

		for (int64_t idx = endIdx + 1;
			idx < static_cast<int64_t>(code.size());
			++idx) {
			const auto& ins = code[idx]; // reference to the current instruction in iteration
			if (ins.___TODO___ == "end" &&
				!ins.___PARAMETERS___.empty() &&
				ins.___PARAMETERS___[0] == identifier) {
				endIdx = idx;       // update the caller's endIdx to the index of this 'end'
				return body;       // return the collected body instructions
			}
			body.push_back(ins);   // otherwise, append instruction to the body
		}

		// we've scanned to the end without finding the matching 'end' -> error
		throw std::runtime_error("missing matching 'end' for @start " + identifier);
	}

public:
	/*
	 compiledCode:
	 Parameters:
	   - none (data member).
	 Description:
	   - The compiled program: map from section name (string) to a vector of instructions.
	 Visualization:
				+-----------------------------+
				| "main" -> [ins1, ins2, ...] |
				| "utils" -> [ins1, ins2, ...]|
				+-----------------------------+
	*/
	___UMAP___STRING___VECTOR___INSTRUCTIONS___T___ compiledCode;

	/*
	 memoryUnion:
	 Parameters:
	   - none (data member).
	 Description:
	   - Map of section name -> SectionMemory that holds runtime variables for that section.
	 Visualization:
				 +----------------------+
				 | "#main" -> SectionMemory |
				 | "#lib"  -> SectionMemory |
				 +----------------------+
	*/
	robin_hood::unordered_map<str, SectionMemory> memoryUnion;

	/*
	 Constructor: VerlightVM(___UMAP___STRING___VECTOR___INSTRUCTIONS___T___& compiledCode)
	 Parameters:
	   - ___UMAP___STRING___VECTOR___INSTRUCTIONS___T___& compiledCode
		 (reference to the compiled program data structure to be executed)
	 Description:
	   - Initializes the VM instance by copying the compiledCode reference into the instance member.
	 Visualization:
				+------------------+
				| constructor copies |
				| compiled code ref  |
				+------------------+
	 Per-line notes:
	   - compiledCode(compiledCode) : member initializer copying the given reference into the member.
	*/
	VerlightVM(___UMAP___STRING___VECTOR___INSTRUCTIONS___T___& compiledCode) :
		compiledCode(compiledCode) {
	};

	/*
	 buildMemory()
	 Parameters:
	   - none
	 Description:
	   - Walks through compiledCode keys (section names) and creates an empty SectionMemory entry within memoryUnion
		 for each section name. This prepares the memoryUnion so execute() can use memory for each section.
	 Visualization:
				compiledCode keys:
				| "main" | "lib" | "util" |
						  |
						  v
				memoryUnion:
				| "main" -> SectionMemory |
				| "lib"  -> SectionMemory |
	 Per-line notes:
	   - for (auto name_instructions : compiledCode): iterate all pairs (name, instructions)
	   - SectionMemory newMemory; : create an empty SectionMemory object
	   - memoryUnion.insert({ name_instructions.first , newMemory }); : insert mapping sectionName -> newMemory
	*/
	void buildMemory() {
		for (auto name_instructions : compiledCode) {
			SectionMemory newMemory;
			memoryUnion.insert({ name_instructions.first , newMemory });
		}
	}

	/*
	 execute(...)
	 Parameters:
	   - const str& programName = "main"
		 (section name to execute; default is "main")
	   - int64_t startIdx = 0
		 (index of the first instruction to execute within the section)
	   - int64_t endIdx = -1
		 (index of the last instruction to execute; -1 means "till the end")
	 Description:
	   - The main interpreter loop. It iterates instructions in compiledCode[programName] from startIdx to endIdx.
	   - Supports:
		 * dispatching builtin utilities via all_utilities map,
		 * control flow constructs such as @start/@end (loop), @import/@export, @execute (call other section),
		 * goto/destination navigation,
		 * branching via the ___VM___WILL___EXECUTE___ expression.
	   - Uses SectionMemory for variables and return addresses.
	 Visualization (high-level):
				+-----------------------------------------------+
				| fetch base_instructions for programName       |
				| for idx in [startIdx..endIdx]:                |
				|   read instruction                            |
				|   evaluate will_this_execute (branching)      |
				|   switch on instruction type (@start, import, |
				|               export, execute, goto, util)    |
				+-----------------------------------------------+
	 Per-line & block notes: (the function is fairly long; I annotate logical blocks)
	*/
	void execute(const str& programName = "main",
		int64_t startIdx = 0,
		int64_t endIdx = -1)
	{
		// verify the program name exists in compiledCode (otherwise cannot execute)
		if (!compiledCode.contains(programName))
		{
			std::cerr << "\n[ERROR_ENGINE] The programName that is causing exception is: ("
				<< programName << ")\n";
			throw std::runtime_error("Can't find the program name!");
		}

		// verify that memoryUnion has an entry for programName (buildMemory should be run first)
		if (!memoryUnion.contains(programName))
		{
			std::cerr << "\n[ERROR_ENGINE] The programName that is causing exception is: ("
				<< programName << ")\n";
			throw std::runtime_error
			("Can't find the program name in memoryUnion. Use buildMemory() if you haven't drawn the blueprint");
		}

		// fetch references to program instructions and memory
		auto& base_instructions = compiledCode[programName];
		SectionMemory& base_memory = memoryUnion[programName];
		int64_t size = static_cast<int64_t>(base_instructions.size());

		// if endIdx is -1, set it to last instruction index
		if (endIdx == -1) endIdx = size - 1;

		// main interpreter loop: iterate instructions within [startIdx..endIdx]
		for (int64_t idx = startIdx; idx <= endIdx; ++idx) {

			auto& currentIns = base_instructions[idx]; // current instruction reference
			str will_this_execute = currentIns.___VM___WILL___EXECUTE___; // branching expression (string)
			bool target = true; // default target flag for inversion logic

			// if will_this_execute begins with '!' then invert the target logic
			if (!will_this_execute.empty() && will_this_execute.front() == '!') {
				target = false;
				will_this_execute = will_this_execute.substr(1); // remove '!'
			}

			// if will_this_execute begins with '$' then fetch the actual boolean from memory (stringified)
			if (!will_this_execute.empty() && will_this_execute.front() == '$')
				will_this_execute = base_memory.m_fetchStringalizedValue(will_this_execute);

			// now will_this_execute must be "true" or "false"; otherwise it's an unknown boolean -> error
			if (will_this_execute != "true" && will_this_execute != "false")
				throw std::runtime_error("Unknown boolean type!");

			// compute raw boolean result (true/false) and then combine with target (considering inversion)
			bool raw_result = (will_this_execute == "true");
			bool actual_result = !(raw_result ^ target); // XOR then NOT gives equality check with target
			if (!actual_result) continue; // skip this instruction if branching says not to execute

			auto& return_address = currentIns.___RETURN___ADDRESS___; // where results will be stored

			// ensure the return address exists in memory unless it's "nullptr"
			if (!base_memory.m_containsVariable(return_address) && return_address != "nullptr")
			{
				std::cerr << "\n[ERROR_ENGINE] The return_address that is causing exception is: ("
					<< return_address << ")\n";
				throw std::runtime_error
				("Can't find the return address!");
			}

			// dispatch based on the instruction verb stored in ___TODO___
			if (currentIns.___TODO___ == "start") {
				// @start handling - loop construct

				// require exactly 1 parameter (iterator name)
				if (currentIns.___PARAMETERS___.size() != 1)
					throw std::runtime_error("Parameters for @start : () requires a single parameter (iterator_name)!");

				str identifier = currentIns.___PARAMETERS___[0]; // loop iterator name

				uint64_t loop_start_idx = idx + 1; // first instruction inside the loop body
				std::vector<___INSTRUCTION___T___> temp = pm_selectStartEndCode(base_instructions, idx);
				// pm_selectStartEndCode will update idx to the index of the matching 'end'
				uint64_t loop_end_idx = idx - 1; // after pm_selectStartEndCode, idx points at 'end'; body ends at idx-1

				// build names of helper variables used by LoopEngine to store start/stop/step
				str start_name = str("___LOOP___ENGINE___") + identifier + str("___start___");
				str stop_name = str("___LOOP___ENGINE___") + identifier + str("___stop___");
				str step_name = str("___LOOP___ENGINE___") + identifier + str("___step___");

				// ensure those loop variables exist in memory (they should have been created earlier)
				if (!base_memory.m_containsVariable(start_name) ||
					!base_memory.m_containsVariable(stop_name) ||
					!base_memory.m_containsVariable(step_name))
					throw std::runtime_error("Can't find the value of start or stop or step in the section buffer!");

				// read the start/stop/step values from memory and convert them to int64_t via makeValueToString
				int64_t loop_start = std::stoll(makeValueToString(base_memory.m_getVariable(start_name)));
				int64_t loop_stop = std::stoll(makeValueToString(base_memory.m_getVariable(stop_name)));
				int64_t loop_step = std::stoll(makeValueToString(base_memory.m_getVariable(step_name)));

				// step cannot be zero (would cause infinite loop)
				if (loop_step == 0)
					throw std::runtime_error("Loop step cannot be zero");

				// iterate using the loop_start/loop_stop/loop_step semantics
				for (int64_t iterator = loop_start; iterator <= loop_stop; iterator += loop_step) {
					// re-insert or update the iterator variable in the section memory
					base_memory.m_reInsertVariable(identifier, { iterator , ENUM_TYPE_I64 });
					// recursively execute the loop body (loop_start_idx .. loop_end_idx)
					execute(programName, loop_start_idx, loop_end_idx);
				}
				// after finishing the loop, set idx to the instruction after the end of the loop block
				idx = static_cast<int64_t>(loop_end_idx + 1);
				continue; // move to next top-level instruction
			}

			else if (currentIns.___TODO___ == "import")
			{
				// @import handling - copy variables from another section's memory into current section

				auto& parameters = currentIns.___PARAMETERS___;

				// require at least 2 parameters: source section name and at least one variable name
				if (parameters.size() < 2)
					throw std::runtime_error(
						"@import requires at least 2 parameters.\n"
						"Syntax: <state> @import : (source_section, var1, var2, ...)\n"
						"Example:        @import : (#main, x) ~ nullptr;"
					);

				str& from_address = parameters[0]; // source section identifier

				// ensure the source section exists inside memoryUnion
				if (!memoryUnion.contains(from_address))
				{
					std::cerr << "\n[ERROR_ENGINE] The from_address that is causing exception is: ("
						<< from_address << ")\n";
					throw std::runtime_error
					("Can't find the memory address in the memory union! buildMemory() before execute() might fix this issue");
				}

				SectionMemory& fromMemory = memoryUnion[from_address]; // ref to source memory

				// iterate parameters from 1..N and move each variable from source into base_memory
				for (size_t index = 1; index < parameters.size(); index++) {

					str& currentVar_name = parameters[index];

					// ensure the variable exists in source memory
					if (!fromMemory.m_containsVariable(currentVar_name))
					{
						std::cerr << "\n[ERROR_ENGINE] The currentVar_name that is causing exception is: ("
							<< currentVar_name << ")\n";
						throw std::runtime_error
						("Can't find the variable!");
					}
					// ensure the destination (base_memory) does not already contain the variable
					if (base_memory.m_containsVariable(currentVar_name))
					{
						std::cerr << "\n[ERROR_ENGINE] The currentVar_name that is causing exception is: ("
							<< currentVar_name << ")\n";
						throw std::runtime_error
						("Variable already exists!");
					}
					// perform the actual pass/copy operation of variable data
					fromMemory.m_passVariabe(currentVar_name, base_memory);
					continue;
				}

			}
			else if (currentIns.___TODO___ == "export")
			{
				// @export handling - copy variables from current section into another section's memory

				auto& parameters = currentIns.___PARAMETERS___;

				// require at least 2 parameters: target section name and at least one variable name
				if (parameters.size() < 2)
					throw std::runtime_error(
						"@export requires at least 2 parameters.\n"
						"Syntax: <state> @export : (destination_section, var1, var2, ...)\n"
						"Example:          @export : (#main, x) ~ nullptr;"
					);

				str& to_address = parameters[0]; // target section identifier
				// ensure the target section exists in memoryUnion
				if (!memoryUnion.contains(to_address))
				{
					std::cerr << "\n[ERROR_ENGINE] The to_address that is causing exception is: ("
						<< to_address << ")\n";
					throw std::runtime_error(
						"Can't find the memory address in the memory union! "
						"buildMemory() before execute() might fix this issue"
					);
				}

				SectionMemory& toMemory = memoryUnion[to_address]; // ref to target memory
				for (size_t index = 1; index < parameters.size(); index++)
				{
					str& currentVar_name = parameters[index];
					// ensure base_memory has the variable to be exported
					if (!base_memory.m_containsVariable(currentVar_name))
					{
						std::cerr << "\n[ERROR_ENGINE] The currentVar_name that is causing exception is: ("
							<< currentVar_name << ")\n";
						throw std::runtime_error("Can't find the variable!");
					}
					// ensure target memory doesn't already have the variable
					if (toMemory.m_containsVariable(currentVar_name))
					{
						std::cerr << "\n[ERROR_ENGINE] The currentVar_name that is causing exception is: ("
							<< currentVar_name << ")\n";
						throw std::runtime_error("Variable already exists!");
					}
					// perform the variable transfer from base_memory to toMemory
					base_memory.m_passVariabe(currentVar_name, toMemory);
				}
			}

			else if (currentIns.___TODO___ == "execute") {

				// @execute handling - invoke other sections by name
				std::vector<str>& parameters = currentIns.___PARAMETERS___;
				for (auto sectionName : parameters) {

					// ensure the section exists in compiledCode
					if (!compiledCode.contains(sectionName))
					{
						std::cerr << "\n[ERROR_ENGINE] The sectionName that is causing exception is: ("
							<< sectionName << ")\n";
						throw std::runtime_error("Can't find the section!");
					}

					// ensure memoryUnion contains a memory for that section (buildMemory required)
					if (!memoryUnion.contains(sectionName))
					{
						std::cerr << "\n[ERROR_ENGINE] The sectionName that is causing exception is: ("
							<< sectionName << ")\n";
						throw std::runtime_error("Can't find the section in the memory! Use buildMemory() before execute()");
					}
					// recursively execute other section
					execute(sectionName);
				}
			}
			else if (currentIns.___TODO___ == "goto") {
				// @goto handling - find a matching 'destination' instruction and jump to it

				std::vector<str>& parameters = currentIns.___PARAMETERS___;
				if (parameters.size() != 1)
					throw std::runtime_error("@goto : () requires a single parameter (the destination name)");

				str destination_name = parameters[0];
				// lambda to test whether a given instruction is the destination we want
				auto is_target = [&destination_name](const ___INSTRUCTION___T___& ins) -> bool {
					return ins.___TODO___ == "destination"
						&& !ins.___PARAMETERS___.empty()
						&& ins.___PARAMETERS___[0] == destination_name;
					};
				int64_t left = idx - 1; // search backward
				int64_t right = idx + 1; // search forward
				bool found = false;

				// bidirectional search for the destination instruction
				while (left >= 0 || right < size) {
					if (left >= 0) {
						const auto& ins = base_instructions[left];
						if (is_target(ins)) {
							idx = left; // jump to left destination
							found = true;
							break;
						} left--;
					}
					if (right < size) {
						const auto& ins = base_instructions[right];
						if (is_target(ins)) {
							idx = right; // jump to right destination
							found = true;
							break;
						}   right++;
					}
				}

				// if destination not found, it's an error
				if (!found)
					throw std::runtime_error("Couldn't find the goto destination!");
				continue; // after adjusting idx, continue the for-loop (will increment idx at top)
			}
			else if (currentIns.___TODO___ == "destination")
				// 'destination' is a label target and has no runtime behaviour by itself
				continue;
			else {
				// default: treat instruction as utility function name and dispatch it

				// ensure the utility exists in the map; otherwise throw
				if (!all_utilities.contains(currentIns.___TODO___)) {
					std::cerr << "\n[ERROR_ENGINE] The ___TODO___ that is causing exception is: ("
						<< currentIns.___TODO___ << ")\n";
					throw std::runtime_error("The function is not provided by the utility!");
				}
				// call the utility handler with parameters, return address and the section memory
				all_utilities[currentIns.___TODO___]
				(currentIns.___PARAMETERS___, currentIns.___RETURN___ADDRESS___, base_memory);
			}
		}
	}
};

#endif
