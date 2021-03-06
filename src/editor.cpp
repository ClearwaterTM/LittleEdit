// * * *			LittleEdit				* * *
// * * * 	  A LittleWood save editor 		* * * 
// * * * Created 30th October by Clearwater * * *


// A save editor for LittleWood that allows modification of variables to change amount of Dewdrops, change event flags, etc.
// **AS THE GAME IS IN EARLY ACCESS, UPDATES TO THE GAME MAY/WILL BREAK THIS SAVE EDITOR UNTIL IT IS UPDATED.


//Get the base libraries we need.
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>

//Import various libraries to interface with Windows functions (get working directory, get processes, etc)
#include <windows.h>
#include <direct.h>

#include <Lmcons.h>
#include <tlhelp32.h>
#include <io.h>

#include <sys/types.h>
#include <sys/stat.h>

//Use the standard namespace as we don't need anything from any other namespaces for now.
using namespace std;


// * * * GLOBAL VARIABLES * * *
bool detectedsaves[2];
	map<string,int> tags_with_limits;
	map<string,int>::iterator it;

//Initalize the map. More tags will be added throughout development.



//Function declarations begin here.

// ******************************************************
// ******************************************************

// ** CLASSES **

//Our class for error handling.
class ErrorCodes
{
	public:
	
	void noAccessToWorkingDir()
	{
		cout << "[error: noAccessToWorkingDir]" << endl << endl;
		cout << "Unable to access the save folder of the game." << endl;
		cout << "Possible causes: " << endl;
		cout << "Have you played the game once to generate the save folder?" << endl;
		cout << "Did you run this .exe as an admin?" << endl;
		cout << "Exiting." << endl;
		
		exit(1);
	}
	
	void noSaves()
	{
		cout << "[error: noSaves]" << endl << endl;		
		
		cout << "Unable to detect any valid saves in the folder." << endl;
		cout << "Are your saves corrupt, or are you running this editor outside of the game's save folder?" << endl;
		cout << "Have you played the game at least once to generate a save?" << endl;
		cout << "Exiting." << endl;
		exit(2);
		
	}
	
	void invalidSelection()
	{
		cout << "[error: invalidSelection]" << endl << endl;
		
		cout << "An incorrect selection was entered." << endl;
		cout << "Exiting." << endl;
		exit(3);
	}
	
	void currentFileNotOpen()
	{
		cout << "[error: currentFileNotOpen]" << endl << endl;
		
		cout << "Cannot access the selected save file." << endl;
		cout << "Possible causes: " << endl;
		cout << "- Is the file read-only?" << endl;
		cout << "- Are you running this .exe as administrator?" << endl;
		cout << "Exiting." << endl;
		exit(4);
	}
	
	void gameIsRunning()
	{
		cout << "[error: gameIsRunning]" << endl << endl;
		
		cout << "LittleEdit has detected that Littlewood is currently running." << endl;
		cout << "It cannot correctly modify the save files while this is the case." << endl;
		cout << "Please exit the game before attempting to run this editor." << endl;
		cout << "Exiting." << endl;
		exit(5);
	}
};

//Initialize the error class as a global instance straight away.
ErrorCodes Error;

// ******************************************************
// ******************************************************

// ** WINDOWS FUNCTIONS **

//Check to see if the game is open by seeing if the game process is active
bool gameIsNotClosed()
{
	PROCESSENTRY32 entry;
	string gameProcessName = "Littlewood";
	wstring wGameProcessName(gameProcessName.length(), L' ');
	
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	
	 if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (_stricmp(entry.szExeFile, "Littlewood.exe") == 0)
            {  
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

                CloseHandle(hProcess);
				return 1;
            }
        }
    }
	return 0;
}

//Get the save folder location based off of the current user's name. Usual location: C:\Users\[USERDATA]\AppData\LocalLow\SmashGames\Littlewood
string getWorkingDir()
{
	//Get the Windows username.
	char username[UNLEN+1];
	DWORD username_len = UNLEN+1;
	GetUserName(username, &username_len);
	
	string workingDir = "C\:\\Users\\";
	workingDir.append(username);
	workingDir += "\\AppData\\LocalLow\\SmashGames\\Littlewood";
	
	return workingDir;
}


// ******************************************************
// ******************************************************

// ** FILE/FOLDER FUNCTIONS **

//Check if our working directory is correct.
bool checkWorkingDir(string workingdir)
{
	
	const short length = workingdir.length();
	char* lengthchar = new char[length];

	strcpy(lengthchar, workingdir.c_str());
	
	char buff[FILENAME_MAX]; //create string buffer to hold path
	
	if(_chdir(lengthchar)) {return 1;}
	else {return 0;}

}

//Scan the working directory for any valid save files.

