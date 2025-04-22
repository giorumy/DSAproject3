#include "api.h"


api::api(const string& key) : api_key(key), base_url("https://api.themoviedb.org/3") {
    apiLog.open("apiLog.txt");
    if (!apiLog.is_open()) {
        apiLog << "ERROR: Error opening api log file" << endl;
    }
}

api::~api() {
    if (apiLog.is_open()) {
        apiLog.close();
    }
}

string api::fetchData(const string &url) {

    //getting command for fetching data
    string tempFile = "temp.json";
    string command = "curl -s \"" + url + "\" > " + tempFile;

    //TODO: delete later. this is for debugging only
    apiLog << "Executing API request: " << url << endl;

    //executing command
    int result = system(command.c_str());
    if(result != 0 ) {
        apiLog << "ERROR: Failed to execute command: " << command << endl;
        apiLog << "ERROR: Error code: " << result << endl;
        return "";
    }

    //read file and add to return string
    ifstream file(tempFile);
    if(!file.is_open()) {
        apiLog << "ERROR: Failed to open temporary file: " << tempFile << endl;
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
    apiLog << "ERROR: Actor " << name << " was not found in database." << endl;
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
        apiLog << "ERROR: Error parsing JSON: " << e.what() << endl;
    }

    return nullptr;
}

vector<Actor> api::getActors(const string &movieName) {
    vector<Actor> actors;

    string encodedName = urlEncode(movieName);
    string url = base_url + "/search/movie?api_key=" + api_key + "&query=" + encodedName;

    string response = fetchData(url);
    if(response.empty()) {
        apiLog << "ERROR: Failed to fetch actors for movie: " << movieName << endl;
        return actors;
    }

    try {
        json movieData = json::parse(response);
        if(!movieData["results"].empty()) {
            int movieID = movieData["results"][0]["id"].get<int>();

            stringstream creditsURL;
            creditsURL << base_url << "/movie/" << movieID << "/credits?api_key=" << api_key;
            string creditResponse = fetchData(creditsURL.str());

            if(creditResponse.empty()) {
                apiLog << "ERROR: Failed to fetch credits for movie: " << movieName << endl;
                return actors;
            }

            json creditData = json::parse(creditResponse);
            if(creditData.contains("cast")) {
                for(const auto& actor : creditData["cast"]) {
                    int id = actor.value("id", 0);
                    string name = actor.value("name", "Unknown name");
                    string profile_path = actor.contains("profile_path") && !actor["profile_path"].is_null() ? actor["profile_path"].get<string>() : "";

                    actors.emplace_back(id, name, profile_path);
                }
            }
        } else {
            apiLog << "ERROR: Movie " <<  movieName << " not found." << endl;
        }
    } catch (const exception& e) {
        apiLog << "ERROR: Error parsing JSON: " << e.what() << endl;
    }
    return actors;
}

vector<Movie> api::getMovies(const string &actorName) {
    vector<Movie > movies;

    int actorID = searchActor(actorName);
    if(actorID <= 0) {
        apiLog << "ERROR: Actor " << actorName << "not found." << endl;
        return movies;
    }

    stringstream creditURL;
    creditURL << base_url << "/person/" << actorID << "/movie_credits?api_key=" << api_key;

    string creditResponse = fetchData(creditURL.str());
    if(creditResponse.empty()) {
        apiLog << "ERROR: Failed to fetch credits for actor: " << actorName << endl;
        return movies;
    }

    try {
        json creditData = json::parse(creditResponse);
        if(creditData.contains("cast")) {
            for(const auto& movie : creditData["cast"]) {
                int id = movie.value("id", 0);
                string title = movie.value("title", "Unknown title");
                string date = (movie.contains("release_date") &&
               !movie["release_date"].is_null() &&
               !movie["release_date"].get<string>().empty())
              ? movie["release_date"].get<string>()
              : "No release date";
                string poster_path = movie.contains("poster_path") && !movie["poster_path"].is_null() ? movie["poster_path"].get<string>() : "";

                movies.emplace_back(id, title, date, poster_path);
            }
        }
    } catch (const exception& e) {
        apiLog << "ERROR: Error parsing JSON: " << e.what() << endl;
    }

    return movies;
}


