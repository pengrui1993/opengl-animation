#include<cstddef>
#include<iostream>
#include<regex>
#include<string>
static const std::string fnames[] = {"foo.txt","bar.txt","baz.dat","zoidberg"};
void test0(){
	std::cout << std::boolalpha;
	for(const auto& fname:fnames){
		std::cout << fname << ",result:"
			<< std::regex_match(fname,std::regex("[a-z]+\\.txt"))
			<< std::endl;			
	}
	std::cout << std::noboolalpha;

}
void test1(){
	std::smatch base_match;
	for(const auto& fname:fnames){
		if(std::regex_match(fname,base_match,std::regex("([a-z]+)\\.txt"))){
			if(2==base_match.size()){
				std::cout << fname 
					<< " has basename:"
					<< base_match[1].str()
					<< std::endl;
			}	
		}	
	}
}
void test2(){
	std::smatch base_match;
	for(const auto& fname:fnames){
		if(std::regex_match(fname,base_match,std::regex("([a-z]+)\\.txt"))){
			for(int i=0;i<base_match.size();i++){
				std::cout << fname 
					<< " part " << i << ":"
					<< base_match[i].str()
					;
			}	
			std::cout << std::endl;
		}	
	}
}
int main(){
	test2();
	return 0;
}
