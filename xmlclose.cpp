#include<regex>
#include<string>
#include<iostream>

using Regex = std::regex;
int main(){
	Regex close("(</|/>)");
	std::string msg = 
		R"(
		<hello> that </hello>
		<world key="yes" />
		)";
	std::cout << msg << std::endl;	
	return 0;
}
