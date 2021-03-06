//Copyright <Bratescu Stefan-Gabriel> 2018

#include "stdafx.h"
#include "utils.h"

#define WILDCARD "*"
#define EXTSPLT ';'
#define DOT "."
#define NOEXT "NOEXT"
#define NUM_THREADS 3
#define NOTFOUND 4294967295

using namespace std;

//Class to store info and solve task
class Solution {

private:
	string givenDirectoryPath;
	string toSearch;
	vector<string> extensions;
	string outputFile;
	ofstream outputStream;
	string workingDirectory;
	Queue<string> directories;
	mutex writeMutex;
public:
	void singleThread() {
		SearchDirectories(directories, -1);
	}
	void multiThread() {
		std::thread trds[NUM_THREADS];
		//launch
		for (int i = 0; i < NUM_THREADS; ++i) {
			trds[i] = std::thread(&Solution::SearchDirectories, this, ref(directories), i);
		}
		//join
		for (int i = 0; i < NUM_THREADS; ++i) {
			trds[i].join();

		}
	}
	Solution(char* arguments[]) {
		//get the path
		char cCurrentPath[FILENAME_MAX];
		_getcwd(cCurrentPath, sizeof(cCurrentPath));
		//make sure there is a terminator
		cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
		//update the string in the class
		workingDirectory = cCurrentPath;
		//add termination for directory
		workingDirectory += "\\";

		//Get arguments into wanted structures
		givenDirectoryPath = arguments[1];
		string stringExtensions = arguments[2];
		toSearch = arguments[3];
		outputFile = arguments[4];

		//Put the first directory (given as arg) in queue for search
		directories.push(givenDirectoryPath);

		//Get the extensions parsed
		extensions = getExtensions(stringExtensions);

		try {
			outputStream.open(outputFile);
			if (!outputStream.is_open())exit(0);
		}
		catch (exception e) {
			cout << e.what() << "\n";
			exit(0);
		}
	}
	~Solution() {
		outputStream.close();
	}
	void printParsedArguments() {
		cout << ("Path: " + givenDirectoryPath ) << "\n" ;
		cout << ("Exts: ");
		for (auto ext : extensions) {
			cout << (ext + " ");
		}
		cout << ("\n");
		cout << ("TextToSearch: " + toSearch) << "\n";
		cout << ("OutputFile: " + outputFile) << "\n";
	}
	/*Gets files from a directory*/
	vector<string> GetFiles(string &currentDirPath) {
		vector<string> files;
		WIN32_FIND_DATA findfiledata;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		string fp = string(currentDirPath + "\\*");
		std::wstring wsString = std::wstring(fp.begin(), fp.end());
		LPCWSTR sw = wsString.c_str();

		hFind = FindFirstFile(sw, &findfiledata);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if ((findfiledata.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY
					&& (findfiledata.cFileName[0] != '.'))
				{
					string nextDirPath = currentDirPath + "\\" +
						wsToString(findfiledata.cFileName);
					//replace double slashes if directory was given with slash
					replace(nextDirPath, "\\", "");
					files.push_back(nextDirPath);
				}
			} while (FindNextFile(hFind, &findfiledata) != 0);
		}
		return files;
	}
	/*Gets subdirectories from a directory*/
	vector<string> GetDirectories(string &currentDirPath) {
		vector<string> directories;
		WIN32_FIND_DATA findfiledata;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		string fp = string(currentDirPath + "\\*");
		std::wstring wsString = std::wstring(fp.begin(),fp.end());
		LPCWSTR sw = wsString.c_str();

		hFind = FindFirstFile(sw, &findfiledata);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if ((findfiledata.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY
					&& (findfiledata.cFileName[0] != '.'))
				{
					directories.push_back(currentDirPath + "\\" + 
						wsToString(findfiledata.cFileName));
				}
			} while (FindNextFile(hFind, &findfiledata) != 0);
		}
		return directories;
	}
	void SearchDirectories(Queue<string>& directories, int id) {
		cout << id << " started.\n";
		//While there are directories in the queue
		while (!directories.empty()) {
			//Get an element out of the queue
			string currentDir = directories.pop();

			//get directories in current folder
			vector<string> d = GetDirectories(currentDir);
			int numdirs = d.size();
			//push the directories in the queue
			//to be processed
			for (int i = 0; i < numdirs; i++) {
				directories.push(d[i]);
			}

			//Get the files in current directory
			vector<string> filesInCurrentDir;
			filesInCurrentDir = GetFiles(currentDir);
			int numfiles = filesInCurrentDir.size();
			//For each file..
			for (auto file : filesInCurrentDir) {
				//check if file matches and search

				// got wildcard '*' case
				if (extensions.size() == 0) {
					checkForStringInFile(file);
					break;
				}
				//get the file extension
				string currentExt = GetFileExtension(file);
				for (auto ext : extensions) {
					if (currentExt == ext) {
						checkForStringInFile(file);
						break;
					}
				}
			}
		}
		cout << id << " ended.\n";
	}

private:
	vector<string> getExtensions(string extensionsstring) {
		vector<string> extensions;
		//If wild card was used
		if (extensionsstring == WILDCARD) {
			//No items
			//Convention for wildcard extension
		}
		else {
			extensions = split(extensionsstring, EXTSPLT);
		}
		return extensions;
	}

	string GetFileExtension(string filepath) {
		//TODO: Need to check for more cases, couldn't use 
		//some function to get extension
		int indexOfLastDot = filepath.find_last_of(DOT);
		string s;
		if (indexOfLastDot != string::npos) {
			//+1 to get string without dot char
			s = filepath.substr(indexOfLastDot + 1);
		}
		else {
			s = NOEXT;
		}
		return s;
	}
	/*Write to output file if mutex is unlocked.*/
	void writeToFile(string file, string line, int lineNumber){
		writeMutex.lock();
		outputStream << file << "(" << lineNumber << "):" << line << "\n";
		writeMutex.unlock();
	}
	void checkForStringInFile(string file) {
		try
		{
			std::ifstream infile(file);
			//check for errors
			if (!infile.is_open()) {
				cout << file << "\n";
				return;
			}
			//check if output file is current file
			if (outputFile == file || file.find(outputFile) != NOTFOUND) {
				cout << file << "\n";
				return;
			}
			//starting line
			int lineNumber = 1;
			string line;

			while (std::getline(infile, line))
			{
				if (line.find(toSearch) != string::npos){
					writeToFile(file,line,lineNumber);
				}
				lineNumber++;
			}
			infile.close();
		}catch (exception e)
		{
			cout << "Exception at checkForStringInFile: " << e.what() << "\n";
		}
	}
};
int main(int argc, char *argv[]) {

	//Get the starting time
	auto start_time = std::chrono::high_resolution_clock::now();

	// Check for enough arguments 
	if (argc != 5) {
		cout << "Usage: FolderSearch <PathToSearch> <extension|extension;extension|*> <textToSearch> <outputFile>\n";
		return 0;
	}

	//Solve the task
	Solution *mainSolution = new Solution(argv);
	mainSolution->printParsedArguments();
	mainSolution->multiThread();
	delete mainSolution;

	//Get the end time
	auto end_time = std::chrono::high_resolution_clock::now();
	auto time = end_time - start_time;

	//Print execution time
	cout << "ExecutionTime: " <<
		(std::chrono::duration_cast
			<std::chrono::milliseconds>
			(time).count()) << "ms\n";
	return 0;
}
