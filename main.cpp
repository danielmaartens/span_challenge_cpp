#include <cstdlib>
#include <iostream>
#include <regex>
#include <algorithm>
#include <map>
#include <fstream>
#include <sstream>
#include <list>
#include <utility>


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
    TeamValue(string n, int v) : name(n), value(v), rank(NULL) {}

    string GetName() {
        return name;
    }

    void SetName(string n) {
        name = n;
    }

    int GetValue() {
        return value;
    }

    void SetValue(int v) {
        value = v;
    }

    int GetRank() {
        return rank;
    }

    void SetRank(int r) {
        rank = r;
    }

};

struct MatchPointComparator {
    bool operator ()(TeamValue tv1, TeamValue tv2)
    {
        if(tv1.GetValue() == tv2.GetValue())
            return tv1.GetName() < tv2.GetName();
        return tv1.GetValue() > tv2.GetValue();

    }
};

bool booleanFromString(string s) {

    transform(s.begin(), s.end(), s.begin(), ::tolower);

    return s == "y" || s == "yes";
}

list<TeamValue> vectorToList(vector<TeamValue> vector) {
    list<TeamValue> list;

    for (TeamValue v : vector) {
        list.push_back(v);
    }

    return list;
}

void setTeamRanks(list<TeamValue>& sortedTeamValues) {

    int index = 1;
    int rank = 0;
    int previousTeamPoints = -1;

    for (TeamValue &team : sortedTeamValues) {

        int points = team.GetValue();

        if (points != previousTeamPoints) {
            rank++;
        }

        team.SetRank(rank);

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

    auto it = teamValuesMap.begin();

    while (it != teamValuesMap.end()) {
        TeamValue team = TeamValue(it->first, it->second);

        list.push_back(team);
    }

    return list;
}

vector<TeamValue> calculateMatchPoints(const vector<TeamValue> &matchResults) {

    vector<TeamValue> matchPoints;

    TeamValue teamA = matchResults[0];
    TeamValue teamB = matchResults[1];

    string teamAName = teamA.GetName();
    int teamAGoals = teamA.GetValue();
    TeamValue teamAPoints = TeamValue(teamAName, 0);

    string teamBName = teamB.GetName();
    int teamBGoals = teamB.GetValue();
    TeamValue teamBPoints = TeamValue(teamBName, 0);

    if (teamAGoals == teamBGoals) {

        teamAPoints.SetValue(1);
        teamBPoints.SetValue(1);

    } else if (teamAGoals > teamBGoals) {
        teamAPoints.SetValue(3);
    } else {
        teamBPoints.SetValue(3);
    }

    matchPoints.push_back(teamAPoints);
    matchPoints.push_back(teamBPoints);

    return matchPoints;
}

vector<TeamValue> reduceTeamMatchPoints(const vector<TeamValue> allTeamMatchPoints) {
    map<string, int> finalTeamPoints;

    for (TeamValue matchPoints : allTeamMatchPoints) {

        string name = matchPoints.GetName();
        int points = matchPoints.GetValue();

        if (finalTeamPoints.find(name) == finalTeamPoints.end()) {
            finalTeamPoints.insert(pair<std::string, int>(name, points));
        } else {
            int nextPointsTotal = finalTeamPoints.find(name)->second + points;

            finalTeamPoints.insert(pair<string, int>(name, nextPointsTotal));
        }
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
        cout << line << "\n";

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

int main() {

    string file_path = "/Users/danielmaartens/personal/span/span_challenge_cpp/input.csv";

    list<TeamValue> matchPoints = getOrderedMatchPointsFromFile(file_path);

    return 0;
}

