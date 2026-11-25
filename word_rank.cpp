// ============================================================================
//  RANK OF A WORD  --  Lexicographic (dictionary) rank using Permutations & Combinations
//  Version 1.0
//
//  IDEA
//  ----
//  Take the given word, write its letters in alphabetical order (a -> z).
//  All the arrangements of those letters, listed alphabetically, form a
//  dictionary. The "rank" of the word is its position (1-based) in that
//  dictionary.
//
//  PnC TECHNIQUE
//  -------------
//  Go position by position, left to right. At each position count how many
//  smaller letters could have been placed there instead; for each of them the
//  remaining letters can be arranged in
//
//        (remaining)!  /  (product of factorials of repeated letters)!
//
//  ways. Add all of those blocks, then fix the real letter and move right.
//  Rank = 1 + (sum of all those blocks).
//
//  Repeated letters, mixed case and spaces in the input are handled.
//  Case-insensitive by default (A and a are the same letter).
//
//  BUILD :  g++ -std=c++17 -O2 -o word_rank word_rank.cpp
//  RUN   :  ./word_rank                 interactive
//           ./word_rank APPLE           one word, full explanation
//           ./word_rank --rank APPLE    print only the rank number
//           ./word_rank --version       version
// ============================================================================

#include <algorithm>
#include <cctype>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

using i128 = __int128;                 // 26! = 4.03e26, needs more than 64 bits
static i128 fact[27];

// ---------------------------------------------------------------- utilities

static void buildFactorials() {
    fact[0] = 1;
    for (int i = 1; i <= 26; ++i) fact[i] = fact[i - 1] * i;
}

static string bigToString(i128 v) {
    if (v == 0) return "0";
    bool neg = v < 0;
    if (neg) v = -v;
    string s;
    while (v > 0) {
        s.push_back(char('0' + int(v % 10)));
        v /= 10;
    }
    if (neg) s.push_back('-');
    reverse(s.begin(), s.end());
    return s;
}

static int totalOf(const vector<int>& cnt) {
    return accumulate(cnt.begin(), cnt.end(), 0);
}

// how many distinct arrangements the given letters can make
static i128 arrangements(const vector<int>& cnt) {
    i128 res = fact[totalOf(cnt)];
    for (int c = 0; c < 26; ++c)
        if (cnt[c] > 1) res /= fact[cnt[c]];
    return res;
}

// "3! / (2!)" style formula for the same number
static string formulaOf(const vector<int>& cnt) {
    string s = to_string(totalOf(cnt)) + "!";
    string den;
    for (int c = 0; c < 26; ++c)
        if (cnt[c] > 1) den += (den.empty() ? "" : " * ") + to_string(cnt[c]) + "!";
    if (!den.empty()) s += " / (" + den + ")";
    return s;
}

// ------------------------------------------------------------------ input

// trims, upper-cases, rejects anything that is not a letter; max 26 letters
static bool normalizeWord(const string& in, string& out, string& err) {
    string w;
    for (char ch : in) {
        if (isspace(static_cast<unsigned char>(ch))) continue;
        if (!isalpha(static_cast<unsigned char>(ch))) {
            err = string("'") + ch + "' is not a letter. Only A-Z / a-z allowed.";
            return false;
        }
        w.push_back(char(toupper(static_cast<unsigned char>(ch))));
    }
    if (w.empty()) { err = "The word is empty."; return false; }
    if (w.size() > 26) { err = "Word too long (max 26 letters)."; return false; }
    out = w;
    return true;
}

// ------------------------------------------------------------- the answer

static i128 rankOf(const string& w) {
    vector<int> cnt(26, 0);
    for (char c : w) cnt[c - 'A']++;

    i128 rank = 1;
    for (size_t i = 0; i < w.size(); ++i) {
        int cur = w[i] - 'A';
        // the real letter is still in the pool while we count the blocks
        for (int c = 0; c < cur; ++c) {
            if (cnt[c] == 0) continue;          // this smaller letter is used up
            cnt[c]--;
            rank += arrangements(cnt);
            cnt[c]++;
        }
        cnt[cur]--;                             // now fix this letter here
    }
    return rank;
}

// k-th arrangement of the multiset (1-based) -- used for the neighbours
static string kthArrangement(vector<int> cnt, i128 k) {
    int n = totalOf(cnt);
    string out;
    for (int pos = 0; pos < n; ++pos) {
        for (int c = 0; c < 26; ++c) {
            if (cnt[c] == 0) continue;
            cnt[c]--;
            i128 block = arrangements(cnt);
            if (k > block) {
                k -= block;
                cnt[c]++;
            } else {
                out.push_back(char('A' + c));
                break;
            }
        }
    }
    return out;
}

