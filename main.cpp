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

static string TEAM_RESULT_GROUPING_PATTERN = "^([a-zA-Z\\s]+)([0-9]+$)";

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

/**
 * This is a struct with a single operator function.
 * it is a Comparator that serves as a sorting tool for our data later,
 * where it would "compare" our TeamValue data first by value and if that is equal then by name.
 */
struct MatchPointComparator {
    bool operator()(TeamValue tv1, TeamValue tv2) {
        if (tv1.getValue() == tv2.getValue())
            return tv1.getName() < tv2.getName();
        return tv1.getValue() > tv2.getValue();

    }
};

/**
 * Converts a vector to a list.
 * This is useful because lists can be easily sorted through
 * by using Comparator classes.
 *
 * @param vector
 * @return
 */
list <TeamValue> vectorToList(vector <TeamValue> vector) {
    list <TeamValue> list;

    for (TeamValue v : vector) {
        list.push_back(v);
    }

    return list;
}

/**
 * Converts expected user input for yes/no/continue questions into a boolean value.
 * @param s
 * @return
 */
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

/**
 * Checks if the file inputted by the user is valid/exists
 * @param name
 * @return
 */
bool fileExists(const string &name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

/**
 * Sets the rank value for all teams.
 * Note: the list must be sorted.
 * @param sortedTeamValues
 */
void setTeamRanks(list <TeamValue> &sortedTeamValues) {

    int index = 1;
    int rank;
    int previousTeamPoints;

    for (TeamValue &team : sortedTeamValues) {

        int points = team.getValue();

        // Only change rank to running index if current points and previous points are different
        // This is to make sure that teams who have the same points have the same rank.
        if (points != previousTeamPoints) {
            rank = index;
        }

        team.setRank(rank);

        // Set previous points to current points for next iteration check.
        previousTeamPoints = points;
        index++;
    }

}

/**
 * Expects a string containing the name of the team followed by a space and then the team's score for that match.
 * E.g. team "GoGetters" with score 10 should have a string as follows: "GoGetters 10"
 *
 * It will then convert this string into a TeamValue object that has a name and value variable.
 * It should also convert the string score into a number.
 *
 * @param s
 * @param pattern
 * @return
 */
TeamValue getTeamResultFromString(const string &s, const string &pattern) {

    regex r(pattern);

    smatch m;

    // Use regex pattern to match team names that include spaces
    regex_search(s, m, r);

    string team = m[1];

    // Remove the space at the end of the team name
    string name = team.substr(0, team.size() - 1);

    // Convert string value into int type.
    int value = stoi(m[2]);

    // return a TeamValue class object
    return TeamValue(name, value);
}

/**
 * Convert a map object to a list for easier processing of data later.
 * @param teamValuesMap
 * @return
 */
vector <TeamValue> convertTeamValueMapToList(map<string, int> teamValuesMap) {
    vector <TeamValue> list;

    for (auto &tv : teamValuesMap) {
        TeamValue team = TeamValue(tv.first, tv.second);
        list.push_back(team);
    }

    return list;
}

/**
 * Processes a vector of the two team scores in a single match
 * and returns a new TeamValue object for each team where the value parameter
 * represents the points the team received from either Losing/Winning/Drawing the match.
 *
 * @param matchResults
 * @return
 */
vector <TeamValue> calculateMatchPoints(const vector <TeamValue> &matchResults) {

    vector <TeamValue> matchPoints;

    // Initialise new TeamValue objects for each team
    // setting initial points to 0

    TeamValue teamA = matchResults[0];
    TeamValue teamB = matchResults[1];

    string teamAName = teamA.getName();
    int teamAGoals = teamA.getValue();
    TeamValue teamAPoints = TeamValue(teamAName, 0);

    string teamBName = teamB.getName();
    int teamBGoals = teamB.getValue();
    TeamValue teamBPoints = TeamValue(teamBName, 0);

    // Match is a DRAW
    if (teamAGoals == teamBGoals) {

        teamAPoints.setValue(1);
        teamBPoints.setValue(1);

        // Team A WON
    } else if (teamAGoals > teamBGoals) {
        teamAPoints.setValue(3);

        // Team B WON
    } else {
        teamBPoints.setValue(3);
    }

    // Add the new objects to an empty vector
    matchPoints.push_back(teamAPoints);
    matchPoints.push_back(teamBPoints);

    return matchPoints;
}

/**
 * When this function is called we have a vector
 * containing each team's match points for all games played.
 *
 * We want to reduced that vector to one that only has
 * one entry for each team, with each new object having it's
 * value represent the sum of all match points gained in the league.
 *
 * @param allTeamMatchPoints
 * @return
 */
vector <TeamValue> reduceTeamMatchPoints(const vector <TeamValue> allTeamMatchPoints) {

    // Using a map here makes it easier to reduce into a single entry per team.
    map<string, int> finalTeamPoints;

    for (TeamValue matchPoints : allTeamMatchPoints) {

        string name = matchPoints.getName();
        int points = matchPoints.getValue();

        // If the name does not exist in the map, it will be initialised with the value of points.
        // Otherwise it will just add this match's points to the previous points value.
        finalTeamPoints[name] += points;
    }

    // Convert the map back into a list for better processing later.
    return convertTeamValueMapToList(finalTeamPoints);
}

/**
 * C++ does not have a readily available function that can easily split strings into vectors.
 * Since our eventual output will be a vector of size 2 (vector of the 2 teams' results in a game)
 * and we know that the delimiter we are splitting by is pre-defined
 * we are only concerned with splitting the string once with the specified delimiter, and not everywhere it may occur.
 *
 * @param resultLine
 * @param delimiter
 * @return
 */
vector <string> splitResultsLineIntoVector(string resultLine, const string delimiter) {
    vector <string> result;
    size_t pos = resultLine.find(delimiter);
    string token = resultLine.substr(0, pos);
    result.push_back(token);
    resultLine.erase(0, pos + delimiter.length());
    result.push_back(resultLine);

    return result;
}

/**
 * This is the most important function.
 * It serves as the parent for most of the other functions within this module.
 * It is responsible for reading through the file contents line by line and
 * processing the final ranks of teams in the league based on all the matches played.
 *
 * @param file
 * @return
 */
list <TeamValue> getLeagueResults(const string file) {
    vector <TeamValue> allTeamMatchPoints;
    vector <TeamValue> finalTeamMatchPoints;

    // read file contents
    ifstream infile(file);
    string line;

    // go through each line of the file
    while (getline(infile, line)) {

        vector <TeamValue> scores;

        // Each line represents the outcome of a match.
        // Each team's own outcome of the match is separated by a ", "
        // which is why we first split the line by ", " to get a matchResults vector
        // of two strings representing the outcome of each team for the match.
        vector <string> matchResults = splitResultsLineIntoVector(line, ", ");

        // Now we loop through the matchResults
        for (string result : matchResults) {

            // We parse the string into a TeamValue object for easy processing later.
            TeamValue teamResult = getTeamResultFromString(result, TEAM_RESULT_GROUPING_PATTERN);

            // We add this result to a vector representing the scores for each team of this match.
            scores.push_back(teamResult);
        }

        // Now that we have a vector of TeamValue objects for the match representing each team,
        // we can calculate the match points.
        vector <TeamValue> matchPoints = calculateMatchPoints(scores);

        // Here we concatenate the new matchPoints vector with all previous added matchPoints.
        // The purpose of this is to have a vector of TeamValue objects each representing
        // the points the team gained in a match.
        allTeamMatchPoints.insert(allTeamMatchPoints.end(), matchPoints.begin(), matchPoints.end());

    }

    // Now we reduce this vector of all our teams' matchPoints
    // into a vector containing a single entry for each team
    // with the value representing the sum of all their match points gained.
    finalTeamMatchPoints = reduceTeamMatchPoints(allTeamMatchPoints);

    // Convert our finalTeamMatchPoints vector to a list so that we can
    // easily sort our data with a custom Comparator.
    list <TeamValue> finalTeamMatchPointsList = vectorToList(finalTeamMatchPoints);

    // Sort finalTeamMatchPoints by points DESC, and then by name ASC.
    finalTeamMatchPointsList.sort(MatchPointComparator());

    // Set the team ranks on the sorted data.
    setTeamRanks(finalTeamMatchPointsList);

    return finalTeamMatchPointsList;
}


/**
 * This Print class serves as a DRY way
 * of delaying output to the console.
 *
 * In the beginning we want to simulate the console talking to you
 * and giving you enough time to read the messages.
 *
 * This class serves to make the process of accomplishing that
 * in a more flowing and succinct manner when a situation arises
 * where you may have multiple outputs to the console in sequence
 * which you do not want to display all at once.
 *
 * The class is instantiated with an initialDelay.
 * Once the print.delayed() function is invoked the initialDelay will be added to the current runningDelay.
 *
 * Every time the print.delayed() is invoked it uses the runningDelay property to figure out how long it should sleep for.
 *
 * So every subsequent call to the function will always result in an even delay between subsequent outputs.
 */
class Print {

private:
    int initialDelay;
    int runningDelay;

public:
    explicit Print(int timeout) : initialDelay(timeout), runningDelay(timeout) {}

    void withDelay(string s) {
        usleep(runningDelay);
        cout << s << '\n';

        // runningDelay is incremented by initialDelay value
        // so that next delayed print can occur timeously after this one by an initialDelay.
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
    Print print = Print(900000);

    print.ln("\nWelcome to the League Rank Calculator !\n");
    print.withDelay("This program will calculate the ranking table for a soccer league.\n");
    print.withDelay("The data for the results of the games should be stored in a text file.");

    while (running) {
        print.withDelay(
                "\nPlease provide the full path of the file where your results are stored:\n\nFull File Path To Data: ");
        print.reset(0);

        cin >> file; // read in user input and store it in the file variable
        cin.get(); // cin.get() is called so that we can wait for the user to input before going on with the program.

        // Does file exist ?
        if (fileExists(file)) {

            // It does so let's start processing
            // process the file contents and get the league results
            list <TeamValue> matchPoints = getLeagueResults(file);

            print.ln("\nLEAGUE RANK RESULTS\n");

            // Print out the ranks in a format specified in the challenge.
            for (TeamValue team : matchPoints) {
                string result =
                        to_string(team.getRank()) + ". " + team.getName() + ", " + to_string(team.getValue()) +
                        (team.getValue() == 1 ? " pt" : " pts");
                print.ln(result);
            }

            print.ln("\nWould you like to check match point results of another league ? [y/n]: ");

            cin >> userInput;
            cin.get();

            answerYes = booleanFromString(userInput);


            // if the answerYes == -1 that means their input did not match any of our options. So we repeat the question until they do.
            while (answerYes == -1) {
                print.ln("\nI do not understand your command, please try again...");
                print.ln("Would you like to check match point results of another league ? [y/n]: ");

                cin >> userInput;
                cin.get();

                answerYes = booleanFromString(userInput);
            }

            running = answerYes;
        } else {
            print.ln(
                    "\nSorry, your file does not exist ! Please double-check your file path and try again... Press [c] to continue, or any other key (besides ENTER) to exit...\n");

            cin >> userInput;
            cin.get();

            // ignore checks for unspecified strings that come back as -1 and default them to false as well.
            running = booleanFromString(userInput) == 1;

        }
    }

    print.ln("\nThank you for using the League Rank Calculator !");
    return 0;

}

