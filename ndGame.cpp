#include <iostream>
#include <cassert>
#include <random>
#include <sstream>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

typedef char* cell_t;

struct Board
{
    const int n;
    const int d;
    const char* array;
};

Board initboard(int n, int d, bool rand) // init a d dimensional, Board struct.
{   
    int size = pow(n, d);
    char* array = (char*)malloc(size);
    for (int idx = 0; idx < size; ++idx)
    {
        if (rand)
        {
            switch (std::rand() % 3)
            {
            case 0:
                array[idx] = 'o';
                break;
            case 1:
                array[idx] = 'x';
                break;
            case 2:
                array[idx] = '-';
                break;
            }
        }
        else
            array[idx] = '-';
    }
    return Board{n, d, array};
}

int appendHorizontly(std::string& baseStr, std::string strToAppend, std::string spacing) // appends two strings *horizontally*.
{   // the two str must have the same amount of '\n'.

    /*
    1. seperate strToAppend with '\n'.
    2. add before every '\n' in baseStr spacing + the corrisponding strToAppend line
    */
    strToAppend += '\n';
    baseStr += '\n';

    if (std::count(strToAppend.begin(), strToAppend.end(), '\n') != std::count(baseStr.begin(), baseStr.end(), '\n'))
        return -1;
    // assert the newline counts are equal
    int lastNL = 0;
    while (!strToAppend.empty())
    {
        std::string substr = strToAppend.substr(0, strToAppend.find('\n')); // crop from beg. to \n, not including \n.
        // .substr(index, length)
        strToAppend.erase(0, strToAppend.find('\n') + 1);                   // delete from beg. to \n.
        // from "blah\nblah" >==> substr = blah, strToAppend = "blah"
        int n = baseStr.find('\n', lastNL);
        // n = idx of the next \n
        baseStr.insert(n, spacing + substr);
        lastNL = baseStr.find('\n', lastNL) + 1;
    }
    baseStr.erase(baseStr.end() - 1);
    return 0;
}

std::string Board_toString(const Board &board)    // recursivly prints the board
{
/*  2d:
*   X - - \n
*   - O - \n
*   X - O \n
* 
*   3d:         4d:
*   X - - \n    X - -   X - -   X - - \n
*   - O - \n    - O -   - O -   - O - \n
*   X - O \n    X - O   X - O   X - O \n
*
*   X - - \n    X - -   X - -   X - - \n
*   - O - \n    - O -   - O -   - O - \n
*   X - O \n    X - O   X - O   X - O \n
* 
*   X - - \n    X - -   X - -   X - - \n
*   - O - \n    - O -   - O -   - O - \n
*   X - O \n    X - O   X - O   X - O \n
*/
    std::string str = "";
    if (board.d == 2)
    {
        for (int k = 0; k < board.n * board.n; k += board.n)  // k = stride
        {
            for (int j = 0; j < board.n; ++j)   // j = step
                str += {board.array[k + j], ' '};
            str += '\n';
        }
        return str;
    }

    if (board.d % 2 == 1) // d = 3, 5 etc
    {
        std::string spacing(board.d - 2, '\n'); // passing arguments for construction

        for (int i = 0; i < pow(board.n, board.d); i += pow(board.n, board.d - 1))
            str += Board_toString(Board{ board.n, board.d-1, board.array+i}) + spacing; // pass every hyperplane in the hypercube to prn(d-1).
        // array+i == ptr to next hyperplane

        return str;
    }
    // d = 4, 6 etc
    // generate d-1 strings, and nail them horizontaly.
    std::string spacing(board.d, ' ');

    str = Board_toString(Board{ board.n, board.d - 1, board.array}); // gotta start with the same number of newlines.
    int leap = pow(board.n, board.d - 1);
    for (int i = leap; i < board.n*leap; i += leap) // i = 1 because ^, also n*leap = pow(n, d)
        appendHorizontly(str, Board_toString(Board{ board.n, board.d - 1, board.array + i }), spacing);

    return str;
}

cell_t vector_toCell(std::vector<int> vector, Board &board)
{
    cell_t cell = (char*)board.array;
    for (int curDimension = 0; curDimension < board.d; ++curDimension)
        cell += static_cast<int>(pow(board.n, curDimension)) * vector[curDimension]; // for every dimension, cell += step*cordinate
    return cell;
}

int mark(std::vector<int> vector, char ch, Board &board, bool debug)   // marks a char in the board.
{   // also, verifies vector.
    // -1: vector doesnt match.
    // -2: not on the board.
    // -3: already marked.

    if (vector.size() != board.d)
        return -1;
    for (int val : vector)
        if (val >= board.n)
            return -2;
    cell_t cell = vector_toCell(vector, board);
    if (!debug && *cell != '-')
        return -3;
    *cell = ch;
    return 0;
}

char checkLine(std::vector<int> vector, std::vector<int> step_vector, Board &board) { //check whether a line is won, and if so highlights it

    bool zero_step = true;
    for (int val : step_vector)
        if (val != 0)
            zero_step = false;
    if (zero_step)
        return -1;
    // verify step > 0.

    std::vector<int> base_vector = vector;
    for (int idx = 0; idx < base_vector.size(); ++idx) {
        if (step_vector[idx] == 1)
            base_vector[idx] = 0;
        else if (step_vector[idx] == -1)
            base_vector[idx] = board.n-1;
    }
    // calculate base vector

    char ch = *vector_toCell(base_vector, board);

    vector = base_vector;
    for (int bidx = 1; bidx < board.n; ++bidx) {

        for (int vidx = 0; vidx < vector.size(); ++vidx)
            vector[vidx] += step_vector[vidx];
        // increment vector

        if (ch != *vector_toCell(vector, board))
            return 0;
    }
    // check line

    vector = base_vector;
    for (int bidx = 0; bidx < board.n; ++bidx) {

        cell_t cell = vector_toCell(vector, board);
        *cell = toupper(*cell);
        std::cout << *cell;

        for (int vidx = 0; vidx < vector.size(); ++vidx)
            vector[vidx] += step_vector[vidx];
        // increment vector
    }
    // highlight line
    return ch;
}