short scanForSaves()
{
	bool saveexists[3];
	
	ifstream save1("games0.json");
	saveexists[0] = (bool)save1;
	ifstream save2("games1.json");
	saveexists[1] = (bool)save2;
	ifstream save3("games2.json");
	saveexists[2] = (bool)save3;

	save1.close();
	save2.close();
	save3.close();
	detectedsaves[0] = saveexists[0];
	detectedsaves[1] = saveexists[1];
	detectedsaves[2] = saveexists[2];
	return (saveexists[0] + saveexists[1] + saveexists[2]);
}

//Check to see if an existing save is valid.
short validateSaves()
{
	//For this, run a simple check to see if the first string in the save file is equal to the following: {"title"
	//Returns a sum between: 0, 1, 4, 7.
	//0 - No valid saves, 1 - save #1 , 4 - save #2, 7 - save #3, 5 = saves #1&2, 11 - saves #2&#3, 8 - saves #1&#3, 12 - 3 saves
	
	//TODO: Factor this function to cut down on lines
	
		short validatedsaves = 0;
		string validstring = "{\"title\":";
		string teststring;
		char extract1, extract2, extract3;
	
	if (detectedsaves[0]){
		ifstream save1("games0.json");
		for (int a=0;a<9;a++)
		{
			save1.get(extract1);
			teststring += extract1;
		}
		if (validstring == teststring)
		{
			validatedsaves += 1;
			save1.close();
		}
		else 
		{
		cout << "Save #1 is not valid." << endl;
		save1.close();
		}
		teststring.clear();
	}
	
	if (detectedsaves[1]){
		ifstream save2("games1.json");
		for (int b=0;b<9;b++)
		{
			save2.get(extract2);
			teststring += extract2;
		}
		if (validstring == teststring)
		{	
			validatedsaves += 4;
			save2.close();
		}
		else 
		{
		cout << "Save #2 is not valid." << endl;
		save2.close();
		}
		teststring.clear();
	}
	
	if (detectedsaves[2]){
		ifstream save3("games2.json");
		for (int c=0;c<9;c++)
		{
			save3.get(extract3);
			teststring += extract3;
		}
		if (validstring == teststring)
		{
			validatedsaves += 7;
			save3.close();
		}
		else 
		{
		cout << "Save #3 is not valid." << endl;
		save3.close();
		}
		teststring.clear();
	}
	
	return validatedsaves;
	
}

//Find the requested tag in the file, and return the starting byte of its data.
int seekStartingByte(fstream &inputFile, string tagToFind)
{
	
	short lengthOfTag = tagToFind.length();
	int shortByteLocation;
	int currentByteLocation = 0;
	
	char extractedChar;
	string extractedString;
	
	//Reset stream position to 0 just in case.
	inputFile.seekg(0,ios::beg);
		
	while(inputFile.get(extractedChar))
	{
		currentByteLocation++;
		//Make sure we can break if EOF is reached and we still haven't found the tag.
		if (inputFile.eof())
		{
			cout << "Unable to locate tag. Aborting." << endl;
			exit(6);

		}
		extractedString += extractedChar;
		if (extractedString.length() > lengthOfTag)
		{
			extractedString.erase(0,1);
		}
		if (extractedString == tagToFind)
		{
			return currentByteLocation+1;
		}
	}
	return 0;
}


//Write the new data into the new save file.
template <typename T>
int saveContent(fstream &inputFile, string nameOfOldFile, string nameOfTempFile, int startByte, T data, T dataToReplace)
{
	
	//Convert the string of the old file to a char array for renaming it later.
	
	char* tempFileNameChar = new char[nameOfTempFile.size()+1];
	strcpy(tempFileNameChar, nameOfTempFile.c_str());
	
	char* oldFileNameChar = new char[nameOfOldFile.size()+1];
	strcpy(oldFileNameChar, nameOfOldFile.c_str());
	
	//Create the temp file to write changes to.
	fstream newFile;
	newFile.open(nameOfTempFile,ios::app);
	
	//Set the original file stream back to the beginning.
	inputFile.seekg(0,inputFile.beg);
	
	//Write original file until the startByte is hit, at which point we write modifications until endByte is hit.
	char currentByte;
	int currentByteLocation = 0;
	bool alreadyWritten = 0;
	
	cout << "Writing data, please wait..." << endl;
	
	while(inputFile.get(currentByte))
	{
		currentByteLocation++;

		
	if((currentByteLocation >= startByte) && !(alreadyWritten))
	{
		newFile << data;

		//Determine the offset of the stream seek.
		
		//First, convert both data to strings.
		string oldDataString = (string)dataToReplace;
		string newDataString = (string)data;
		
		short offset = 1;
		
		//Default offset is -1. If new data is smaller than existing data, increase offset by difference in length between data.
		//No offset if the length of the old data is the same as the new data
		
		inputFile.seekg(data.length()-(offset + (newDataString.length()-2)-oldDataString.length()),ios::cur);
		alreadyWritten = 1;
	}
	else
	{
		newFile << currentByte;
		if (alreadyWritten)
		{
		}
	}
	}
	cout << "Finished writing to file." << endl;
	
	//Close the file streams as we're done with them
	inputFile.close();
	newFile.close();
	
	//Lastly, remove the original file and rename the new file.
	
	remove(oldFileNameChar);
	rename(tempFileNameChar,oldFileNameChar);
	
	cout << endl;
	cout << "Done! If you would like to make any further changes, simply run this .exe again." << endl;
	cout << "DISCLAIMER: THIS PROGRAM IS STILL IN DEVELOPMENT AND IS SUBJECT TO MAJOR CHANGES." << endl;
	
	return 0;
}


