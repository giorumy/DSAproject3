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

    //remove the temporary file
    remove(tempFile.c_str());

    return content;
}

string api::urlEncode(const string& str){
    string encoded;
    for(char c : str){
        if(c == ' '){
            encoded += "%20"; //spaces become %20
        } else {
            encoded += c;
        }
    }
    return encoded;
}

int api::searchActor(const string &name) {
    string encodedName = urlEncode(name);
    string url = base_url + "/search/person?api_key=" + api_key + "&query=" + encodedName;
    string response = fetchData(url);
    if(!response.empty()) {
        json data = json::parse(response);
        if (data.contains("results") && data["results"].is_array() && !data["results"].empty()) {
            const auto& result = data["results"][0];
            if (result.contains("id")) {
                return result["id"].get<int>();
            }
        }
    }
    cerr << "Actor was not found in database." << endl;
    return 0;
}

Actor* api::getActor(int actorID) {
    stringstream ss;
    ss << base_url << "/person/" << actorID << "?api_key=" << api_key;
    string response = fetchData(ss.str());

    if(response.empty()) {
        return nullptr;
    }

    try {
        json data = json::parse(response);

        if(data.contains("id") && data.contains("name")) {
            int id = data["id"].get<int>();
            string name = data["name"].get<string>();
            string profile_path = data.contains("profile_path") && !data["profile_path"].is_null() ?
                                 data["profile_path"].get<string>() : "";
            return new Actor(id, name, profile_path);
        }
    } catch (const exception& e) {
        cerr << "Error parsing JSON: " << e.what() << endl;
    }

    return nullptr;
}



