#include "VerlightVM.h"

//This is just a test example . We have to include a header file "VerlightVM.h" to get everything working.
int main()
{
	/*
	    The code down is a sample code for prime checking.
		Check the API Reference for syntax.
	*/
	std::string code = R"(
       #main{
          @new_i32 : (n , 7);
          @execute : (isPrime);
          @print   : ("Is N a prime number?: " , $bool);
       }
       #isPrime{
          @import : (main , n);
          @new_i32 : (count , 0);
          @new_i32 : (it , 0);
          @new_i32 : (mod_res, 0);
          @new_bool : (bool , false);
          @loop     : (1 , $n , 1) ~ it;
          @start    : (it);
              @mod  : ($n , $it) ~ mod_res;
              @isEqual : ($mod_res , 0) ~ bool;
              <$bool> @add : ($count , 1) ~ count; 
          @end      : (it);
          @isEqual : ($count , 2) ~ bool;
          @export  : (main ,bool);
          @delete_var : (count);
          @delete_var : (it);
          @delete_var : (mod_res);
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


