/*
    base64 /dev/urandom | head -c 1000000 > big.txt
    g++ -std=c++11 extsort.cpp && time ./a.out big.txt out.txt
    du big.txt out.txt
    wc big.txt out.txt
    wc out.txt big.txt && du out.txt big.txt
    g++ -std=c++11 extsort.cpp && time ./a.out big.txt out.txt && wc out.txt big.txt && du out.txt big.txt
*/

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <cstring>
#include <string>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <memory>
using namespace std;

const size_t BUF_SIZE = 1024 * 1024; // 1MB
const size_t LINE_MAX_SIZE = 100;
const size_t MAX_READ_COUT = ( BUF_SIZE / LINE_MAX_SIZE );

struct chunk {
	chunk( FILE* f ) : f( f ) {
	    line = new char[LINE_MAX_SIZE];
    }
	~chunk(){}; // { delete []line; fclose(f); }

	/*
	chunk(const chunk& ch) {
	    f = ch.f;
	    line = ch.line;
	}

	chunk(const chunk&& ch) {
	    f = ch.f;
	    line = ch.line;
	}
    */

	bool pop_line() {
        return fgets(line, LINE_MAX_SIZE, f);
    }

	FILE*	f;
	char*	line;
};

FILE*	 in	= NULL;
FILE*    out = NULL;

char*	 line	= new char[LINE_MAX_SIZE]; // memory buffer
unsigned read_count = MAX_READ_COUT;

vector<chunk>	chunks;
vector<string>	lines;

string next() {
    static size_t num = 0;
    stringstream ss;
    ss << num++;
    return ss.str();
}

bool cmp_1( const string& a, const string& b ) { return a > b; }
bool cmp_2( const chunk& a, const chunk& b ) { return strcmp(a.line, b.line) < 0; }

void add_chunk () {
    //
    // sort lines and write in chunk file
    //

    std::sort( lines.begin(), lines.end(), cmp_1 );

    string fname = "sort_" + next();
    FILE* _out = fopen( fname.c_str(), "wb+" );

    for( int i = 0; i < lines.size(); i++ )
        fwrite( lines[i].c_str(), strlen(lines[i].c_str()), 1, _out );

    fseek( _out, 0, SEEK_SET );

    chunks.push_back( _out );
    chunks.back().pop_line();

    //clear lines
    lines.clear();
};

void complete() {
	fclose( out );
	fclose( in );
	delete []line;
	chunks.clear();
	lines.clear();
}

int main(int argc, char* argv[]) {

    try {
        if(BUF_SIZE <= 0) {
            throw std::runtime_error("allocate 0 or smaller");
        }

        size_t fileSize = ifstream("big.txt", std::ifstream::ate | std::ifstream::binary).tellg();
        if(!ifstream("big.txt").good() || !fileSize) {
            throw std::runtime_error("input file error");
        }

        ////////////////////////////
        char *t = new char[BUF_SIZE];
        delete[] t;
        ////////////////////////////

        in = fopen( "big.txt", "rb" );
        out = fopen( "out.txt", "wt" );

        if(fileSize <= BUF_SIZE) { // sort in memory, lucky!
            while( fgets( line, LINE_MAX_SIZE, in ) ) {
                lines.push_back( string(line) );
            }
            std::sort( lines.begin(), lines.end(), cmp_1 );

            while( !lines.empty() ) {
                fwrite( lines.back().c_str(),
                       strlen(lines.back().c_str())-1, 1, out );
                lines.pop_back();
            }

            complete();
            return 0;
        }

        while( fgets( line, LINE_MAX_SIZE, in ) ) {
            lines.push_back( string(line) );
            read_count--;

            if( read_count <= 0) {
                add_chunk();
                read_count = MAX_READ_COUT;
            }
        }

        // if after 'while' lines not empty, create one more chunk
        if( !lines.empty() ) add_chunk();

        while( !chunks.empty() ) {
            std::sort( chunks.begin(), chunks.end(), cmp_2);
            fwrite( chunks.back().line, strlen(chunks.back().line)-1, 1, out );
            if( !chunks.back().pop_line() ) chunks.pop_back();
        }

        complete();
    } catch (std::exception &ex) {
        cout << "exception : " << ex.what() <<  endl;
        complete();
    }

    return 0;
}