void overwritePromptString(fstream &inputFile, string nameOfOldFile, string nameOfTempFile, string tag)
{
	string tagToFind = tag;
	string currentDataString, newDataString;
	char extractedByte = '\0';
	
	//Find the starting position of the data to be overwritten.
	int startPosition = seekStartingByte(inputFile, tagToFind);
	
	//Set the stream position to the start pos.
	inputFile.seekg(startPosition);
	
	while(extractedByte != '\"')
	{
		inputFile.get(extractedByte);
		currentDataString += extractedByte;
	}
	currentDataString.pop_back();
	
	cout << tag;
	
	cout << "The current " << tag << " value is: " << currentDataString << endl;
	cout << "Enter the new " << tag << " : " << endl;
	
	cin >> newDataString;
	
	newDataString.insert(0,"\"");
	newDataString.append("\"");
	
	saveContent(inputFile, nameOfOldFile, nameOfTempFile, startPosition, newDataString, currentDataString);
}


short getInputLimit(string tag)
{
	//Create the maps that will hold tags with limits, and their respective max limits.
	
	tags_with_limits["\"skin\"\:"] = 3;
	tags_with_limits["\"hair\"\:"] = 11;
	tags_with_limits["\"color\"\:"] = 8;
	tags_with_limits["\"outfit\"\:"] = 12;
	tags_with_limits["\"type\"\:"] = 8;
	
	tags_with_limits["\"playerName\"\:"] = 10;
	
	it = tags_with_limits.find(tag);
	
	if (it == tags_with_limits.end())
	{
		cout << "No limit for tag found, proceeding as normal." << endl;
		return -1;
	}
	else
	{
		cout << "Tag with limit found, enforcing limit on input. Limit: " << tags_with_limits[tag] << endl;
		return tags_with_limits[tag];
	}
}

//Checks to see if a tag has a list of contents we can choose from.
bool checkTagPrompt(string tagToCheck)
{
	if (tagToCheck == "\"skin\"\:")
	{
		cout << "Select an option from the list to apply:" << endl << endl;
		cout << "1: Ivory" << endl;
		cout << "2: Honey" << endl;
		cout << "3: Chestnut" << endl;
		return 1;
	}
	else if (tagToCheck == "\"hair\"\:")
	{
		cout << "Select an option from the list to apply:" << endl << endl;
		cout << "1: Short" << endl;
		cout << "2: Long" << endl;
		cout << "3: Hero" << endl;
		cout << "4: Wavy" << endl;
		cout << "5: Fuzz" << endl;
		cout << "6: Ponytail" << endl;
		cout << "7: Smooth" << endl;
		cout << "8: Spiky" << endl;
		cout << "9: Column" << endl;
		cout << "10: Rolls" << endl;
		cout << "11: Hawk" << endl;
		return 1;
	}
	else if (tagToCheck == "\"color\"\:")
	{
		cout << "Select an option from the list to apply:" << endl << endl;
		cout << "1: Cedar" << endl;
		cout << "2: Onyx" << endl;
		cout << "3: Lemon" << endl;
		cout << "4: Ruby" << endl;
		cout << "5: Ocean" << endl;
		cout << "6: Emerald" << endl;
		cout << "7: Bubblegum" << endl;
		cout << "8: Pearl" << endl;
		return 1;
	}
	else if (tagToCheck == "\"outfit\"\:")
	{
		cout << "Select an option from the list to apply:" << endl << endl;
		cout << "1: Overalls" << endl;
		cout << "2: Farmhand" << endl;
		cout << "3: Wizard" << endl;
		cout << "4: Grayscale" << endl;
		cout << "5: Lilac" << endl;
		cout << "6: Adventurer" << endl;
		cout << "7: Peasant" << endl;
		cout << "8: Mustard" << endl;
		cout << "9: Neon" << endl;
		cout << "10: Cream Robe" << endl;
		cout << "11: Blush Robe" << endl;
		cout << "12: Sky Robe" << endl;
		return 1;
	}
	else if (tagToCheck == "\"type\"\:")
	{
		cout << "Select an option from the list to apply:" << endl << endl;
		cout << "1: Curious" << endl;
		cout << "2: Kind" << endl;
		cout << "3: Naughty" << endl;
		cout << "4: Creative" << endl;
		cout << "5: Friendly" << endl;
		cout << "6: Sassy" << endl;
		cout << "7: Pirate" << endl;
		cout << "8: Bookworm" << endl;
		return 1;
	}
	else {return 0;}
}

