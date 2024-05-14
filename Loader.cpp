#include"Loader.h"
#include<fstream>
#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<regex>
#include<filesystem>
#include<algorithm>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<memory>
#include<sstream>
#include<stdexcept>
#define PRINT(line) std::cout << (line) << std::endl
namespace{
    using String = std::string;
    using Strings = std::vector<String>;
    using mat4 = glm::mat4x4;
    template<typename T>
    static int indexOf(const std::vector<T>& v,const T& t){
        auto ret = std::find(v.begin(),v.end(),t);
        return v.end()==ret?-1:ret-v.begin();
    }
    static int lastIndexOf(const String& s,char c){
        for(int i=s.size()-1;i>=0;i--)
            if(c==s[i])return i;
        return -1;
    }

    static int firstIndexOf(const String& s,char c){
        for(int i=0;i<s.size();i++)
            if(c==s[i])return i;
        return -1;
    }
    static int countChar(const String& s,char c){
        int res = 0;
        for(auto ch:s){
            if(ch==c)res++;
        }
        return res;
    }
    static Strings split(const String& s, char c) {
            Strings result;
            size_t begin = 0;
            while (true) {
                size_t end = s.find_first_of(c, begin);
                result.push_back(s.substr(begin, end - begin));
                if (end == String::npos)break;
                begin = end + 1;
            }
            return result;
    }
    class XmlNode{
        using AttributeMap = std::unordered_map<String,String>;
        using Nodes = std::vector<XmlNode*>;
        using Children = std::unordered_map<String,Nodes>;
    private:
        String name;
        AttributeMap attributes;
        String data;
        Children children;
    private:
        void recursiveDelete(XmlNode* node){
            for(auto& child:node->children){
                for(auto node:child.second){
                    if(node)
                        recursiveDelete(node);
                    if(node)
                        delete node;
                }
            }
        }
    public:
        XmlNode(String n):name(n){}
        ~XmlNode(){
            for(auto& child:children){
                for(auto node:child.second){
                    recursiveDelete(node);
                }
            }
        }
        String getName(){
            return String(name);
        }
        String getData(){
            return String(data);
        }
        void setData(String d){
            data = d;
        }
        AttributeMap& getAttributes(){
            return attributes;
        }
        void addAttribute(String att,String value){
            attributes[att] = value;
        }
        String getAttribute(String att){
            return attributes[att];
        }
        void addChild(XmlNode* child){
            children[child->name].push_back(child);
        }
        Children& allChildren(){
            return children;
        }
        Nodes& getChildren(String name){
            return children[name];
        }
        XmlNode* getChild(String name){
            Nodes& ns = getChildren(name);
            if(ns.empty())
                return nullptr;
            return ns[0];
        }
        void attributeDebug(String& childName,String& attr,String& value){
            if(String("input")==childName&&String("semantic")==attr&&String("JOINT")==value){
                Nodes& ns = getChildren(name);
                PRINT("*******");
                PRINT(name+",size of input:"+(std::stringstream()<<ns.size()).str());
                for(auto& p :this->allChildren()){
                    PRINT("name:"+p.first);
                    for(auto& node:p.second){
                        auto& map = node->getAttributes();
                        PRINT("+++++");
                        for(auto& kv:map){
                            std::stringstream ss;
                            ss<<kv.second.size();
                            PRINT(kv.first+":"+kv.second+",size:"+ss.str());
                        }
                        PRINT("+++++");
                    }
                    PRINT("---------");
                }
            }
        }
        XmlNode* getChildWithAttribute(String childName,String attr,String value){
            Nodes& ns = getChildren(childName);
            // attributeDebug(childName,attr,value);
            if(ns.empty())
                return nullptr;
            for(auto child:ns){
                if(value==child->getAttribute(attr))
                    return child;
            }
            return nullptr;
        }
    };

    using Regex = std::regex;
    class XmlParser{
        static Regex DATA;
        static Regex START_TAG;
        static Regex ATTR_NAME;
        static Regex ATTR_VAL;
        static Regex CLOSE;
        using FilePath = String;
   
        using FileInputStream = std::ifstream;
        using Matcher = std::smatch ;
       
