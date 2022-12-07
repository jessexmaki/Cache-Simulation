#include <string>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <cmath>
#include <vector>

std::string convertToBinary(const std::string& hex)
{
    std::string binary;

    for (int i = 2; i < hex.size(); ++i)
    {
        switch (hex[i])
        {
            case '0':
                binary += "0000";
                break;
            case '1':
                binary += "0001";
                break;
            case '2':
                binary += "0010";
                break;
            case '3':
                binary += "0011";
                break;
            case '4':
                binary += "0100";
                break;
            case '5':
                binary += "0101";
                break;
            case '6':
                binary += "0110";
                break;
            case '7':
                binary += "0111";
                break;
            case '8':
                binary += "1000";
                break;
            case '9':
                binary += "1001";
                break;
            case 'a':
            case 'A':
                binary += "1010";
                break;
            case 'b':
            case 'B':
                binary += "1011";
                break;
            case 'c':
            case 'C':
                binary += "1100";
                break;
            case 'd':
            case 'D':
                binary += "1101";
                break;
            case 'e':
            case 'E':
                binary += "1110";
                break;
            case 'f':
            case 'F':
                binary += "1111";
                break;
        }
    }

    std::string leadingZero;

    while (binary.size() % 4 != 0)
    {
        leadingZero = "0" +  leadingZero;
        binary =  leadingZero + binary;
    }
    return binary;
}

// fully-associative
void readTraceFileFA(const int& replaceType, const std::string& fileName, int& hits, int& misses, const int& numBlocks, const int& cacheSize)
{
    std::ifstream inFile(fileName);
    std::string line;
    std::string addressBinary;
    std::list<std::string> tagsList;
    std::set<std::string> tags;

    while (getline(inFile, line))
    {
        std::istringstream stream(line);
        std::string addressHex;
        getline(stream, addressHex, ' ');
        addressBinary = convertToBinary(addressHex);
        int tagIndex = addressBinary.size() - cacheSize; std::string currentTag;

        for (int i = 0; i < tagIndex; ++i)
        {
            currentTag += addressBinary[i];
        }
        if (tags.find(currentTag) != tags.end()) {
            ++hits;
            if (replaceType == 2) {

                auto iter = tagsList.begin();
                while (iter != tagsList.end()) {
                    if (*iter == currentTag) {
                        std::string temp = *iter;
                        tagsList.erase(iter);
                        tagsList.push_back(temp);
                        break;
                    }
                    ++iter;
                }
            }
        }
        else if (tags.size() >= numBlocks)
        {
            ++misses;
            tags.erase(tags.find(tagsList.front()));
            tagsList.pop_front();
            tags.emplace(currentTag);
            tagsList.push_back(currentTag);
        }
        else
        {
            ++misses;
            tags.emplace(currentTag);
            tagsList.push_back(currentTag);
        }
    }
}

// direct-mapped
void readTraceFileDM(const std::string& fileName, int& hits, int& misses, int& cacheSize, int& numSets)
{
    std::ifstream inFile(fileName);
    std::map<std::string, std::string> tagMap;
    std::string addressLine;
    std::string addressBinary;

    while(getline(inFile, addressLine))
    {
        std::istringstream stream(addressLine);
        std::string addressHex;
        getline(stream, addressHex, ' ');
        addressBinary = convertToBinary(addressHex);
        int lineOffset = log2(numSets);
        int tagOffset = addressBinary.size() - cacheSize - lineOffset;
        std::string lineBits;

        for (int i = tagOffset; i < addressBinary.size() - cacheSize; ++i)
        {
            lineBits += addressBinary[i];
        }

        std::string currentTag;

        for (int i = 0; i < tagOffset; ++i)
        {
            currentTag += addressBinary[i];
        }

        if (tagMap.find(lineBits) != tagMap.end())
        {
            if (tagMap.find(lineBits)->second == currentTag)
            {
                ++hits;
            }
            else
            {
                ++misses;
                tagMap.find(lineBits)->second = currentTag;
            }
        }
        else
        {
            ++misses;
            tagMap.insert(make_pair(lineBits, currentTag));
        }
    }
}