void overwritePromptInt(fstream &inputFile, string nameOfOldFile, string nameOfTempFile, string tag)
{
	//Some tags have a max limit to their size. During input we need to enforce a limit as to how big input can be.
	bool menuPromptFlag = 1;
	
	string tagToFind = tag;
	string currentDataString, newDataString;
	char extractedByte = '\0';
	int startPosition = seekStartingByte(inputFile, tagToFind)-1;
	short inputLimit = getInputLimit(tag);
	
	inputFile.seekg(startPosition);
	
	while(extractedByte != ',')
	{
		inputFile.get(extractedByte);
		currentDataString += extractedByte;
	}
	
	int currentDataInt = stoi(currentDataString);
	

	//Check if the selected tag has a specific menu to choose from.
	if (!(checkTagPrompt(tag)))
	{
		cout << "The current " << tag << " value is: " << currentDataInt << endl;
		cout << "Enter the new " << tag << " value: " << endl;
		menuPromptFlag = 0;
	}
	
	cin >> newDataString;
	
	if (inputLimit != -1)
	{
		if (stoi(newDataString) > inputLimit )
		{
			cout << "Over limit!" << endl;
		}
	}
	
	if (menuPromptFlag)
	{
		stringstream strstream;
		short toConvert = stoi(newDataString);
		toConvert--;
		strstream << toConvert;
		newDataString = strstream.str();
		
	}

	newDataString.append("\,\"");
	newDataString.insert(0, "\:");

	saveContent(inputFile, nameOfOldFile, nameOfTempFile, startPosition, newDataString, currentDataString);

}