    private:
        XmlNode* loadNode(FileInputStream& is){
            String line;
            if(!std::getline(is,line))
                return nullptr;
            if(0==trim(line).rfind("</",0))//start with
                return nullptr;
            String sentence(line);
            Matcher match;
            std::regex_search(sentence,match,START_TAG);
            Strings startTagParts = split(match[1].str(),' ');
            XmlNode* node = new XmlNode(startTagParts[0]);
            for(int i=1;i<startTagParts.size();i++){
                String currentLine = startTagParts[i];
                if(contains(currentLine,"=")){
                    Matcher name,val;
                    std::regex_search(currentLine,name,ATTR_NAME);
                    std::regex_search(currentLine,val,ATTR_VAL);
                    node->addAttribute(name[1].str(),val[1].str());
                }
            }
            Matcher data;
            if(line.size()>1000){//bug on gcc 11
                trim(line);
                if('<'==line[0]
                    &&'>'==line[line.size()-1]
                    && 2==countChar(line,'<')
                    && 2==countChar(line,'>')
                    ){
                    const int start = firstIndexOf(line,'>')+1;
                    const int len = lastIndexOf(line,'<')-start;
                    auto data = line.substr(start,len);
                    node->setData(trim(data));
                    // PRINT(node->getData());
                }
            }else{
                std::regex_search(line,data,DATA);
                if(data.size()){
                    node->setData(data[1].str());
                }
            }            
            Matcher close;
            std::regex_search(line,close,CLOSE);
            if(close.size()){;
                return node;
            }
            XmlNode* child = nullptr;
            while(child=loadNode(is))
                node->addChild(child);
            return node;
        }
        static bool contains(const String&str,const String& sub){
            return str.find(sub)!= String::npos;
        }   
     
        static String& trim(String& s){
            if(s.empty())return s;
            s.erase(0,s.find_first_not_of(" "));
            s.erase(s.find_last_not_of(" ")+1);
            s.erase(s.find_last_not_of("\n")+1);
            s.erase(s.find_last_not_of("\r")+1);
            return s;
        }
    public:
        XmlNode* loadXmlFile(FilePath path){
            namespace fs = std::filesystem;
            if(!fs::is_regular_file(path)){
                std::cerr<< path<< " is not a regular file" << std::endl;
                return nullptr;
            }
            FileInputStream is;
            is.open(path,std::ios::in);
            if(!is){
                std::cerr << "open file error,path:"<< path << std::endl;
                std::exit(-1);
            }
            String line;
            std::getline(is,line);
            return loadNode(is);
        }
    };
    Regex XmlParser::DATA(">(.+?)<");
    Regex XmlParser::START_TAG("<(.+?)>");
    Regex XmlParser::ATTR_NAME("(.+?)=");
    Regex XmlParser::ATTR_VAL("\"(.+?)\"");
    Regex XmlParser::CLOSE("(</|/>)");
    void test(std::istream& is){
        String line;
        std::getline(is,line);
    }
    using ID = int;
    class ColladaLoader;
    using Set = std::unordered_set<ID>;
    using Map = std::unordered_map<ID,ColladaLoader*>;
    Set ids;
    Map loaders;

    struct VertexSkinData{
        using JointIds = std::vector<int>;
        using Weights = std::vector<float>;
        JointIds jointIds;
        Weights weights;
        void addJointEffect(int jointId,float weight){
            for(int i=0;i<weights.size();i++) {
				if(weight>weights[i]) {
					jointIds[i] = jointId;//insert
					weights[i]=weight;//insert
					return;
				}
			}
			jointIds.push_back(jointId);//insert
			weights.push_back(weight);//insert
        }
        void limitJointNumber(int max){
            if(jointIds.size()>max){
                float topMaxTotal = 0;
                for(int i=0;i<jointIds.size();i++){
                    if(i<max){
                        topMaxTotal += weights[i];
                    }else{
                        auto start = jointIds.begin()+i;
                        jointIds.erase(start,start+1);
                        auto wstart = weights.begin()+i;
                        weights.erase(wstart,wstart+1);
                    }
                }
                for(int i=0;i<max;i++){
                    weights[i] = std::min(weights[i]/topMaxTotal,1.f);
                }
            }else if(jointIds.size()<max){
                do{
                    jointIds.push_back(0);
                    weights.push_back(0.f);
                }while(jointIds.size()<max);
            }
        }
    };
    struct VertexData{
        using Vector3f = glm::vec3;
        using Next = std::shared_ptr<VertexData>;
        static const int NO_INDEX = -1;
        int index;
        Vector3f position;
        VertexSkinData weightsData;
        int textureIndex;
        int normalIndex;
        Next next;
        VertexData(int idx,const Vector3f& pos
            ,const VertexSkinData&weights
        ):index(idx),position(pos),weightsData(weights)
            ,textureIndex(NO_INDEX),normalIndex(NO_INDEX)
            ,next(){}
        VertexData& setTextureIndex(int index){
            textureIndex = index;
            return *this;
        }
        VertexData& setNormalIndex(int index){
            normalIndex = index;
            return *this;
        }

    };
    class JointListData{
        using List=Strings;
    public:
        List list;
    public:
        JointListData():list(){}
        void swap(List&& l){
            List temp(list);
            list = List(l);
            l = List(temp);
        }
    };

