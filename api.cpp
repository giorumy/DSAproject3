#include "api.h"

string api::fetchData(const string &url) {

    //getting command for fetching data
    string tempFile = "temp.json";
    string command = "curl -s \"" + url + "\" > " + tempFile;

    //TODO: delete later. this is for debugging only
    cout << "Executing API request: " << url << endl;

    //executing command
    int result = system(command.c_str());
    if(result != 0 ) {
        cerr << "Failed to execute command: " << command << endl;
        cerr << "Error code: " << result << endl;
        return "";
    }

    //read file and add to return string
    ifstream file(tempFile);
    if(!file.is_open()) {
        cerr << "Failed to open temporary file: " << tempFile << endl;
        return "";
    }
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    // Remove the temporary file
    remove(tempFile.c_str());

    return content;
}

string api::urlEncode(const string &str) {
    string encoded;
    for(char c : str){
        if(c == ' '){
            encoded += "%20";
        } else {
            encoded += c;
        }
    }
    return encoded;
}