void overwritePromptBoolArray(fstream& inputFile, string nameOfOldFile, string nameOfTempFile, string tag)
{
	// ELEMENTS ARE SEPERATED BY COMMAS INSIDE AN ARRAY


	// First thing we need to do is get the array as a string.

	string tagToFind = tag;
	string currentDataBool, newDataBool;
	char extractedByte = '\0';
	int startPosition = seekStartingByte(inputFile, tagToFind) - 1;

	inputFile.seekg(startPosition);
	cout << "Starting byte: " << startPosition << endl;
	system("pause");

	while (extractedByte != ']')
	{
		inputFile.get(extractedByte);
		currentDataBool += extractedByte;
	}

	cout << "Raw array data: " << currentDataBool << endl;
	string tempDataBool = currentDataBool;

	//Next, get the amount of elements from it.
	//While we're at it, convert the elements in the table from string (true, false) to number (1,0).

	int amountOfElements = 0;
	short byteToStart;

	for (short a = 0; a < currentDataBool.length(); a++)
	{
		if (currentDataBool.find("false", a) < currentDataBool.length())
		{
			byteToStart = currentDataBool.find("false", a);
			currentDataBool.replace(byteToStart, 5, "0");
			amountOfElements++;
		}
	}

	for (short a = 0; a < currentDataBool.length(); a++)
	{
		if (currentDataBool.find("true", a) < currentDataBool.length())
		{
			byteToStart = currentDataBool.find("false", a);
			currentDataBool.replace(a, 4, "1");
			amountOfElements++;
		}
	}
	cout << "Amount of elements inside array: " << amountOfElements << endl;
	cout << "Array after modification: " << currentDataBool << endl;

	//Now that we have the converted bool array, lookup the tag to find out what we're working with.
	//This table will be updated as we add more tags to work with.

	map<string, short> tagTable;
	tagTable["\"whirlybugUsedToday\"\:"] = 1;
	tagTable["\"cavernExplored\"\:"] = 2;

	cout << tag << endl;
	cout << tagTable.find("test")->second << endl;

	//Store the keyvalue from the map in a variable.
	short keyValue = tagTable.find(tag)->second;

	//Prepare limit variables to be resized depending on the found tag.

	vector<bool> dataArray;
	dataArray.resize(amountOfElements);

	switch (keyValue)
	{
		// If the keyValue is 0 then the key wasn't found, so something went wrong.
		// Will make this into a thrown error later.
	case 0: { cout << "Key not found. Something must have gone wrong." << endl; break; }
	case 1: { cout << "whirlybugUsedToday" << endl; break; }
	case 2: { cout << "cavernExplored" << endl; break; }

	default: {break; }

	}


	for (short a = 1; a < currentDataBool.size(); a += 2)
	{
		short c = 0;
		bool toAdd = 0;
		cout << (short)currentDataBool[a] << endl;

		if ((short)currentDataBool[a] == 48) {toAdd = 0;}
		else if ((short)currentDataBool[a] == 49) {toAdd = 1;}

		dataArray[c] = toAdd;

	}

	cout << "[";
	for (short b = 0; b < dataArray.size(); b++)
	{
		cout << dataArray[b] << " ";
	}
	cout << "]";

	// User modification of the bool arrray starts here.
	// Set this to be a loop - while the user hasn't pressed a specific button to exit and confirm changes, the modification prompt continues.
	// Exit key: 0 (zero)

	while (true)
	{
		system("cls");
		cout << "Currently modifying tag - " << tag << endl;
		cout << "Tag size - " << amountOfElements << endl;

		cout << "Current position values: " << endl;
		for (short a = 0; a < amountOfElements; a++)
		{
			cout << (a+1) << " : " << dataArray[a] << endl;
			
		}

		short selection;

		cout << "Select a position to modify, or enter 0 to finish modifying and save the array." << endl;
		cin >> selection;

		if (selection > amountOfElements) { cout << "Selection is over tag size, ignoring selection."; }
		else if (selection == 0) {break; }
		else
		{
			cout << "Selected position: " << selection << endl;
			cout << "Select a value to set (0: False, 1: True)" << endl;
			bool newValue;
			cin >> newValue;
			if (newValue > 1)
			{
				cout << "Invalid value, cancelling modification." << endl;
				system("pause");
			}
			else
			{
				dataArray[selection - 1] = newValue;
			}

		}
	}
	//Once the modifications have been made and the interface has been quit, convert the dataArray back into a string and save it into the file.

	newDataBool = "";

	//Convert the bools to strings...
	for (short a = 0; a < amountOfElements; a++)
	{
		if (dataArray[a] == 0)
		{
			newDataBool.append("false");
		}
		else if (dataArray[a] == 1)
		{
			newDataBool.append("true");
		}

		if (a != amountOfElements-1)
		{
			cout << a << endl;
			newDataBool.append(",");
		}
	}

	// ...and pop on brackets at the start and end, and voil� we have our array ready to save.

	newDataBool.insert(0, ":[");
	newDataBool.append("],");

	//Save the modified array into the file.
	saveContent(inputFile, nameOfOldFile, nameOfTempFile,startPosition,newDataBool,tempDataBool);
}

void overwritePromptBool(fstream &inputFile, string nameOfOldFile, string nameOfTempFile, string tag)
{
	string tagToFind = tag;
	string currentDataBool, newDataBool;
	char extractedByte;
	int startPosition = seekStartingByte(inputFile, tagToFind)-1;
	
	inputFile.seekg(startPosition);
	
	while(extractedByte != ',')
	{
		inputFile.get(extractedByte);
		currentDataBool += extractedByte;
	}
	
	cout << "The current " << tag << " value is: " << currentDataBool << endl;
	cout << "Enter the new " << tag << " value (true or false): " << endl;
	
	cin >> newDataBool;
	
	newDataBool.append("\,\"");
	newDataBool.insert(0,"\:");
	
	saveContent(inputFile, nameOfOldFile, nameOfTempFile, startPosition, newDataBool, currentDataBool);
}

