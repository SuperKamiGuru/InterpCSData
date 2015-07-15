// Created by: Wesley Ford

using namespace std;

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <dirent.h>
#include <new>
#include "include/ElementNames.hh"
#include "zlib.h"
#include <iomanip>


void GetClosestTempFileList(string inFileName, string outFileName, stringstream& streamMCNP, std::vector<string> &inFileListLow,
                            std::vector<string> &inFileListHigh, std::vector<string> &outFileList, std::vector<double> &prevTempListLow,
                            std::vector<double> &prevTempListHigh, std::vector<double> &newTempList);
bool GetAllFiles(string inFileName, std::vector<string> &inFileList);
bool FindTemp(string name, double &tempMatch);
bool FindTemp(string name, double tempMatch, int &count);
bool FindProcess(string fileName, int &process);
bool CompleteFile(string fileName);
void SwapListElem(std::vector<string> &inFileList, std::vector<string> &outFileList, std::vector<double> &newTempList, std::vector<double> &prevTempList, int i, int j);
bool CompareIsotopeNum(string name1, string name2, string comparison);
void InsertListElem(std::vector<string> &inFileList, std::vector<string> &outFileList, std::vector<double> &newTempList, std::vector<double> &prevTempList, int startPos, int endPos);
bool DirectoryExists( const char* pzPath );
bool FindDir(string &inFileName, string &outFileName, double prevTemp, double newTemp);
bool ExtractDouble(string name, double &num);
void GetClosestTempDir(string &inFileName, double &prevTemp, double newTemp);
bool ConvertFile(string inFileName, string outFileName, double prevTemp, double newTemp, bool ascii, bool log, std::ofstream* logFile, bool overWrite);
bool ConvertFile(string inDirName, string outDirName, string fileName, double prevTemp, double newTemp, bool ascii, bool log, std::ofstream* logFile, bool overWrite);
void ExtractZA(string fileName, int &Z, int &A);
int DoppBroad(string inFileLow, string inFileHigh, string outFile, double prevTempLow, double prevTempHigh, double newTemp, bool ascii, bool overWrite);
double findCS(double nKEnerTrans, const double* prevEnVec, const double* prevCSVec, int vecSize);
void GetDataStream( string, std::stringstream&);
void SetDataStream( string, std::stringstream&, bool ascii);
double Linear(double x, double x1, double x2, double y1, double y2);

std::ofstream *logFileData=NULL;