static void printFullReport(const string& w) {
    const int n = int(w.size());

    vector<int> cnt(26, 0);
    for (char c : w) cnt[c - 'A']++;

    // ---- header -----------------------------------------------------------
    string sortedLetters;
    for (int c = 0; c < 26; ++c)
        for (int k = 0; k < cnt[c]; ++k) sortedLetters.push_back(char('A' + c));

    string freq;
    for (int c = 0; c < 26; ++c)
        if (cnt[c]) freq += string(1, char('A' + c)) + "=" + to_string(cnt[c]) + "  ";

    cout << "\n==============================================================\n";
    cout << "  WORD            : " << w << "\n";
    cout << "  LENGTH          : " << n << " letters\n";
    cout << "  ALPHABETICAL    : " << sortedLetters << "   (a -> z order)\n";
    cout << "  FREQUENCY       : " << freq << "\n";
    cout << "  TOTAL ARRANGEMENTS : " << formulaOf(cnt) << " = " << bigToString(arrangements(cnt)) << "\n";
    cout << "==============================================================\n";

    // ---- step by step -----------------------------------------------------
    cout << "\nSTEP-BY-STEP WORKING (PnC)\n";
    cout << "--------------------------------------------------------------\n";

    i128 rank = 1;
    for (int i = 0; i < n; ++i) {
        int cur = w[i] - 'A';

        cout << "Position " << (i + 1) << " -> real letter is '" << w[i] << "'\n";

        i128 added = 0;
        bool any = false;
        for (int c = 0; c < cur; ++c) {
            if (cnt[c] == 0) continue;
            cnt[c]--;
            i128 p = arrangements(cnt);
            added += p;
            any = true;
            cout << "    put '" << char('A' + c) << "' here instead (smaller letter)\n";
            cout << "        remaining " << totalOf(cnt) << " letters  ->  "
                 << formulaOf(cnt) << " = " << bigToString(p) << " words\n";
            cnt[c]++;
        }
        if (!any)
            cout << "    no smaller letter left  ->  0 words before it\n";

        cnt[cur]--;                                 // fix the real letter here

        rank += added;
        cout << "    words skipped at this position : " << bigToString(added)
             << "     rank so far : " << bigToString(rank) << "\n\n";
    }

    i128 total = arrangements([&] {
        vector<int> c0(26, 0);
        for (char ch : w) c0[ch - 'A']++;
        return c0;
    }());

    cout << "--------------------------------------------------------------\n";
    cout << "  RANK OF \"" << w << "\"  =  " << bigToString(rank)
         << "   (out of " << bigToString(total) << " arrangements)\n";
    cout << "--------------------------------------------------------------\n";

    vector<int> c0(26, 0);
    for (char ch : w) c0[ch - 'A']++;
    if (rank > 1)
        cout << "  just before it : " << kthArrangement(c0, rank - 1) << "\n";
    else
        cout << "  it is the first arrangement (smallest possible)\n";
    if (rank < total)
        cout << "  just after it  : " << kthArrangement(c0, rank + 1) << "\n";
    else
        cout << "  it is the last arrangement (largest possible)\n";
    cout << "\n";
}

// ------------------------------------------------------------------- main

int main(int argc, char** argv) {
    buildFactorials();

    vector<string> args(argv + 1, argv + argc);

    if (!args.empty() && (args[0] == "--version" || args[0] == "-v")) {
        cout << "Rank of a Word  v1.0\n";
        return 0;
    }

    // ---- script mode: ./word_rank --rank WORD -----------------------------
    if (!args.empty() && (args[0] == "--rank" || args[0] == "-r")) {
        if (args.size() < 2) {
            cerr << "usage: word_rank --rank <word>\n";
            return 1;
        }
        string w, err;
        if (!normalizeWord(args[1], w, err)) {
            cerr << "error: " << err << "\n";
            return 1;
        }
        cout << bigToString(rankOf(w)) << "\n";
        return 0;
    }

    // ---- one word from the command line -----------------------------------
    if (!args.empty()) {
        string w, err;
        if (!normalizeWord(args[0], w, err)) {
            cerr << "error: " << err << "\n";
            return 1;
        }
        printFullReport(w);
        return 0;
    }

    // ---- interactive ------------------------------------------------------
    cout << "==============================================================\n";
    cout << "   RANK OF A WORD  --  dictionary rank using PnC      v1.0\n";
    cout << "   Letters only, A-Z / a-z, max 26.  Case does not matter.\n";
    cout << "   Type 'exit' to quit.\n";
    cout << "==============================================================\n";

    string line;
    while (true) {
        cout << "\nEnter a word : ";
        if (!getline(cin, line)) break;

        string probe;
        for (char ch : line)
            if (!isspace(static_cast<unsigned char>(ch))) probe.push_back(char(tolower(static_cast<unsigned char>(ch))));

        if (probe == "exit" || probe == "quit") break;
        if (probe.empty()) continue;

        string w, err;
        if (!normalizeWord(line, w, err)) {
            cout << "  !! " << err << "\n";
            continue;
        }
        printFullReport(w);
    }

    cout << "\nBye.\n";
    return 0;
}