    class ColladaLoader:public Loader{
        using FilePath = String;
    public:
        ColladaLoader(FilePath p,int mw):Loader()
        ,id(),maxWights(mw)
        ,done{false},path(p)
        ,rootJointData(),mesh(),keyFrameDatas()
        ,jointCount(0),jointOrder()
        ,furthestPoint(0){
            if(done)return;
            while(ids.count(id = (++nextId)));
            ids.emplace(id);
            loaders[id] = this;
            init();
            done = true;
        }
        ~ColladaLoader(){
            loaders[id] = nullptr;
            ids.erase(id);
            PRINT("destructor ColladaLoader");
            delete xmlRoot;
        }
        virtual int getJointCount() override{
            return jointCount;
        }
        virtual float getAnimationTime() override{
            return animationTimeDuration;
        }
        virtual bool getRootJointData(JointData& data)override {
            data = rootJointData;
            return true;
        }
        virtual bool getMeshData(MeshData&data)override{
            data = mesh;
            return true;
        }
        virtual bool getKeyFramesData(KeyFramesData&data)override{
            data = keyFrameDatas;
            return true;
        }
    private:
        using VertexSkinDataList = std::vector<VertexSkinData>;
        using Integers = std::vector<int>;
        using Floats = std::vector<float>;
        using VertexDataList = std::vector<std::shared_ptr<VertexData>>;
        using Vector3fList = std::vector<glm::vec3>;
        using Vector2fList = std::vector<glm::vec2>;
        VertexSkinDataList libraryControllers(){
            XmlNode* skinNode = xmlRoot->getChild("library_controllers")
                                        ->getChild("controller")
                                        ->getChild("skin")
            ;
            XmlNode* vertexWeightsNode = skinNode->getChild("vertex_weights");
            String joinDataId = vertexWeightsNode
                ->getChildWithAttribute("input", "semantic", "JOINT")
                ->getAttribute("source")
                .substr(1);
            String data = skinNode
                ->getChildWithAttribute("source", "id", joinDataId)
                ->getChild("Name_array")
                ->getData();
            jointOrder.swap(split(data,' '));
            String weightDataId = vertexWeightsNode
                ->getChildWithAttribute("input", "semantic", "WEIGHT")
                ->getAttribute("source")
                .substr(1);
            data = skinNode
                ->getChildWithAttribute("source", "id", weightDataId)
                ->getChild("float_array")
                ->getData();
            Strings weightRawData = split(data,' ');
            Floats weights(weightRawData.size());
            for(int i=0;i<weights.size();i++){
                weights[i] = std::stof(weightRawData[i]);
            }
            data = vertexWeightsNode->getChild("vcount")->getData();
            Strings vcountRawData = split(data,' ');
            Integers effectorJointCounts(vcountRawData.size());
            for(int i=0;i<effectorJointCounts.size();i++){
                effectorJointCounts[i] = std::stoi(vcountRawData[i]);
            }
            VertexSkinDataList vertexSkinDataList;
            int pointer = 0;
            data =  vertexWeightsNode->getChild("v")->getData();
            Strings vRawData = split(data,' ');
            for(int count:effectorJointCounts){
                vertexSkinDataList.push_back(VertexSkinData());
                VertexSkinData& skinData = *(--vertexSkinDataList.end());
                for(int i=0;i<count;i++){
                    const auto jointId = std::stoi(vRawData[pointer++]);
                    const auto weightId = std::stoi(vRawData[pointer++]);
                    skinData.addJointEffect(jointId,weights[weightId]);
                }
                skinData.limitJointNumber(maxWights);
            }

            return vertexSkinDataList;
        }
        JointData loadJointData(XmlNode*node,bool isRoot){
            String nameId = node->getAttribute("id");
            auto& list = jointOrder.list;
            int index = indexOf(list,nameId);
            String data;
            data = node->getChild("matrix")->getData();
            Strings matrixRawData = split(data,' ');
            mat4 localMatrix;
            //The spec for the <matrix> element states that COLLADA matrices are column-major ordered.
            for(int i=0;i<16;i++){
                int col = i/4;
                int row = i%4;
                localMatrix[row][col] = std::stof(matrixRawData[i]);
            }
            // glm::transpose(loadMatrix);
            if(isRoot){
                // glm::transpose(root);
                localMatrix  = CORRECTION*localMatrix;
            }
            JointData joint(index,nameId,localMatrix);
            jointCount++;
            for(auto childNode:node->getChildren("node")){
                joint.addChild(loadJointData(childNode,false));
            }
            return joint;
        }
        void libraryVisualScenes(){
            XmlNode* armatureNode = xmlRoot
                ->getChild("library_visual_scenes")
				->getChild("visual_scene")
				->getChildWithAttribute("node","id","Armature");
            XmlNode* joinRootNode = armatureNode->getChild("node");
            rootJointData = loadJointData(joinRootNode,true);
        }
        void dealWithAlreadyProcessedVertex(VertexData& previousVertex
            , int newTextureIndex, int newNormalIndex
            ,VertexDataList& vertices,Integers& indices
        ){
            	//递归返回条件。如果递归查找  链表（VertexData.next） 中存在该纹理 则不创建新的纹理添加到链表末尾 直接添加索引
            if(newTextureIndex == previousVertex.textureIndex && previousVertex.normalIndex == newNormalIndex) {
                indices.push_back(previousVertex.index);
            }else {//递归体
                //查找该节点是否存在下一个节点 如果存在 则递归处理
                auto another = previousVertex.next;
                if(another) {
                    dealWithAlreadyProcessedVertex(*another,newTextureIndex,newNormalIndex,vertices,indices);
                }else {//如果不存在 也就是递归查找整个 共同纹理索引和法向量索引的 VertexData 链表中不存在 需要新创建并添加到 vertices 并添加到链表末尾
                    auto last = std::make_shared<VertexData>(vertices.size(),previousVertex.position,previousVertex.weightsData);
                    last->textureIndex = newTextureIndex;
                    last->normalIndex = newNormalIndex;
                    previousVertex.next = last; //添加到链表末尾
                    vertices.push_back(last);
                    indices.push_back(last->index);
                }
            }
        }
        void libraryGeometries(const VertexSkinDataList& vertexSkinDataList){
            VertexDataList vertices;
            Vector3fList normals;
            Vector2fList textures;
            Integers indices;
            auto meshNode = xmlRoot->getChild("library_geometries")
                                    ->getChild("geometry")
                                    ->getChild("mesh");
            //position
            String positionsId = meshNode->getChild("vertices")
                                        ->getChild("input")
                                        ->getAttribute("source")
                                        .substr(1);
            auto positionNode = meshNode->getChildWithAttribute("source","id",positionsId)
                                        ->getChild("float_array");
            int count = std::stoi(positionNode->getAttribute("count"));
            String data;
            data = positionNode->getData();
            Strings positionRawData = split(data,' ');

            for(int i=0;i<count/3;i++){
                float x = std::stof(positionRawData[i*3+0]);
                float y = std::stof(positionRawData[i*3+1]);
                float z = std::stof(positionRawData[i*3+2]);
                glm::vec4 tmp(x,y,z,1);
                tmp = CORRECTION*tmp;
                int index = vertices.size();
                vertices.push_back(std::make_shared<VertexData>(index,glm::vec3(tmp.x,tmp.y,tmp.z),vertexSkinDataList[i]));
                //error 
                //vertices.emplace_back(index,glm::vec3(tmp.x,tmp.y,tmp.z),vertexSkinDataList[i]);
            }
            //normal
            auto polyListNode = meshNode->getChild("polylist");
            String normalsId = polyListNode->getChildWithAttribute("input", "semantic", "NORMAL")
                                            ->getAttribute("source")
                                            .substr(1);
            auto normalNode = meshNode->getChildWithAttribute("source", "id",normalsId)
                                        ->getChild("float_array");
            count = std::stoi(normalNode->getAttribute("count"));
            data = normalNode->getData();
            Strings normalRawData = split(data,' ');
            for(int i=0;i<count/3;i++){
                float x = std::stof(normalRawData[i*3+0]);
                float y = std::stof(normalRawData[i*3+1]);
                float z = std::stof(normalRawData[i*3+2]);
                glm::vec4 tmp(x,y,z,1);
                tmp = CORRECTION*tmp;
                normals.emplace_back(tmp.x,tmp.y,tmp.z);
            }
            //texture coordination
            String texCoordsId = 
                polyListNode->getChildWithAttribute("input", "semantic", "TEXCOORD")
                            ->getAttribute("source")
                            .substr(1);
            //PRINT(texCoordsId);//#Cube-mesh-map-0
            auto textureNode = meshNode->getChildWithAttribute("source", "id", texCoordsId)
                            ->getChild("float_array");
            count = std::stoi(textureNode->getAttribute("count"));
            data = textureNode->getData();
            auto texRawData = split(data,' ');
            for(int i=0;i<count/2;i++){
                float s = std::stof(texRawData[i*2 + 0]);
                float t = std::stof(texRawData[i*2 + 1]);
                textures.push_back(glm::vec2(s,t));
            }
            // assemble vertices
            count = polyListNode->getChildren("input").size();
            data = polyListNode->getChild("p")->getData();
            Strings indexRawData = split(data,' ');
            for(int i=0;i<indexRawData.size()/count;i++){
                int positionIndex = std::stoi(indexRawData[i*count +0]);
                int normalIndex = std::stoi(indexRawData[i*count +1]);
                int texCoordIndex = std::stoi(indexRawData[i*count +2]);
                auto currentVertex = vertices[positionIndex];
                if(!(VertexData::NO_INDEX!=currentVertex->textureIndex
                    &&VertexData::NO_INDEX!=currentVertex->normalIndex
                )){
                    currentVertex->setTextureIndex(texCoordIndex)
                                .setNormalIndex(normalIndex);
                    indices.push_back(positionIndex);
                }else{
                    dealWithAlreadyProcessedVertex(*currentVertex,texCoordIndex,normalIndex,vertices,indices);
                }
            }
            // set unused vertices texture index and normal index to zero
            for(auto vertex:vertices){
                // vertex.averageTangents;// 学习的源码中有这行代码， 但是没看到动画这个教程中用到 所以注释掉
			    if(!(VertexData::NO_INDEX!=vertex->normalIndex 
                    && VertexData::NO_INDEX!=vertex->textureIndex))//如果索引没有被设置过 则设置为0
				vertex->setNormalIndex(0).setTextureIndex(0);
            }
            Floats verticesArray(vertices.size()*3);
            Floats texturesArray(vertices.size()*2);
            Floats normalsArray(vertices.size()*3);
            Integers jointIdsArray(vertices.size()*3);
            Floats weightsArray(vertices.size()*3);
            float length;
            for(int i=0;i<vertices.size();i++){
                auto& cur = vertices[i];
                if((length=glm::length(cur->position))>furthestPoint){
                    furthestPoint = length;
                }
                auto& position = cur->position;
                auto& texCoord = textures[cur->textureIndex];
                auto& normal = normals[cur->normalIndex];
                auto& weights = cur->weightsData;
                verticesArray[i * 3] = position.x;
                verticesArray[i * 3 + 1] = position.y;
                verticesArray[i * 3 + 2] = position.z;
                texturesArray[i * 2] = texCoord.x;
                texturesArray[i * 2 + 1] = 1 - texCoord.y; // 做了坐标变换  因为纹理坐标是左手系
                normalsArray[i * 3] = normal.x;
                normalsArray[i * 3 + 1] = normal.y;
                normalsArray[i * 3 + 2] = normal.z;
                jointIdsArray[i * 3] = weights.jointIds[0];
                jointIdsArray[i * 3 + 1] = weights.jointIds[1];
                jointIdsArray[i * 3 + 2] = weights.jointIds[2];
                weightsArray[i * 3] = weights.weights[0];
                weightsArray[i * 3 + 1] = weights.weights[1];
                weightsArray[i * 3 + 2] = weights.weights[2];
            }
            Integers indicesArray(indices.size());
            for(int i=0;i<indices.size();i++){
                indicesArray[i] = indices[i];
            }
            mesh.swap(std::move(verticesArray)
            , std::move(texturesArray), std::move(normalsArray), std::move(indicesArray)
            , std::move(jointIdsArray), std::move(weightsArray));
        }
        void libraryAnimations(){
            String data;
            auto animationsNode = xmlRoot->getChild("library_animations");
            data = animationsNode->getChild("animation")
                                ->getChild("source")
                                ->getChild("float_array")
                                ->getData();
            auto rawTimesData = split(data,' ');
            Floats times(rawTimesData.size());
            for(int i=0;i<times.size();i++){
                times[i]=std::stof(rawTimesData[i]);
            }
            animationTimeDuration = times[times.size()-1];
            keyFrameDatas.resize(times.size());
            for(int i=0;i<keyFrameDatas.size();i++){
                keyFrameDatas[i] = KeyFrameData(times[i]);
            }
            String rootJointName = 
				xmlRoot->getChild("library_visual_scenes")
                        ->getChild("visual_scene")
                        ->getChildWithAttribute("node", "id", "Armature")
                        ->getChild("node")
                        ->getAttribute("id");
            auto& animationNodes = animationsNode->getChildren("animation");
            for(auto jointNode:animationNodes){
                String jointNameId = jointNode->getChild("channel")
                                    ->getAttribute("target");
                jointNameId=split(jointNameId,'/')[0];
                String dataId = jointNode->getChild("sampler")
                                        ->getChildWithAttribute("input", "semantic", "OUTPUT")
                                        ->getAttribute("source")
                                        .substr(1);
                data =jointNode->getChildWithAttribute("source", "id", dataId)
                                ->getChild("float_array")
                                ->getData();
                Strings rawMatrixData = split(data,' ');
                float matrixData[16];
                for(int i=0;i<keyFrameDatas.size();i++){
                    for(int j=0;j<16;j++){
                        matrixData[j] = std::stof(rawMatrixData[i*16+j]);
                    }
                    mat4 transform;
                    for(int c=0;c<4;c++){
                        for(int r=0;r<4;r++){//TODO
                            transform[c][r] = matrixData[c*4+r];
                        }
                    }
                    if(rootJointName==jointNameId){
                            transform = CORRECTION*transform;
                    }
                    keyFrameDatas[i].addJointTransform(JointTransformData(jointNameId,transform));
                }
            }
        }
        void init(){
            XmlParser parser;
            xmlRoot = parser.loadXmlFile(path);
            VertexSkinDataList vertexSkinDataList = libraryControllers();
            libraryVisualScenes();
            libraryGeometries(vertexSkinDataList);
            libraryAnimations();
        }
    private:
        FilePath path;
        bool done;
        ID id;
        XmlNode* xmlRoot;
        int maxWights;
        JointData rootJointData;
        MeshData mesh;
        KeyFramesData keyFrameDatas;
        int jointCount;
        float animationTimeDuration;
        JointListData jointOrder;
        float furthestPoint;
        static const mat4 CORRECTION;
        static ID nextId;
    };
    const mat4 ColladaLoader::CORRECTION{{1,0,0,0},{0,0,-1,0},{0,1,0,0},{0,0,0,0}};
    ID ColladaLoader::nextId = 0;
  
}

Loader* Loader::create(FilePath path,FileType type){
    return new ColladaLoader(path,3);
}

void Loader::destroy(Loader* loader){
    if(loader) delete loader;
}