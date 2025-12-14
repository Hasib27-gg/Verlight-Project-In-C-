#include "VerlightVM.h"

//This is just a test example . We have to include a header file "VerlightVM.h" to get everything working.
int main()
{
	/*
	    The code down is a sample code for Celcius to Ferenhite Calculation.
		Check the API Reference for syntax.
	*/
	std::string code = R"(
       #main{
             @new_str : (buff , "") ;
             @input   : ("Enter the temperature here(C): ") ~ buff;
             @new_f32 : (resBuff , $buff);
             @multiply : (1.8 , $resBuff) ~ resBuff;
             @add      : ($resBuff , 32) ~ resBuff;
             @print    : ("The temp in f is: " , $resBuff);
       }
    )";
     /*
	    First , we need to compile the code from raw string instructions to actual VM readable instruction.
	 */
	auto compiled_code = compileVerlight(code);
	//Creating the environment for the VM.
	VerlightVM wrapper(compiled_code);
	//Now , wee need to sketchout the memroy layout.
	wrapper.buildMemory();
	//Actual execution.
	wrapper.execute();
	return 0;
}