//
//doppler broadens the given data to the given temperature using multiple processors
int main(int argc, char **argv)
{

    ElementNames elementNames;
    elementNames.SetElementNames();

    //double duration;
    int result=0;
    bool overWrite =true;
    string dummy;
    string macroFileName;
    string inFileName, outFileName;
    string inSubDirName, outSubDirName;
    string outputType="false", overWriteFile="true";
    double prevTemp;
    bool ascii=true;
    stringstream streamMCNP;

    //gets user inputs
    if(argc==2)
    {
        streamMCNP << argv[1];
        streamMCNP >> macroFileName;
        if((macroFileName=="help")||(macroFileName=="Help")||(macroFileName=="h")||(macroFileName=="H"))
        {
            cout << "\n### The Potential User Inputs For DopplerBroadData Are:\n" <<
            "inFileName: (file name/directory name) The name of the cross-section data file to be broadened or\n" <<
            " the name of the directory containing the cross-section files to be broadened\n" <<
            "outFileName: (file name/directory name) The name of the cross-section data file for the broadened data to be placed or\n" <<
            " the name of the directory for the broadened cross-section files to be placed\n" <<
            "prevTemp: (number) the temperature of the cross-section data contianed by inFileName, Note: if the\n" <<
            " closestTemp flag is set this input will be ignored\n" <<
            "newTemp: (number) the temperature that the output files will be broadened to, Note if the macroFile\n" <<
            " option is used then this input will be ignored\n" <<
            "outputType: (Default=ascii/compress/...) this determines the format of the output files, if ascii is select then normall\n" <<
            " text files are created if compress is selected then .z compressed files are created\n" <<
            "createLogFile: (Default=false/true) this determines whether a log file is created to contian any errors or signeificant events\n" <<
            "overWriteFile: (Default=true/false) this determines whether existing Doppler broadened files will be regenerated if they have the same name\n" <<
            "See manual for more detials about the inputs and what they do ###" << endl;
            return 0;
        }

        else
        {
            streamMCNP.str("");
            streamMCNP.clear();

            GetDataStream( macroFileName, streamMCNP);

            if(streamMCNP.str()!="")
            {
                //gets inputs in macrofile
                int numArg;
                streamMCNP >> numArg;

                if(numArg==8)
                {
                    streamMCNP >> inFileName >> outFileName >> dummy >> prevTemp >> outputType >> dummy >> overWriteFile;
                }
                else if(numArg==7)
                {
                    streamMCNP >> inFileName >> outFileName >> dummy >> prevTemp >> outputType >> dummy;
                }
                else if(numArg==6)
                {
                    streamMCNP >> inFileName >> outFileName >> dummy >> prevTemp >> outputType;
                }
                else if(numArg==5)
                {
                    streamMCNP >> inFileName >> outFileName >> dummy >> prevTemp;
                }
            }
            else
            {
                cout << "\n### Error Invalid Input, type DoppBroad help to see valid inputs ###" << endl;
                return 1;
            }
        }
    }
    else
    {
        cout << "\n### Error Invalid Number Of Inputs\n" << "The Potential User Inputs For DopplerBroadData Are:\n" <<
            "inFileName: (file name/directory name) The name of the cross-section data file to be broadened or\n" <<
            " the name of the directory containing the cross-section files to be broadened\n" <<
            "outFileName: (file name/directory name) The name of the cross-section data file for the broadened data to be placed or\n" <<
            " the name of the directory for the broadened cross-section files to be placed\n" <<
            "prevTemp: (number) the temperature of the cross-section data contianed by inFileName, Note: if the\n" <<
            " closestTemp flag is set this input will be ignored\n" <<
            "newTemp: (number) the temperature that the output files will be broadened to, Note if the macroFile\n" <<
            " option is used then this input will be ignored\n" <<
            "outputType: (Default=ascii/compress/...) this determines the format of the output files, if ascii is select then normall\n" <<
            " text files are created if compress is selected then .z compressed files are created\n" <<
            "createLogFile: (Default=false/true) this determines whether a log file is created to contian any errors or signeificant events\n" <<
            "overWriteFile: (Default=true/false) this determines whether existing Doppler broadened files will be regenerated if they have the same name\n" <<
            "See manual for more detials about the inputs and what they do ###" << endl;
            return 1;
    }

    if(outputType == "compressed"||outputType == "compress"||outputType == "Compressed"||outputType == "Compress"||outputType == "Zipped"||outputType == "Zip"||outputType == "zipped"||outputType == "zip" )
        ascii=false;

    if(overWriteFile == "false"||overWriteFile == "False"||overWriteFile == "0"||overWriteFile == "Off"||overWriteFile == "off")
        overWrite=false;

// creates doppler broadened files using the settings in the macrofile
    std::vector<double> prevTempListLow, prevTempListHigh, newTempList;
    std::vector<string> inFileListLow, inFileListHigh, outFileList;
    string inDirName, outDirName, fileName;
    int temp=1;

    // finds the files with temperatures closest to that of the temperatures that the files are to be broadened too and puts them in a list
    GetClosestTempFileList(inFileName, outFileName, streamMCNP, inFileListLow, inFileListHigh, outFileList, prevTempListLow, prevTempListHigh, newTempList);

    cout << "\n" << outFileList.size() << " Files to be Broadened" << endl;

    cout << "\n" << endl;

    for(int i=0; i<int(outFileList.size()); i++)
    {
        outDirName = (outFileList[i]).substr(0,(outFileList[i]).find_last_of('/')+1);

        // creates the output directory if it doesn't already exist
        if(!(DirectoryExists(outDirName.c_str())))
        {
            temp = system( ("mkdir -p -m=666 "+outDirName).c_str());
            if(DirectoryExists(outDirName.c_str()))
            {
                cout << "### Created Output Directory " << outDirName << " ###" << endl;
                temp=1;
            }
        }
        if(temp)
        {
            // makes sure that the input file exists and is complete
            if((CompleteFile(inFileListLow[i]))&&(CompleteFile(inFileListHigh[i])))
            {
                DoppBroad(inFileListLow[i], inFileListHigh[i], outFileList[i], prevTempListLow[i], prevTempListHigh[i], newTempList[i], ascii, overWrite);
            }
        }
    }

    streamMCNP.str("");
    streamMCNP.clear();
    elementNames.ClearStore();

    return result;

}

