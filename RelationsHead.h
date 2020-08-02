#ifndef RELATIONSHEAD_H
#define RELATIONSHEAD_H

#include <string>
#include <map>
#include <set>
#include <vector>

using std::endl;
using std::string;
using std::ifstream;
using std::map;
using std::set;
using std::vector;

using namespace std;


class RelationsUnionInfo
{
	public:
	RelationsUnionInfo()
	{

	}

	RelationsUnionInfo(double numTuples){
		this->numTuples=numTuples;
	}

    RelationsUnionInfo(string relation, double numTuples)
	{
		this->relationsSet.insert(relation);
		this->numTuples = numTuples;
	}

	~RelationsUnionInfo(){

	}

	double getNumTuples()
	{
		return this->numTuples;
	}

	void setNumTuples(double numTuples)
	{
		this->numTuples = numTuples;
	}

    set<string> getRelationsSet()
    {
        return this->relationsSet;
    }

	void insertToRelationsSet(string s)
	{
		this->relationsSet.insert(s);
	}

	int getRelationsSetSize()
	{
		return this->relationsSet.size();
	}

    void readInputFileStream(ifstream &reader)
    {
        string currentLine;

        getline(reader, currentLine);
		numTuples = stoi(currentLine);

        getline(reader, currentLine);
		int count = stoi(currentLine);
        while(count>0){
            getline(reader, currentLine);
            relationsSet.insert(currentLine);
			count--;
        }
    }

	private:
		set<string> relationsSet;
		double numTuples;
};


class RelationInfo
{
	public:
	RelationInfo()
	{

	}
	RelationInfo(int numTuples)
	{
		this->numTuples = numTuples;
	}

	~RelationInfo(){

	}

	int getNumTuples(){
		return numTuples;
	}

	void setNumTuples(int numTuples)
	{
	    this->numTuples = numTuples;
	}

	int getAttribute(string attrName)
	{
		return attributesDistMap[attrName];
	}

	void addAttribute(string attName, int numDistincts)
	{
		attributesDistMap[attName] = numDistincts==-1? this->numTuples: numDistincts;
	}

	void setAttributesMap(map<string, int> inMap){
		this->attributesDistMap.clear();
        this->attributesDistMap = inMap;
	}

	map<string, int> getAttributes()
    {
        return attributesDistMap;
    }

    int getAttributeMapSize()
    {
        return attributesDistMap.size();
    }

	void readInputFileStream(ifstream &reader)
	{
		string currentLine;
		getline(reader, currentLine);
		numTuples = stoi(currentLine);

		getline(reader, currentLine);
		int attributesCount = stoi(currentLine);

		while(attributesCount>0){
			getline(reader, currentLine);
			string attName = currentLine;

			getline(reader, currentLine);
			attributesDistMap[attName] = stoi(currentLine);
			attributesCount--;
		}
	}

	private:
	int numTuples;
	map<string, int> attributesDistMap;
};



#endif