int binarydec(std::vector<bool>& bitset) { // treats a boolean vector as a binary number, subtracts 1.
// returns the new bitset converted to an int.
    int num = 0;
    int idx = 0;
    while (!bitset[idx]) {
        bitset[idx] = !bitset[idx];
        num += pow(2, idx) * bitset[idx];
        if (++idx >= bitset.size())
            return -1;
    }
    bitset[idx] = !bitset[idx];
    num += pow(2, idx) * bitset[idx];
    return num;
}


char checkForVector(std::vector<int> vector, Board &board){
    /*  straight lines are generalized as 1 dimensional diagonals.

        diagonals: if a set of n cordinates are equal/opposite, this is an nth dimensional diagonal.
        if {a, a, n-a-1, b}, step is n^0 + n^1 -n^2.

        method: 
        - generate step-vectors:
            - for every 0 < a < n:
                - if ([vidx] == a) push vidx to map{}, push 1 to step_vector{}
                - elif ([vidx] == n-a-1) push vidx to map{}, push -1 to step_vector{}
                - else push 0 to step_vector{}

                - new bitset[size = map.size()] stepbits set all bits to 1
                - for (; stepbits > 0; --stepbits)
                    - create step_subvector from the mapped bits on step_vector{} using map{}
                    - checkLine(vector, step_subvector)

        e.i.: {1, 1, 1} n=3 ~> {1, 1, 1}, {1, 1, 0}, {1, 0, 1}, {0, 1, 1} ~> steps = {1+n+n^2, 1+n, 1+n^2, n+n^2}
              {0, 1, 2} n=3 ~> {1, 0, -1} ~> step = 1-n^2
              {0, 1, 2, 3} n=4 ~> {1, 0, 0, -1}, {0, 1, -1, 0}
              {1, 2, 1, 2} n=4 ~> 2D: {1 -1 0 0}, {0 0 1 -1}, {1 0 1 0}, {0 1 0 1}, {1 0 0 -1}, {0 -1 1 0}
                               ~> 3D: {1 -1 1 0}, {1 -1 0 -1}, {1 0 1 -1}, {0 -1 1 -1}
                               ~> 4D: {1, -1, 1, -1}
    */
    for (int a = 0; a < board.n; ++a) {
        std::vector<int> map{};
        std::vector<int> step_vector{};
        for (int vidx = 0; vidx < vector.size(); ++vidx) {
            if (a == vector[vidx])
                map.push_back(vidx), step_vector.push_back(1);
            else if (a == board.n-vector[vidx]-1)
                map.push_back(vidx), step_vector.push_back(-1);
            else
                step_vector.push_back(0);
        }
        std::vector<bool> dimensions{};
        for (int i : map)
            dimensions.push_back(true);

        while (true) {

            std::vector<int> step_subvector{ step_vector };
            for (int sidx = 0; sidx < dimensions.size(); ++sidx) // sidx == subset idx
                if (!dimensions[sidx])
                    step_subvector[map[sidx]] = 0;
            // generate subset of step vector

            char result = checkLine(vector, step_subvector, board);
            if (result == -1)
                break;
            else if (result == 'o' || result == 'x')
                return result;

            binarydec(dimensions);
        }
    }
    return 0;
}

std::vector<int> stov(std::string str) { // casts vector to std::string
    // Doesn't verify vector.

    std::stringstream ss;
    ss << str;
    std::vector<int> vector;
    while (!ss.eof()) {
        std::string tmp;
        ss >> tmp;
        if (!tmp.empty())
            vector.push_back(stoi(tmp));
    }
    return vector;
}

int main()
{
    std::cout << "  Dimensions to play in: \n> ";
    int dimensions;
    std::cin >> dimensions;
    std::cin.ignore(32768, '\n');
    std::cout << "  Size of board: \n> ";
    int n;                                              // n = length, width, depth, or any spacial dimension of the hyperboard
    std::cin >> n;
    std::cin.ignore(32768, '\n');
    assert(n > 1 && dimensions > 1);
    Board board = initboard(n, dimensions, false);
    
    std::cout << Board_toString(board);
    char winner = 0;
    char player = 'o';
    while (!winner) {
        system("cls");
        std::cout << Board_toString(board);
        std::cout << "\n  It is " << player << "'s turn.\n  Where do you mark?\n> ";
        std::string move;
        //std::cin.ignore(32768, '\n');
        std::getline(std::cin, move);
        std::vector<int> move_vector = stov(move);
        int error = mark(move_vector, player, board, false);
        switch (error)
        {
        case -1:
            std::cout << "  Vector doesn't match.\n";
            Sleep(1000);
            continue;
        case -2:
            std::cout << "  Not on the board.\n";
            Sleep(1000);
            continue;
        case -3:
            std::cout << "  Position already taken.\n";
            Sleep(1000);
            continue;
        }
        winner = checkForVector(move_vector, board);
        player = 'o' + 'x' - player; // switch between o and x
    }
    system("cls");
    std::cout << Board_toString(board) << "\n  The winner is " << winner;

    return 0;
}