//GetClosestTempFileList
//Finds the files with temperatures closest to the temperatures that the files are to be broadened to and places them in a list
void GetClosestTempFileList(string inFileName, string outFileName, stringstream& streamMCNP, std::vector<string> &inFileListLow,
                            std::vector<string> &inFileListHigh, std::vector<string> &outFileList, std::vector<double> &prevTempListLow,
                            std::vector<double> &prevTempListHigh, std::vector<double> &newTempList)
{
    int bestLow[4], bestHigh[4], index, numIso, count, Z, A;
    string isoName, name, outName, nameTemp;
    double temp, tempMatch;
    double tempBestLow[4]={0.,0.,0.,0.};
    double tempBestHigh[4]={0.,0.,0.,0.};
    std::vector<string> allFiles, matchList;
    bool check[4]={false, false, false, false};
    string process[4]={"Capture","Elastic","Fission","Inelastic"};

    streamMCNP >> numIso;
    GetAllFiles(inFileName, allFiles);

    for(int i=0; i<numIso; i++)
    {
        streamMCNP >> isoName >> temp >> nameTemp;
        ExtractZA(isoName,Z,A);
        for(int j=0; j<4; j++)
        {
            check[j]=false;
            tempBestLow[j]=-1.;
        }
        for(int j=0; j<int(allFiles.size()); j++)
        {
            name = (allFiles[j]).substr((allFiles[j]).find_last_of('/')+1, std::string::npos);
            name = name.substr(0, name.find_last_of('.'));

            if(CompareIsotopeNum(name, isoName, "=="))
            {
                if(FindTemp(allFiles[j], tempMatch))
                {
                    if(FindProcess(allFiles[j], index))
                    {
                        if(((temp-tempBestLow[index])>=(temp-tempMatch))&&(tempMatch<=temp))
                        {
                            if(CompleteFile(allFiles[j]))
                            {
                                tempBestLow[index] = tempMatch;
                                bestLow[index] = j;
                                check[index] = true;
                            }
                        }
                    }
                }
            }
        }
        for(int j=0; j<4; j++)
        {
            check[j]=false;
            tempBestHigh[j]=-1.;
        }
        for(int j=0; j<int(allFiles.size()); j++)
        {
            name = (allFiles[j]).substr((allFiles[j]).find_last_of('/')+1, std::string::npos);
            name = name.substr(0, name.find_last_of('.'));

            if(CompareIsotopeNum(name, isoName, "=="))
            {
                if(FindTemp(allFiles[j], tempMatch))
                {
                    if(FindProcess(allFiles[j], index))
                    {
                        if(((temp-tempBestHigh[index])>=(temp-tempMatch))&&(tempMatch>=temp))
                        {
                            if(CompleteFile(allFiles[j]))
                            {
                                tempBestHigh[index] = tempMatch;
                                bestHigh[index] = j;
                                check[index] = true;
                            }
                        }
                    }
                }
            }
        }
        for(int k=0; k<4; k++)
        {
            if(check[k])
            {
                newTempList.push_back(temp);
                inFileListLow.push_back(allFiles[bestLow[k]]);
                inFileListHigh.push_back(allFiles[bestHigh[k]]);
                prevTempListLow.push_back(tempBestLow[k]);
                prevTempListHigh.push_back(tempBestHigh[k]);
                // create out file directory from temperature that the file will be doppler broadened too
                FindTemp(allFiles[bestHigh[k]], tempBestHigh[k], count);
                outName = outFileName+nameTemp+'k'+'/'+(allFiles[bestHigh[k]]).substr(count+1, std::string::npos);
                outFileList.push_back(outName);
            }
            else if((k!=2)||(Z>87))
            {
                cout << "Error: the input CS file for isotope " << isoName << " could not be found for process " << process[k] << "\n" << endl;
            }
        }
    }
}

//GetAllFiles
//creates a list off all of the files contianed in the given directory and the subdirectories inside of it
bool GetAllFiles(string inFileName, std::vector<string> &inFileList)
{
    DIR *dir;
    struct dirent *ent;
    string name;

    if ((dir = opendir (inFileName.c_str())) != NULL)
    {
      while ((ent = readdir (dir)) != NULL)
      {
        if((string(ent->d_name)!="..")&&(string(ent->d_name)!="."))
        {
            if (GetAllFiles(inFileName+ent->d_name+"/", inFileList))
            {

            }
            else
            {
                inFileList.push_back(inFileName+ent->d_name);
            }
        }
      }
    }
    else
    {
        return false;
    }

    return true;
}

