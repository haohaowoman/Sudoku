#include <iostream>
#include <vector>
#include <random>
#include <cassert>
#include <map>
#include <string>
#include <chrono>
#include "../sudo/sudoku.h"

using namespace std;

#ifdef _ALGLORITHM_TEST

using skMatrix = vector<vector<int>>;

bool genItem(skMatrix& src, int r, int c);

bool generatorSudoku(skMatrix& src, size_t nitem);

random_device rdev;
mt19937 reng(rdev());

vector<int> getCandidate(const skMatrix& src, int row, int col)
{
    vector<int> candi;
    size_t sz = src.size();
    vector<bool> trac(sz + 1, false);
    for (size_t i = 0; i < sz; i++)
    {
        trac[src[row][i]] = true;
        trac[src[i][col]] = true;
    }
    int candCount = 0;
    vector<int> cand(sz, 0);
    for (int i = 1; i <= (int)sz; i++)
    {
        if (!trac[i])
        {
            candi.push_back(i);
        }
    }
    return candi;
}

struct idxWeight
{
    size_t idx;
    size_t weight;
};

vector<int> outOfOrder(const vector<int>& src)
{
    if (src.empty())
    {
        return src;
    }

    vector<idxWeight> dw(src.size());
    uniform_int_distribution<size_t> rrang(0, 10000);
    for (size_t i = 0; i < src.size(); i++)
    {
        dw[i] = { i, rrang(reng) };
    }
    sort(dw.begin(), dw.end(), [](vector<idxWeight>::const_reference l, vector<idxWeight>::const_reference r) { return l.weight > r.weight; });
    vector<int> result(src.size());
    for (size_t i = 0; i < dw.size(); i++)
    {
        result[i] = src[dw[i].idx];
    }
    return result;
}

bool generatorSudoku(skMatrix& src, size_t nitem)
{
    size_t rows = src.size();
    size_t nCount = rows * rows;
    if (nitem >= nCount)
    {
        return true;
    }

    size_t row = nitem / rows;
    size_t col = nitem % rows;

    if (src[row][col] != 0)
    {
        return generatorSudoku(src, nitem + 1);
    }

    auto candidate = outOfOrder(getCandidate(src, row, col));
    if (!candidate.empty())
    {
        for (size_t i = 0; i < candidate.size(); i++)
        {
            src[row][col] = candidate[i];
            if (generatorSudoku(src, nitem + 1))
            {
                return true;
            }
        }
        src[row][col] = 0;
    }

    return false;
}


bool genItem(skMatrix& src, int r, int c)
{
    size_t sz = src.size();
    vector<bool> trac(sz + 1, false);
    for (size_t i = 0; i < sz; i++)
    {
        trac[src[r][i]] = true;
        trac[src[i][c]] = true;
    }

    vector<int> cand(sz, 0);
    int candCount = 0;
    for (size_t i = 1; i <= sz; i++)
    {
        if (!trac[i])
        {
            cand[candCount++] = (int)i;
        }
    }

    if (candCount > 1)
    {
        uniform_int_distribution<int> rrang(0, candCount - 1);
        src[r][c] = cand[rrang(reng)];
    }
    else if (candCount == 1)
    {
        src[r][c] = cand[0];
    }

    return candCount != 0;
}

#endif // _ALGLORITHM_TEST

map<string, char> _flags = {
    {"-o", 'o'},
    {"--order", 'o'},
    {"-n", 'n'},
    {"-numbers", 'n'},
    {"-c", 'c'}
};

struct ISudokuOEM
{
    
};

std::shared_ptr<ISudokuOEM> CreateSudokuFactory(int order)
{
    std::shared_ptr<ISudokuOEM> inst;
    switch (order)
    {
    case 9:
        break;
    default:
        break;
    }
    return inst;
}

int main(int argc, const char** args)
{
    int order = 6;
    size_t nums = 100;
    bool compile = false;
    int level = 2;
    for (int i = 1; i < argc; i++)
    {
        auto fit = _flags.find(args[i]);
        if (_flags.end() == fit)
        {
            try
            {
                order = stoi(args[i]);
            }
            catch (const std::exception&)
            {
                cerr << "Invalid arguments..." << endl;
                return 1;
            }
        }
        else
        {
            switch (fit->second)
            {
            case 'o':
            {
                try
                {
                    if (++i < argc)
                    {
                        size_t index = 0;
                        string argStr = args[i];
                        order = stoi(argStr, &index);
                    }
                    else
                    {
                        order = 3;
                    }
                }
                catch (const std::exception&)
                {
                    order = 0;
                }
            }
            break;
            case 'n':
            {
                if (++i < argc)
                {
                    try
                    {
                        size_t index = 0;
                        string argStr = args[i];
                        nums = stoi(argStr, &index);
                    }
                    catch (const std::exception&)
                    {
                        cerr << "Invalid arguments of numbers ..." << endl;
                        return 1;
                    }
                }
            }
            break;
            case 'c':
                compile = true;
                break;
            default:
                break;
            }
        }
    }

    if (order < 3 || order > 9)
    {
        cerr << "Invalid arguments of order..." << endl;
        return 1;
    }
    
    sudoku::seek_9 sk9;
    
    std::chrono::high_resolution_clock begin;
    auto last = begin.now();
    for (size_t n = 0; n < nums; n++)
    {
        sk9.next();
    }
    
    cout << "backtree: " << chrono::duration_cast<chrono::milliseconds>((begin.now() - last)).count() / (float)nums << endl;

    return 0;
}
