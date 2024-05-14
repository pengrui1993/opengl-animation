#include<cstddef>
#include<iostream>
#include<regex>
#include<string>
static std::string lines[] = {
	"Roses are #ff0000"
	,"violets are #0000ff"
	,"all of my base are belong to you"
};
static std::regex color_regex("#([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})");
void test0(){
	std::regex color_regex("#([a-f0-9]{2})([a-f0-9]{2})([a-f0-9]{2})");
	for(const auto& line:lines)
		std::cout << line << ":" << std::boolalpha
			<< std::regex_search(line,color_regex)
			<< std::endl;
}
void test1(){
	std::smatch color_match;
	for(const auto& line:lines){
		if(std::regex_search(line,color_match,color_regex)){
			std::cout << "matches for:"<<line
				<<",prefix:" <<color_match.prefix()
				<<",";
			for(std::size_t i=0;i<color_match.size();++i){
				std::cout << i << ":" << color_match[i]	<<" ";
			}	
			std::cout <<",suffix:"<<color_match.suffix()
				<< std::endl;
		}
		
	}
}
void test2(){
	std::string str(R"(
		Speed:	366
		Mass:	35
		Speed:	378
		Mass:	32
		Speed:	400
		Mass:	30)");
	auto log(str);
	std::regex r(R"(Speed:\t\d*)");
	for(std::smatch sm;std::regex_search(log,sm,r);){
		std::cout << sm.str() << ",size:"<<sm.size() <<std::endl;	
		log = sm.suffix();
	}
}
void test3(){
	std::cmatch cm;
	if(std::regex_search("this is a test",cm,std::regex("test"))){
		std::cout << "Found " << cm[0]
		<<" at position "
		<<cm.prefix().length()
		<< std::endl;	
	}
			
}
int main(){
	test2();
	return 0;
}