//FindTemp
//Finds the temperature directory that the file is stored in
bool FindTemp(string name, double &tempMatch)
{
    bool foundTemp=false;
    stringstream numConv;
    int pos1=name.length(), pos2;
    string partName;

    while((pos1!=0)&&(!foundTemp))
    {
        pos2=name.find_last_of('/', pos1);
        pos1=name.find_last_of('/', pos2-1);
        partName=name.substr(pos1+1, pos2-pos1-1);

        if(((partName[0]>='0')&&(partName[0]<='9'))||(partName[0]=='.'))
        {
            foundTemp=true;
            numConv << partName[0];
            for(int i=1; i<int(partName.length()); i++)
            {
                if(((partName[i]>='0')&&(partName[i]<='9'))||(partName[i]=='.'))
                {
                    numConv << partName[i];
                }
            }
        }

    }

    if(foundTemp)
    {
        numConv >> tempMatch;
    }

    numConv.str("");

    return foundTemp;
}

//FindTemp
//Finds the temperature directory that the file is stored in and the postion along the file name that the temperature directory is listed
bool FindTemp(string name, double tempMatch, int &count)
{
    bool foundTemp=false;
    stringstream numConv;
    int pos1=name.length(), pos2;
    string partName;
    count=0;
    double match;

    while((pos1!=0)&&(!foundTemp))
    {
        pos2=name.find_last_of('/', pos1);
        pos1=name.find_last_of('/', pos2-1);
        partName=name.substr(pos1+1, pos2-pos1-1);

        if(((partName[0]>='0')&&(partName[0]<='9'))||(partName[0]=='.'))
        {
            foundTemp=true;
            numConv << partName[0];
            count=pos2;
            for(int i=1; i<int(partName.length()); i++)
            {
                if(((partName[i]>='0')&&(partName[i]<='9'))||(partName[i]=='.'))
                {
                    numConv << partName[i];
                }
            }
        }

    }

    if(foundTemp)
    {
        numConv >> match;
        foundTemp = match==tempMatch;
    }

    numConv.str("");

    return foundTemp;
}

//FindProcess
//finds which process directory the given file is within
bool FindProcess(string fileName, int &process)
{
    bool found=false;
    int pos1=fileName.length(), pos2;
    string partName;

    while((pos1!=0)&&(!found))
    {
        pos2=fileName.find_last_of('/', pos1);
        pos1=fileName.find_last_of('/', pos2-1);
        partName=fileName.substr(pos1+1, pos2-pos1-1);

        if(partName=="capture"||partName=="Capture")
        {
            process=0;
            found=true;
        }
        else if(partName=="elastic"||partName=="Elastic")
        {
            process=1;
            found=true;
        }
        else if(partName=="fission"||partName=="Fission")
        {
            process=2;
            found=true;
        }
        else if(partName=="inelastic"||partName=="Inelastic"||partName=="inElastic"||partName=="InElastic")
        {
            process=3;
            found=true;
        }

    }

    return found;
}