void backupFiles(string nameOfOldFile)
{

	struct stat info;
	
	//Generate the folder to place our backup files if it doesn't exist already.
	/*Note: NOT USEFUL RIGHT NOW until I can figure out how to create/move files in a subfolder
	
	if (stat("Backups", &info) != 0 )
	{
		cout << "Backup folder does not exist, creating." << endl;
		if (mkdir("./backups") != 0)
		{
			cout << "Failed to create backup folder. Proceed with caution, and make manual backups." << endl;
		}
		else
		{
			cout << "Backup folder created." << endl;
		}
	}
	*/
	
	cout << "Creating backup copies of files..." << endl;
	
	//Open a stream to the original save file and its backup, make empty files of the same name+backup in the backups folder and copy them over.
	
	//Create the strings for the files to backup.
	string fileToCopy = nameOfOldFile;
	string backupfileToCopy = nameOfOldFile;
	
	//One stream for the original file...
	fstream originalFileStream(nameOfOldFile);
	fileToCopy.insert(fileToCopy.size(),".backup");
	
	fstream originalFileStreamBackup(fileToCopy, ios::out | ios::trunc);
	
	//...and one for the game-generated backup.
	fstream originalBackupStream(backupfileToCopy);
	backupfileToCopy.insert(6,"BACKUP");
	backupfileToCopy.insert(backupfileToCopy.size(),".backup");
	fstream originalBackupStreamBackup(backupfileToCopy, ios::out | ios::trunc);


	//Copy the original file contents into the backup files.
	char currentByte;
	
	while(originalFileStream.get(currentByte))
	{
		originalFileStreamBackup << currentByte;
	}
	originalFileStream.close();
	originalFileStreamBackup.close();
	
	while(originalBackupStream.get(currentByte))
	{
		originalBackupStreamBackup << currentByte;
	}
	originalBackupStream.close();
	originalBackupStreamBackup.close();
	
	//Move the backup files to the backup folder.
	/* SEE ABOVE
	
	string saveOriginalLocation = getWorkingDir();
	string saveBackupLocation = getWorkingDir();
	
	saveOriginalLocation += "\\";
	saveBackupLocation += "\\backups";
	
	char saveOriginalLocationChar[saveOriginalLocation.size() + 1];
	char saveBackupLocationChar[saveBackupLocation.size() + 1];
	
	strcpy(saveOriginalLocationChar, saveOriginalLocation.c_str());
	strcpy(saveBackupLocationChar, saveBackupLocation.c_str());
	
	cout << saveOriginalLocationChar << endl << saveBackupLocationChar << endl;
	
	if (!rename(saveOriginalLocationChar, saveBackupLocationChar))
	{
		cout << "Failed to move backup files." << endl;
	}
	else
	{
		cout << "Files moved" << endl;
	}
	*/
}

void deleteGameBackupFile(string nameOfOldFile)
{
	//Remove the game-generated backup file.
	string nameOfBackupFile = nameOfOldFile;
	nameOfBackupFile.insert(6,"BACKUP");
	
	const short length = nameOfBackupFile.length() + 1;
	char* nameOfBackupFileChar = new char[length];

	strcpy(nameOfBackupFileChar, nameOfBackupFile.c_str());

	cout << endl << "Deleting game-generated backup file..." << endl;
	if (!remove(nameOfBackupFileChar))
	{
		cout << "File deleted successfully." << endl;
	}
	else
	{
		cout << "Failed to delete game-generated backup file." << endl;
		cout << "It's possible the game has not yet created a backup file, in which case you can disregard this warning." << endl;
		cout << "If the backup file exists, check to see if LittleEdit was launched as an administrator." << endl;
		cout << "If not, proceed with caution as any attempted changes may not succeed." << endl << endl;
		system("pause");
	}
	system("cls");
}

void saveModifyTownMenu(fstream &inputFile, string nameOfOldFile, string nameOfTempFile)
{
	//Town stuff
	short selection;
	
	cout << "1: Rename town title" << endl;
	cout << "2: Rename town" << endl;
	cout << "3: Set town beauty level" << endl;
	cout << "4: Change Tavern current recipe hint" << endl;
	cout << "5: Set status of Well chest" << endl;
	cout << "6: Set maximum length of day" << endl;
	cout << "7: Set current period of day" << endl;
	

	cout << endl;
	
	cin >> selection;
	
	switch(selection)
	{
		case 1: {overwritePromptString(inputFile, nameOfOldFile, nameOfTempFile,"\"title\"\:"); break;}
		case 2: {overwritePromptString(inputFile, nameOfOldFile, nameOfTempFile, "\"townName\"\:");break;}
		case 3: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile,"\"townBeautyLvl\"\:"); break;}
		case 4: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"recipeHintID\"\:");break;}
		case 5: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile,"\"openedWellChest\"\:"); break;}
		case 6: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"maxDayEXP\"\:");break;}
		case 7: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"dayEXP\"\:");break;}
		
		default: break;
	}
}

void saveModifyCharMenu(fstream &inputFile, string nameOfOldFile, string nameOfTempFile)
{
	short selection;
	
	//Character stuff
	cout << "1: Rename character" << endl;
	cout << "2: Player housing wall" << endl;
	cout << "3: Player housing roof" << endl;
	cout << "4: Set player skin" << endl;   //Skin has max of 2, any further messes up the character sprite
	cout << "5: Set player hair" << endl;   //Hair has max of 6
	cout << "6: Set player color" << endl;  //Color has max of 6
	cout << "7: Set player outfit" << endl; //Outfit has max of 6
	cout << "8: Set player style" << endl;  //Style has max of 7
	
	cin >> selection;
	
	switch(selection)
	{
		case 1: {overwritePromptString(inputFile, nameOfOldFile, nameOfTempFile,"\"playerName\"\:"); break;}
		case 2: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile,"\"playerHouseWall\"\:"); break;}
		case 3: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile,"\"playerHouseRoof\"\:"); break;}
		case 4: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile,"\"skin\"\:"); break;}
		case 5: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile,"\"hair\"\:"); break;}
		case 6: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile,"\"color\"\:"); break;}
		case 7: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile,"\"outfit\"\:"); break;}
		case 8: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile,"\"type\"\:"); break;}
		
		default: break;
	}
}

