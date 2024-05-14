#include<algorithm>
#include<string>
#include<iostream>
using String = std::string;
static int indexOf(const String& str,const String sub){
    auto itr = std::search(str.begin(),str.end(),sub.begin(),sub.end());
    if(itr==str.end())return -1;
    return itr-str.begin();
}

int main(int argc, char const *argv[])
{   
    String str{"<hello>world</hello>"};
    // std::cout << indexOf(str,">") << std::endl;//6
    if(str[0]=='<'&&str[str.size()-1]=='>'){
        std::cout << str.substr(0,str.size()-1)<<std::endl;
    }
    return 0;
}
