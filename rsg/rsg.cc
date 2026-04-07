/**
 * File: rsg.cc
 * ------------
 * Provides the implementation of the full RSG application, which
 * relies on the services of the built-in string, ifstream, vector,
 * and map classes as well as the custom Production and Definition
 * classes provided with the assignment.
 */
 
#include <map>
#include <set>
#include <fstream>
#include "definition.h"
#include "production.h"
using namespace std;

/**
 * Takes a reference to a legitimate infile (one that's been set up
 * to layer over a file) and populates the grammar map with the
 * collection of definitions that are spelled out in the referenced
 * file.  The function is written under the assumption that the
 * referenced data file is really a grammar file that's properly
 * formatted.  You may assume that all grammars are in fact properly
 * formatted.
 *
 * @param infile a valid reference to a flat text file storing the grammar.
 * @param grammar a reference to the STL map, which maps nonterminal strings
 *                to their definitions.
 */

static void readGrammar(ifstream& infile, map<string, Definition>& grammar)
{
  while (true) {
    string uselessText;
    getline(infile, uselessText, '{');
    if (infile.eof()) return;  // true? we encountered EOF before we saw a '{': no more productions!
    infile.putback('{');
    Definition def(infile);
    grammar[def.getNonterminal()] = def;
  }
}

/**
 * Recursively expands the given symbol into a sequence of terminal tokens and
 * appends the generated tokens to `result`.
 *
 * The algorithm is a classic grammar traversal:
 *   1. If `key` is not a defined nonterminal in `grammar`, it is treated as a
 *      terminal token and appended to `result` (base case).
 *   2. Otherwise, a random production is chosen for the nonterminal via
 *      `Definition::getRandomProduction()`, and each symbol in that production is
 *      expanded recursively in order.
 *
 * This approach produces a random sentence by walking the grammar tree from the
 * given start symbol and collecting terminals in left-to-right order.
 *
 * @param key the current nonterminal (or terminal) symbol to expand.
 * @param grammar the map that associates nonterminal strings with their definitions.
 * @param result the output vector that collects terminal tokens as they are generated.
 */
static void composeSentence(const string& key, const map<string, Definition>& grammar, vector<string>& result){
  if(!grammar.count(key)){
    if(key.front() == '<' && key.back() == '>'){
      cerr << "Error: Nonterminal " << key << " is not defined in the grammar." << endl;
      exit(3); // exit with a specific error code for undefined nonterminals
    }

    result.push_back(key); // base case to stop recursion
    return;
  }

  for (auto word:grammar.at(key).getRandomProduction()) {
     composeSentence(word, grammar, result);
  }
}

/**
 * Generates a single sentence from the grammar and prints it to standard output.
 *
 * The generator always starts from the reserved nonterminal "<start>" and uses
 * `composeSentence` to recursively build a list of terminal tokens. The tokens are
 * printed space-separated, followed by a newline.
 *
 * @param grammar the map of nonterminals to their definitions.
 */
static void generateSentence(const map<string, Definition>& grammar){
  std::vector<string> result;
  composeSentence("<start>", grammar, result);

  for(const string& r: result){
    cout << r << " ";
  }
  cout << endl;
  cout << endl;
}

/**
 * Performs the rudimentary error checking needed to confirm that
 * the client provided a grammar file.  It then continues to
 * open the file, read the grammar into a map<string, Definition>,
 * and then print out the total number of Definitions that were read
 * in.  You're to update and decompose the main function to print
 * three randomly generated sentences, as illustrated by the sample
 * application.
 *
 * @param argc the number of tokens making up the command that invoked
 *             the RSG executable.  There must be at least two arguments,
 *             and only the first two are used.
 * @param argv the sequence of tokens making up the command, where each
 *             token is represented as a '\0'-terminated C string.
 */

int main(int argc, char *argv[])
{
  if (argc == 1) {
    cerr << "You need to specify the name of a grammar file." << endl;
    cerr << "Usage: rsg <path to grammar text file>" << endl;
    return 1; // non-zero return value means something bad happened 
  }
  
  ifstream grammarFile(argv[1]);
  if (grammarFile.fail()) {
    cerr << "Failed to open the file named \"" << argv[1] << "\".  Check to ensure the file exists. " << endl;
    return 2; // each bad thing has its own bad return value
  }
  
  // things are looking good...
  map<string, Definition> grammar;
  readGrammar(grammarFile, grammar);
  
  // Generate three example sentences from the grammar and print them to standard output.

  cout << "Version #1: --------------------------" << endl;
  generateSentence(grammar);
  cout << "Version #2: --------------------------" << endl;
  generateSentence(grammar);
  cout << "Version #3: --------------------------" << endl;
  generateSentence(grammar);

  return 0;
}