void saveModifyInventoryMenu(fstream &inputFile, string nameOfOldFile, string nameOfTempFile)
{
	short selection;
	
	//Inventory stuff
	cout << "1: Modify Dewdrops" << endl;
	
	cin >> selection;
	
	switch(selection)
	{
		case 1: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"dew\"\:");break;}
		default: break;
	}
}

void saveModifyStatsMenu(fstream &inputFile, string nameOfOldFile, string nameOfTempFile)
{
	short selection;
	
	//Stats stuff
	cout << "1: Set amount of days played" << endl;
	cout << "2: Modify amount of steps taken" << endl;
	cout << "3: Set amount of items gathered" << endl;
	cout << "4: Set amount of ore mined" << endl;
	cout << "5: Set amount of trees chopped" << endl;
	cout << "6: Set amount of bugs caught" << endl;
	cout << "7: Set amount of fish caught" << endl;
	cout << "8: Set amount of crops harvested" << endl;
	cout << "9: Set amount of items crafted" << endl;
	cout << "10: Set amount of items donated to museum" << endl;
	cout << "11: Set amount of town wishes made" << endl;
	cout << "12: Set amount of items sold" << endl;
	cout << "13: Set amount of Dewdrops earned" << endl;
	cout << "14: Set amount of structure donations" << endl;
	cout << "15: Set amount of events experienced" << endl;
	
	cin >> selection;

	switch(selection)
	{
		case 1: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"daysPlayed\"\:");break;}
		case 2: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"steps\"\:");break;}
		case 3: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"itemsGathered\"\:");break;}
		case 4: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"oresMined\"\:");break;}
		case 5: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"treesChopped\"\:");break;}
		case 6: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"bugsCaught\"\:");break;}
		case 7: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"fishCaught\"\:");break;}
		case 8: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"cropsHarvested\"\:");break;}
		case 9: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"itemsCrafted\"\:");break;}
		case 10: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"museumDonations\"\:");break;}
		case 11: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"townWishesMade\"\:");break;}
		case 12: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"itemsSold\"\:");break;}
		case 13: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"dewdropsEarned\"\:");break;}
		case 14: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"structureDonations\"\:");break;}
		case 15: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"eventsExperienced\"\:");break;}
		
		default: break;
	}
}

void saveModifyMiscMenu(fstream &inputFile, string nameOfOldFile, string nameOfTempFile)
{
	short selection;
	
	//Misc stuff
	
	cout << "1: Set current day" << endl;
	cout << "2: Set current season" << endl;
	cout << "3: Set current year" << endl;

	cin >> selection;
	
	switch(selection)
	{
		case 1: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"day\"\:");break;}
		case 2: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"season\"\:");break;}
		case 3: {overwritePromptInt(inputFile, nameOfOldFile, nameOfTempFile, "\"year\"\:");break;}
		default: break;
	}
}

void saveModifyMainMenu(fstream &inputFile, string nameOfOldFile, string nameOfTempFile)
{
	//Backup the original file before editing when it is opened, so it can be manually reverted should this save editor mess up somehow.
	backupFiles(nameOfOldFile);
	
	//Remove the backup file generated by the game when a save is loaded in-game.
	deleteGameBackupFile(nameOfOldFile);
	
	short selection;
	fstream originalFileDumpStream(nameOfTempFile);	//The .tmp file
	
	cout << "Select a category: " << endl;
	
	cout << "1: Town data" << endl;
	cout << "2: Character data" << endl;
	cout << "3: Inventory data" << endl;
	cout << "4: Stats data" << endl;
	cout << "5: Misc. data" << endl;
	//cout << "6: Testing function for bool array (test with whirlybugUsedToday tag)" << endl;
	cout << endl;
	
	cin >> selection;
	
	cout << "Select an option: " << endl;
	switch(selection)
	{
		case 1: {saveModifyTownMenu(inputFile, nameOfOldFile, nameOfTempFile); break;}
		case 2: {saveModifyCharMenu(inputFile, nameOfOldFile, nameOfTempFile); break;}
		case 3: {saveModifyInventoryMenu(inputFile, nameOfOldFile, nameOfTempFile);break;}
		case 4: {saveModifyStatsMenu(inputFile, nameOfOldFile, nameOfTempFile); break;}
		case 5: {saveModifyMiscMenu(inputFile, nameOfOldFile, nameOfTempFile); break;}
		case 6: {overwritePromptBoolArray(inputFile, nameOfOldFile, nameOfTempFile, "\"whirlybugUsedToday\"\:"); break; }
		default: break;
	}
}
// ******************************************************
// ******************************************************

