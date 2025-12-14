#include "VerlightVM.h"


int main() {
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

	auto compiled_code = compileVerlight(code);
	VerlightVM wrapper(compiled_code);
	wrapper.buildMemory();
	wrapper.execute();
	return 0;
}
