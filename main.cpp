#include <cstdlib>
#include <iostream>
#include <regex>
#include <algorithm>
#include <map>
#include <fstream>
#include <sstream>
#include <list>
#include <utility>
#include <unistd.h>
#include <sys/stat.h>


using namespace std;

string TEAM_RESULT_GROUPING_PATTERN = "^([a-zA-Z\\s]+)([0-9]+$)";

struct teamValue {
    string name;
    int value;
    int rank;
};

class TeamValue {
private:
    string name;
    int value;
    int rank;

public:
    TeamValue(string n, int v) : name(n), value(v), rank(0) {}

    string getName() {
        return name;
    }

    void setName(string n) {
        name = n;
    }

    int getValue() {
        return value;
    }

    void setValue(int v) {
        value = v;
    }

    int getRank() {
        return rank;
    }

    void setRank(int r) {
        rank = r;
    }

};

struct MatchPointComparator {
    bool operator()(TeamValue tv1, TeamValue tv2) {
        if (tv1.getValue() == tv2.getValue())
            return tv1.getName() < tv2.getName();
        return tv1.getValue() > tv2.getValue();

    }
};

int booleanFromString(string s) {

    transform(s.begin(), s.end(), s.begin(), ::tolower);

    if (s == "y" || s == "yes" || s == "c") {
        return 1;
    } else if (s == "n" || s == "no") {
        return 0;
    } else {
        return -1;
    }
}

list<TeamValue> vectorToList(vector<TeamValue> vector) {
    list<TeamValue> list;

    for (TeamValue v : vector) {
        list.push_back(v);
    }

    return list;
}

bool fileExists(const string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

void setTeamRanks(list<TeamValue> &sortedTeamValues) {

    int index = 1;
    int rank = 0;
    int previousTeamPoints = -1;

    for (TeamValue &team : sortedTeamValues) {

        int points = team.getValue();

        if (points != previousTeamPoints) {
            rank++;
        }

        team.setRank(rank);

        if (points == previousTeamPoints) {
            rank = index;
        }

        previousTeamPoints = points;
        index++;
    }
}

TeamValue getTeamResultFromString(const string &s, const string &pattern) {

    regex r(pattern);

    smatch m;

    // Use regex pattern to match team names that include spaces
    regex_search(s, m, r);

    string team = m[1];

    // Remove the last space from the team name
    string name = team.substr(0, team.size() - 1);

    int value = stoi(m[2]);

    return TeamValue(name, value);

}

vector<TeamValue> convertTeamValueMapToList(map<string, int> teamValuesMap) {
    vector<TeamValue> list;

    for (auto &tv : teamValuesMap) {
        TeamValue team = TeamValue(tv.first, tv.second);
        list.push_back(team);
    }

    return list;
}

vector<TeamValue> calculateMatchPoints(const vector<TeamValue> &matchResults) {

    vector<TeamValue> matchPoints;

    TeamValue teamA = matchResults[0];
    TeamValue teamB = matchResults[1];

    string teamAName = teamA.getName();
    int teamAGoals = teamA.getValue();
    TeamValue teamAPoints = TeamValue(teamAName, 0);

    string teamBName = teamB.getName();
    int teamBGoals = teamB.getValue();
    TeamValue teamBPoints = TeamValue(teamBName, 0);

    if (teamAGoals == teamBGoals) {

        teamAPoints.setValue(1);
        teamBPoints.setValue(1);

    } else if (teamAGoals > teamBGoals) {
        teamAPoints.setValue(3);
    } else {
        teamBPoints.setValue(3);
    }

    matchPoints.push_back(teamAPoints);
    matchPoints.push_back(teamBPoints);

    return matchPoints;
}

vector<TeamValue> reduceTeamMatchPoints(const vector<TeamValue> allTeamMatchPoints) {
    map<string, int> finalTeamPoints;

    for (TeamValue matchPoints : allTeamMatchPoints) {

        string name = matchPoints.getName();
        int points = matchPoints.getValue();

        finalTeamPoints[name] += points;
    }

    return convertTeamValueMapToList(finalTeamPoints);
}

vector<string> splitResultsLineIntoVector(string resultLine, const string delimiter) {
    vector<string> result;
    size_t pos = resultLine.find(delimiter);
    string token = resultLine.substr(0, pos);
    result.push_back(token);
    resultLine.erase(0, pos + delimiter.length());
    result.push_back(resultLine);

    return result;
}

list<TeamValue> getOrderedMatchPointsFromFile(const string file) {
    vector<TeamValue> allTeamMatchPoints;
    vector<TeamValue> finalTeamMatchPoints;

    ifstream infile(file);
    string line;

    while (getline(infile, line)) {

        vector<TeamValue> scores;
        vector<string> matchResults = splitResultsLineIntoVector(line, ", ");

        for (string result : matchResults) {

            TeamValue teamResult = getTeamResultFromString(result, TEAM_RESULT_GROUPING_PATTERN);
            scores.push_back(teamResult);
        }

        vector<TeamValue> matchPoints = calculateMatchPoints(scores);
        allTeamMatchPoints.insert(allTeamMatchPoints.end(), matchPoints.begin(), matchPoints.end());

    }

    finalTeamMatchPoints = reduceTeamMatchPoints(allTeamMatchPoints);

    list<TeamValue> teamMatchPointsList = vectorToList(finalTeamMatchPoints);

    teamMatchPointsList.sort(MatchPointComparator());
    setTeamRanks(teamMatchPointsList);

    return teamMatchPointsList;
}

class Print {

private:
    int initialDelay;
    int runningDelay;

public:
    explicit Print(int timeout) : initialDelay(timeout), runningDelay(timeout) {}

    void delayed(string s) {
        usleep(runningDelay);
        cout << s << '\n';
        runningDelay += initialDelay;
    }

    void ln(string s) {
        cout << s << '\n';
    }

    void reset(int n = 0) {
        runningDelay = n;
    }
};

int main() {

    bool running = true;
    string file;
    string userInput;
    int answerYes;
    Print print = Print(600000);

    print.ln("\nWelcome to the League Rank Calculator !\n");
    print.delayed("This program will calculate the ranking table for a soccer league.\n");
    print.delayed("The data for the results of the games should be stored in a text file.");

    while (running) {
        print.delayed("\nPlease provide the full path of the file where your results are stored:\n");
        print.reset(0);

        cin >> file;
        cin.get();

        if (fileExists(file)) {

            list<TeamValue> matchPoints = getOrderedMatchPointsFromFile(file);

            print.ln("\nRESULTS\n");

            for (TeamValue team : matchPoints) {
                string result = to_string(team.getRank()) + ". " + team.getName() + ", " + to_string(team.getValue()) +
                                (team.getValue() == 1 ? " pt" : " pts");
                print.ln(result);
            }

            print.ln("\nWould you like to check match point results of another league ? [y/n]: ");

            cin >> userInput;
            cin.get();

            answerYes = booleanFromString(userInput);

            while (answerYes == -1) {
                print.ln("\nI do not understand your command, please try again...");
                print.ln("Would you like to check match point results of another league ? [y/n]: ");

                cin >> userInput;
                cin.get();

                answerYes = booleanFromString(userInput);
            }

            running = answerYes;
        } else {
            print.ln("\nSorry, your file does not exist ! Please double-check your file path and try again... Press [c] to continue, or any other key (besides ENTER) to exit...\n");

            cin >> userInput;
            cin.get();

            running = booleanFromString(userInput);

            if (running == -1) {
                running = false;
            }

        }
    }

    print.ln("\nThank you for using the League Rank Calculator !");
    return 0;

}