// ** OTHER FUNCTIONS **

//Print the logo.
void printLogo()
{
	cout << "- - - - - - - - - - - - - - -" << endl;
	cout << "-        LittleEdit         -" << endl;
	cout << "- A Littlewood save editor  -" << endl;
	cout << "- - - - - - - - - - - - - - -" << endl;
	cout << endl;

}

//Prompt the user to choose a save.
//0 - No valid saves, 1 - save #1 , 4 - save #2, 7 - save #3, 5 = saves #1&2, 11 - saves #2&#3, 8 - saves #1&#3, 12 - 3 saves
void chooseSave(short detectedSaves)
{
	string file1 = "File 1 (games0.json)";
	string file2 = "File 2 (games1.json)";
	string file3 = "File 3 (games2.json)";
	
	string oldFileName, tempFileName;
	
	fstream openFileStream, originalFileStream;
	
	short verify = detectedSaves;
	short choice;
	
	switch (detectedSaves)
	{
		case 1: case 4: case 7:  {cout << "1 save detected." << endl;break;}
		case 5: case 8: case 11: {cout << "2 saves detected." << endl;break;}
		case 12:				 {cout << "3 saves detected." << endl;break;}
		
	default: {cout << "Hmm, shouldn't be seeing this..." << endl; break;}
	}
	
	cout << "Select a save to modify: (1-3)" << endl;
	
	// Print the available saves depending on what ones were detected.
	switch(detectedSaves)
	{
		// Saves 1 OR 2 OR 3
		case 1: {cout << file1 << endl; break;}
		case 4: {cout << file2 << endl; break;}
		case 7: {cout << file3 << endl; break;}
		
		//Saves 1 AND 2
		case 5:{cout << file1 << endl << file2 << endl; break;}
		
		//Saves 2 AND 3
		case 11:{cout << file2 << endl << file3 << endl; break;}
		
		//Saves 1 AND 3
		case 8:{cout << file1 << endl << file3 << endl; break;}
		
		//ALL 3 saves
		case 12:{cout << file1 << endl << file2 << endl << file3 << endl;break;}
		
		default: {break;}
	}

	cin >> choice;
	
	//Check against the detected saves to make sure the selected save exists.
	switch(choice)
	{
		//TODO: Verify that the streams were opened correctly, crash if not.
		case 1:
		{
			if ((verify != 1) && (verify != 5) && (verify != 8) && (verify != 12))
			{
				Error.invalidSelection();
				system("pause");
			}
			else
			{
				openFileStream.open("games0.json");oldFileName = "games0.json";tempFileName = "games0.tmp";
				break;
				}
			}
		case 2: 
		{
			if ((verify != 4) && (verify != 5) && (verify != 11) && (verify != 12)) 
			{
				Error.invalidSelection();
				system("pause");
			}
			else
			{
				openFileStream.open("games1.json");oldFileName = "games1.json";tempFileName = "games1.tmp";
				break;
			}
		}

		case 3:
		{
			if ((verify != 7) && (verify != 8) && (verify != 11) && (verify != 12))
			{
				Error.invalidSelection();
				system("pause");
			}
			openFileStream.open("games2.json");oldFileName = "games2.json";tempFileName = "games2.tmp";
			break;
		}
		
		default: {Error.invalidSelection(); system("pause");}
	}
	
	//Make sure a stream to the selected file is open. If not, throw an error.
	if (!openFileStream.is_open()) {Error.currentFileNotOpen();}
	
	//When a file has been chosen, open the main menu.
	system("cls");
	saveModifyMainMenu(openFileStream, oldFileName, tempFileName);

}

void Initialize()
{
	//Print the program logo.
	printLogo();
	
	//Check to see if we're in the correct working directory.
	if (checkWorkingDir(getWorkingDir())) {Error.noAccessToWorkingDir();}
	
	//Make sure the game is closed, we can't modify the files correctly if it is.
	if (gameIsNotClosed()) {Error.gameIsRunning();}
	
	//Make sure there are saves available in the working directory.
	if (scanForSaves() == 0 || validateSaves() == 0){Error.noSaves();}
	chooseSave(validateSaves());
}


// ** MAIN ENTRY POINT **
int main()
{
	system("cls");
	Initialize();
	return 0;
}