// set-associative
void readTraceFileSA(const int& replaceType, const std::string& fileName, int& hits, int& misses, const int& numBlocks, const int& cacheSize, const int& numSets)
{
    std::ifstream inFile(fileName);
    std::string addressLine;
    std::string addressBinary;
    std::map<std::string, std::vector<std::string>> tagMap;
    std::map<std::string, std::list<std::string>> tagsList;

    while (getline(inFile, addressLine))
    {
        std::istringstream stream(addressLine);
        std::string addressHex;
        getline(stream, addressHex, ' ');
        addressBinary = convertToBinary(addressHex);
        int indexS = log2(numSets);
        std::string currentSet;

        for (int i = addressBinary.size() - cacheSize -  indexS; i < addressBinary.size() - cacheSize; ++i)
        {
            currentSet += addressBinary[i];
        }
        std::string currentTag;
        for (int i = 0; i < addressBinary.size() - cacheSize -  indexS; ++i)
        {
            currentTag += addressBinary[i];
        }
        if (tagMap.find(currentSet) != tagMap.end()) {
            std::list<std::string> &tagList = tagsList.find(currentSet)->second;
            std::vector<std::string> &tags = tagMap.find(currentSet)->second;
            bool hit = false;
            for (int i = 0; i < tags.size(); ++i) {
                if (tags[i] == currentTag) {
                    hit = true;
                }
            }
            if (hit) {
                ++hits;
                if (replaceType == 2) {
                    auto iter = tagList.begin();
                    while (iter != tagList.end()) {
                        if (*iter == currentTag) {
                            std::string temp = *iter;
                            tagList.erase(iter);
                            tagList.push_back(temp);
                            break;
                        }
                        ++iter;
                    }
                }
            }
            else if (tags.size() >= numBlocks) {
                ++misses;
                auto rIndex = tags.begin();
                for (int i = 0; i < tags.size(); ++i) {
                    if (tags[i] == tagList.front()) {
                        rIndex += i;
                    }
                }
                tags.erase(rIndex);
                tagList.pop_front();
                tags.push_back(currentTag);
                tagList.push_back(currentTag);
            }

            else {
                ++misses;
                tagList.push_back(currentTag);
                tags.push_back(currentTag);
            }
        }
        else
        {
            ++misses;
            std::vector<std::string> temp;
            temp.push_back(currentTag);
            tagMap.insert(make_pair(currentSet, temp));
            std::list<std::string>  tList;
            tList.push_back(currentTag);
            tagsList.insert((make_pair(currentSet, tList)));
            std::cout << "";
        }
    }
}



int main() {
    std::string fileName = " ";
    int hits = 0;
    int misses = 0;
    int cacheType;
    int numberOfBlocks;
    int cacheSize;
    int numberOfSets;
    int replaceType = 1;

    //Get user input for cache simulation
    std::cout << "Enter the file name: ";
    std::cin >> fileName;
    fileName = "traces/" + fileName;
    std::cout << "(1 is fully associative, 2 is direct-mapped, 3 is n-way associative)" << std::endl;
    std::cout << "Enter the cache type: ";
    std::cin >> cacheType;
    std::cout << "Enter the cache size: ";
    std::cin >> cacheSize;

    // fully associative
    if (cacheType == 1)
    {
        numberOfSets = 1;
        std::cout << "Enter the number of blocks: ";
        std::cin >> numberOfBlocks;
        std::cout << "1 (FIFO) ; 2 (LRU): ";
        std::cin >> replaceType;
        readTraceFileFA(replaceType, fileName, hits, misses, numberOfBlocks, cacheSize);
        std::cout << "Hits: " << hits << std::endl;
        std::cout << "Misses: " << misses << std::endl;
        std::cout << "Hits/Misses Ratio: " << ((double) hits /  (double) misses) << "%" << std::endl;
    }

        //direct-mapped
    else if (cacheType == 2)
    {
        numberOfBlocks = 1;
        std::cout << "Enter the number of sets: ";
        std::cin >> numberOfSets;
        readTraceFileDM(fileName, hits, misses, cacheSize, numberOfSets);
        std::cout << "Hits: " << hits << std::endl;
        std::cout << "Misses: " << misses << std::endl;
        std::cout << "Hits/Misses Ratio: " << ((double) hits /  (double) misses) << "%" << std::endl;
    }

        //n-way associative
    else if (cacheType == 3)
    {
        std::cout << "Enter the number of sets: ";
        std::cin >> numberOfSets;
        std::cout << "Enter the number of blocks: ";
        std::cin >> numberOfBlocks;
        std::cout << "1 (FIFO) ; 2 (LRU): ";
        std::cin >> replaceType;
        readTraceFileSA(replaceType, fileName, hits, misses,numberOfBlocks, cacheSize, numberOfSets);
        std::cout << "Hits: " << hits << std::endl;
        std::cout << "Misses: " << misses << std::endl;
        std::cout << "Hits/Misses Ratio: " << ((double) hits /  (double) misses) << "%" << std::endl;
    }
    return 0;
}