//CompleteFile
//Determines if the given file exists and if it is complete or not
bool CompleteFile(string fileName)
{
    stringstream stream;
    GetDataStream( fileName, stream);
    int count=0, numPoints;
    double dummy;

    if(stream.str()!="")
    {
        //skips teo dummy variables and gets the number of CS points
        stream >> numPoints;
        stream >> numPoints;
        stream >> numPoints;

        while(stream)
        {
            stream >> dummy >> dummy;
            count++;
        }

        count--;

        if(count==numPoints)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
        return false;
}

//SwapListElem
//swaps the postions of elements i and j for each of the given lists
void SwapListElem(std::vector<string> &inFileList, std::vector<string> &outFileList, std::vector<double> &newTempList, std::vector<double> &prevTempList, int i, int j)
{
    double newTemp, prevTemp;
    string inName, outName;

    newTemp=newTempList[i];
    prevTemp=prevTempList[i];
    inName=inFileList[i];
    outName=outFileList[i];

    newTempList[i]=newTempList[j];
    prevTempList[i]=prevTempList[j];
    inFileList[i]=inFileList[j];
    outFileList[i]=outFileList[j];

    newTempList[j]=newTemp;
    prevTempList[j]=prevTemp;
    inFileList[j]=inName;
    outFileList[j]=outName;
}

// CompareIsotopeNum
//Compares the isotope numbers
bool CompareIsotopeNum(string name1, string name2, string comparison)
{
    stringstream numConv;
    int Z1=0, A1=0, Z2=0, A2=0, num1, num2;

    name1 = (name1).substr(name1.find_last_of('/')+1, std::string::npos);
    name1 = name1.substr(0, name1.find_last_of('.'));
    ExtractZA(name1, Z1, A1);

    name2 = (name2).substr(name2.find_last_of('/')+1, std::string::npos);
    name2 = name2.substr(0, name2.find_last_of('.'));
    ExtractZA(name2, Z2, A2);

    if((A1==0)||(A2==0))
    {
        if(comparison=="==")
        {
            if(Z1==Z2)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else if(comparison==">")
        {
            if(Z1>Z2)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else if(comparison=="<")
        {
            if(Z1<Z2)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            cout << "\nError Comparison symbol is not recognized in CompareIsotopeNum\n" << endl;
            return false;
        }
    }

    num1 = Z1*1000+A1;
    num2 = Z2*1000+A2;

    if(comparison=="==")
    {
        if(num1==num2)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if(comparison==">")
    {
        if(num1>num2)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if(comparison=="<")
    {
        if(num1<num2)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        cout << "\nError Comparison symbol is not recognized in CompareIsotopeNum\n" << endl;
        return false;
    }
}

void InsertListElem(std::vector<string> &inFileList, std::vector<string> &outFileList, std::vector<double> &newTempList, std::vector<double> &prevTempList, int startPos, int endPos)
{
    newTempList.insert(newTempList.begin()+endPos, newTempList[startPos]);
    prevTempList.insert(prevTempList.begin()+endPos, prevTempList[startPos]);
    inFileList.insert(inFileList.begin()+endPos, inFileList[startPos]);
    outFileList.insert(outFileList.begin()+endPos, outFileList[startPos]);

    newTempList.erase(newTempList.begin()+startPos);
    prevTempList.erase(prevTempList.begin()+startPos);
    inFileList.erase(inFileList.begin()+startPos);
    outFileList.erase(outFileList.begin()+startPos);
}

// FindDir
// finds the temperature directory in the inFileName path that corresponds to the given previous temperature
// finds/sets the temperature directory in the outFileName path that corresponds to the given new temperature
bool FindDir(string &inFileName, string &outFileName, double prevTemp, double newTemp)
{
    DIR *dir;
    struct dirent *ent;
    bool check=false;
    stringstream numConv;
    double temp;
    const string originalIn=inFileName;
    string outTemp, fileName;

    numConv << newTemp;
    numConv >> outTemp;

    if ((dir = opendir (inFileName.c_str())) != NULL)
    {
      while ((ent = readdir (dir)) != NULL)
      {
        fileName=string(ent->d_name);
        if((fileName!="..")&&(fileName!="."))
        {
            if ((ExtractDouble( fileName, temp))&&(temp==prevTemp)&&((fileName.back()=='k')||(fileName.back()=='K')))
            {
                inFileName=inFileName+ent->d_name+'/';
                // else if the inFileName doesn't already contain the temperature directory search in the given input file directory for a temperature directory with temperature prevTemp
                double tempEx=-100;
                string partName = outFileName.substr(outFileName.find_last_of('/',outFileName.length()-2), string::npos);
                if(!(ExtractDouble(partName, tempEx)&&(tempEx==newTemp)))
                {
                     outFileName=outFileName+outTemp+'k'+'/';
                }
                check=true;
                break;
            }
            else
            {
                inFileName=inFileName+string(ent->d_name)+'/';
                check=FindDir(inFileName, outFileName, prevTemp, newTemp);
                if(check)
                {
                    break;
                }
                else
                {
                    inFileName=originalIn;
                }
            }
        }
      }
    }
    else
    {
        check= false;
    }

    return check;
}

//ExtractDouble
//extracts the first double it finds in the string starting from the end
bool ExtractDouble(string name, double &num)
{
    bool foundDouble = false;
    stringstream numConv;
    for(int i=0; i<int(name.size()); i++)
    {
        if(((name[i]>='0')&&(name[i]<='9'))||(name[i]=='.'))
        {
            foundDouble=true;
            numConv << (name)[i];
        }
    }
    if(foundDouble)
        numConv >> num;
    return foundDouble;
}

//GetClosestTempDir
//Finds the directory with the temperature closest to newTemp
void GetClosestTempDir(string &inFileName, double &prevTemp, double newTemp)
{
    string closestName, temp;
    double closest=0, test=0;
    stringstream numConv;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (inFileName.c_str())) != NULL)
    {
      /* print all the files and directories within directory */
      while ((ent = readdir (dir)) != NULL)
      {
        if(((ent->d_name)[0]>='0')&&((ent->d_name)[0]<='9'))
        {
            numConv << ent->d_name;
            numConv >> test;

            if(((newTemp-test)<=(newTemp-closest))&&(test<=newTemp))
            {
                closest=test;
                closestName = inFileName+ent->d_name+'/';
            }

        }
        else
        {
            temp=inFileName+ent->d_name+'/';
            GetClosestTempDir(temp, test, newTemp);
            if(((newTemp-test)<=(newTemp-closest))&&(test<=newTemp))
            {
                closest=test;
                closestName = temp;
            }
        }
      }
      closedir(dir);
    }

    prevTemp=closest;
    inFileName=closestName;
}

//DirectoryExists
//Checks if the given directory exists
bool DirectoryExists( const char* pzPath )
{
    if ( pzPath == NULL) return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir (pzPath);

    if (pDir != NULL)
    {
        bExists = true;
        closedir (pDir);
    }

    return bExists;
}

//ExtractZA
//extracts the Z and the A isotope numbers from the file name
void ExtractZA(string fileName, int &Z, int &A)
{
        std::size_t startPos=0;
        stringstream ss;
        ElementNames* elementNames;
        while(startPos!=fileName.length() && (fileName[startPos]<'0' || fileName[startPos]>'9'))
            startPos++;

        if(startPos==fileName.length())
        {
            cout << "### File Name Does Not Contian a Z or an A Value " << fileName << " is Invalid for Broadening ###" << endl;
            Z=A=-1;
        }
        else
        {
        ////
            std::size_t found1 = fileName.find_first_of('_', startPos);
            if (found1==std::string::npos)
            {
                cout << "### File Name Does Not Contian a '_', two are needed, one to seperate the Z and A value, \
                and one to seperate the A and the Element name " << fileName << " is Invalid for Broadening ###" << endl;
                Z=A=-1;
            }
            else
            {
                std::size_t found2 = fileName.find_first_of('_', found1+1);
                if (found2==std::string::npos)
                {
                    cout << "### File Name Does Not Contian a second '_', two are needed, one to seperate the Z and A value, \
                    and one to seperate the A and the Element name " << fileName << " is Invalid for Broadening ###" << endl;
                    Z=A=-1;
                }
                else
                {

                    ss.str(fileName.substr(startPos, found1));
                    ss >> Z;
                    ss.str("");
                    ss.clear();
                    if(((found2-found1-1) > 2) && (fileName[found2-2] == 'm'))
                        ss.str(fileName.substr(found1+1, found2-found1-3));
                    else
                        ss.str(fileName.substr(found1+1, found2-found1-1));
                    ss >> A;
                    ss.str("");
                    ss.clear();
                    ss.str(fileName.substr(found2+1));
                    if (!(elementNames->CheckName(ss.str(), Z)))
                    {
                        cout << "### " << fileName << " does not include the correct element name at the end ###" << endl;
                        Z=A=-1;
                    }
                    ss.str("");
                    ss.clear();
                }

            }

        }
}

//DoppBroad
//Doppler broadens the given file
int DoppBroad(string inFileLow, string inFileHigh, string outFile, double prevTempLow, double prevTempHigh, double newTemp, bool ascii, bool overWrite)
{
	if (inFileLow==inFileHigh)
	{
        return 0;
    }

    double tempRatio = (newTemp-prevTempLow)/(newTemp-prevTempHigh);

    stringstream stream;
    GetDataStream(inFileLow, stream);

    int tableSize, vType;
    stream >> tableSize >> vType;

    // contents
    int sizLow=0;
    stream >> sizLow;
    if (stream.fail())
    { stream.str(""); return 1; }
    if (sizLow<=0)
    {
        cout << " Invalid vector size: " << sizLow << endl;
        stream.str("");
        return 1;
    }

    double tempEn, tempCS;
    double* prevEnVecLow = new double[sizLow];
    double* prevCSVecLow = new double[sizLow];

    for(int i = 0; i < sizLow ; i++)
    {
        tempEn = 0.;
        tempCS = 0.;
        stream >> tempEn >> tempCS;

        if (stream.fail())
        { stream.str(""); return 1; }

        prevEnVecLow[i] = tempEn;
        prevCSVecLow[i] = tempCS;
    }

     stream.str("");
     stream.clear();

    GetDataStream(inFileHigh, stream);

    stream >> tableSize >> vType;

    // contents
    int sizHigh=0;
    stream >> sizHigh;
    if (stream.fail())
    {
        stream.str("");
        delete [] prevCSVecLow;
        delete [] prevEnVecLow;
        return 1;
    }

    if (sizLow<=0)
    {
        cout << " Invalid vector size: " << sizHigh << endl;
        stream.str("");
        return 1;
    }
    double* prevEnVecHigh = new double[sizHigh];
    double* prevCSVecHigh = new double[sizHigh];

    for(int i = 0; i < sizHigh ; i++)
    {
        tempEn = 0.;
        tempCS = 0.;
        stream >> tempEn >> tempCS;

        if (stream.fail())
        { stream.str(""); return 1; }

        prevEnVecHigh[i] = tempEn;
        prevCSVecHigh[i] = tempCS;
    }

     stream.str("");
     stream.clear();

    vector<double> newEnVec, newCSVec;
    int i=0, j=0;
    while((i<sizLow)&&(j<sizHigh))
    {
        if(prevEnVecLow[i]<prevEnVecHigh[j])
        {
            newEnVec.push_back(prevEnVecLow[i]);
            if(j==0)
                newCSVec.push_back((1-tempRatio)*prevCSVecLow[i]+tempRatio*Linear(prevEnVecLow[i], prevEnVecHigh[j], prevEnVecHigh[j+1], prevCSVecHigh[j], prevCSVecHigh[j+1]));
            else
                newCSVec.push_back((1-tempRatio)*prevCSVecLow[i]+tempRatio*Linear(prevEnVecLow[i], prevEnVecHigh[j-1], prevEnVecHigh[j], prevCSVecHigh[j-1], prevCSVecHigh[j]));
            i++;
        }
        else if(prevEnVecLow[i]>prevEnVecHigh[j])
        {
            newEnVec.push_back(prevEnVecHigh[j]);
            if(i==0)
                newCSVec.push_back((1-tempRatio)*Linear(prevEnVecHigh[j], prevEnVecLow[i], prevEnVecLow[i+1], prevCSVecLow[i], prevCSVecLow[i+1])+tempRatio*prevCSVecHigh[j]);
            else
                newCSVec.push_back((1-tempRatio)*Linear(prevEnVecHigh[j], prevEnVecLow[i-1], prevEnVecLow[i], prevCSVecLow[i-1], prevCSVecLow[i])+tempRatio*prevCSVecHigh[j]);
            j++;
        }
        else
        {
            newEnVec.push_back(prevEnVecHigh[j]);
            newCSVec.push_back((1-tempRatio)*prevCSVecLow[i]+tempRatio*prevCSVecHigh[j]);
            i++; j++;
        }
    }

    for(;i<int(sizLow);i++)
    {
        newEnVec.push_back(prevEnVecLow[i]);
        newCSVec.push_back((1-tempRatio)*prevCSVecLow[i]+tempRatio*Linear(prevEnVecLow[i], prevEnVecHigh[sizHigh-2], prevEnVecHigh[sizHigh-1], prevCSVecHigh[sizHigh-2], prevCSVecHigh[sizHigh-1]));
    }

    for(;j<int(sizHigh);j++)
    {
        newEnVec.push_back(prevEnVecHigh[j]);
        newCSVec.push_back((1-tempRatio)*Linear(prevEnVecHigh[j], prevEnVecLow[sizLow-2], prevEnVecLow[sizLow-1], prevCSVecLow[sizLow-2], prevCSVecLow[sizLow-1])+tempRatio*prevCSVecHigh[j]);
    }

    if(ascii)
    {
        stream.fill(' ');
        stream << std::setw(14) << std::right << tableSize << "\n" << std::setw(14) << vType << "\n" << std::setw(14) << newCSVec.size() << "\n";
        stream.precision(6);
        stream.setf(std::ios::scientific);
    }

     else
        stream << tableSize << "\n" << vType << "\n" << newCSVec.size() << "\n";

    for(i=0; i<int(newCSVec.size()); i++)
    {
        if (ascii)
        {
            if(!((i)%3) && i!=0 )
                stream << "\n";
            stream << std::setw(14) << newEnVec[i] << std::setw(14) << newCSVec[i];
        }
        else
            stream << newEnVec[i] << ' ' << newCSVec[i] << ' ';
    }

    stream << '\n';

    SetDataStream( outFile, stream, ascii);

    stream.str("");
    delete [] prevCSVecLow;
    delete [] prevEnVecLow;
    delete [] prevCSVecHigh;
    delete [] prevEnVecHigh;
    return 0 ;
}

double Linear(double x, double x1, double x2, double y1, double y2)
{
  double slope=0, off=0;
  if(x2-x1==0) return (y2+y1)/2.;
  slope = (y2-y1)/(x2-x1);
  off = y2-x2*slope;
  double y = x*slope+off;
  return y;
}

//GetDataStream
//Gets the contents of the given file and puts it into a data stream
void GetDataStream( string filename , std::stringstream& ss)
{
   string* data=NULL;
   std::ifstream* in=NULL;
   //string compfilename(filename);

   if(filename.substr((filename.length()-2),2)==".z")
   {
        in = new std::ifstream ( filename.c_str() , std::ios::binary | std::ios::ate );
   }

   if ( in!=NULL && in->good() )
   {
// Use the compressed file
      uLongf file_size = (uLongf)(in->tellg());
      in->seekg( 0 , std::ios::beg );
      Bytef* compdata = new Bytef[ file_size ];

      while ( *in )
      {
         in->read( (char*)compdata , file_size );
      }

      uLongf complen = (uLongf) ( file_size*4 );
      Bytef* uncompdata = new Bytef[complen];

      while ( Z_OK != uncompress ( uncompdata , &complen , compdata , file_size ) )
      {
         delete[] uncompdata;
         complen *= 2;
         uncompdata = new Bytef[complen];
      }
      delete [] compdata;
      //                                 Now "complen" has uncomplessed size
      data = new string ( (char*)uncompdata , (long)complen );
      delete [] uncompdata;
   }
   else {
// Use regular text file
      std::ifstream thefData( filename.c_str() , std::ios::in | std::ios::ate );
      if ( thefData.good() )
      {
         int file_size = thefData.tellg();
         thefData.seekg( 0 , std::ios::beg );
         char* filedata;
         if(file_size>0)
            filedata = new char[ file_size ];
         else
         {
            if(in!=NULL)
            {
                in->close();
                delete in;
            }
            return;
         }
         while ( thefData )
         {
            thefData.read( filedata , file_size );
         }
         thefData.close();
         data = new string ( filedata , file_size );
         delete [] filedata;
      }
      else
      {
// found no data file
//                 set error bit to the stream
         ss.setstate( std::ios::badbit );
         cout << endl << "### failed to open ascii file " << filename << " ###" << endl;
      }
   }
   if (data != NULL)
   {
        ss.str(*data);
        if(data->back()!='\n')
            ss << "\n";
        ss.seekg( 0 , std::ios::beg );
   }

   if(in!=NULL)
   {
        in->close();
        delete in;
   }

   delete data;
}

//SetDataStream
//opens the given file and stores the data contianed with in the given stream inside of it
void SetDataStream( string filename , std::stringstream& ss, bool ascii )
{
    //bool cond=true;
   if (!ascii)
   {
        string compfilename(filename);

        if(compfilename.back()!='z')
            compfilename += ".z";

       std::ofstream* out = new std::ofstream ( compfilename.c_str() , std::ios::binary | std::ios::trunc);
       if ( ss.good() )
       {
       //
    // Create the compressed file
          ss.seekg( 0 , std::ios::end );
          uLongf file_size = (uLongf)(ss.tellg());
          ss.seekg( 0 , std::ios::beg );
          Bytef* uncompdata = new Bytef[ file_size ];

          while ( ss ) {
              ss.read( (char*)uncompdata , file_size );
          }

          uLongf complen = compressBound(file_size);

          Bytef* compdata = new Bytef[complen];

          if ( Z_OK == compress ( compdata , &complen , uncompdata , file_size ) )
          {
            out->write((char*)compdata, (long)complen);
            if (out->fail())
            {
                cout << endl << "writing the compressed data to the output file " << compfilename << " failed" << endl
                    << " may not have permission to delete an older version of the file" << endl;
            }
          }
          else
          {
            cout << endl << "compressing the data failed" << endl;
          }

          delete [] uncompdata;
          delete [] compdata;
       }
       else
       {
            cout << endl << "### failed to write to binary file ###" << endl;
       }

       out->close(); delete out;
   }
   else
   {
// Use regular text file
    string compfilename(filename);

    if(compfilename.substr((compfilename.length()-2),2)==".z")
    {
        compfilename.pop_back();
        compfilename.pop_back();
    }

      std::ofstream out( compfilename.c_str() , std::ios::out | std::ios::trunc );
      if ( ss.good() )
      {
         ss.seekg( 0 , std::ios::end );
         int file_size = ss.tellg();
         ss.seekg( 0 , std::ios::beg );
         char* filedata = new char[ file_size ];
         while ( ss ) {
            ss.read( filedata , file_size );
            if(!file_size)
            {
                cout << "\n #### Error the size of the stringstream is invalid ###" << endl;
                break;
            }
         }
         out.write(filedata, file_size);
         if (out.fail())
        {
            cout << endl << "writing the ascii data to the output file " << compfilename << " failed" << endl
                 << " may not have permission to delete an older version of the file" << endl;
        }
         out.close();
         delete [] filedata;
      }
      else
      {
// found no data file
//                 set error bit to the stream
         ss.setstate( std::ios::badbit );

         cout << endl << "### failed to write to ascii file " << compfilename << " ###" << endl;
      }
   }
   ss.str("");
}
