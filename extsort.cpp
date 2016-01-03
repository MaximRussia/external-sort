/*
base64 /dev/urandom | head -c 1000000 > big.txt
g++ -std=c++11 extsort.cpp && time ./a.out big.txt out.txt
du big.txt out.txt
wc big.txt out.txt
wc out.txt big.txt && du out.txt big.txt
g++ -std=c++11 extsort.cpp && time ./a.out big.txt out.txt && wc out.txt big.txt && du out.txt big.txt
*/

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>
#include <cassert>
#include <iostream>
using namespace std;

#define _p(v) { cout << v << endl; }

#define BUF_SIZE ( 25 * 1024 * 1024 ) // 25MB
#define LINE_MAX_SIZE ( 1024 )        // 1024 chars per line
#define MAX_READ_COUT ( BUF_SIZE / LINE_MAX_SIZE )

struct chunk {
	chunk( FILE* f ) : f( f ) {
	    line = new char[LINE_MAX_SIZE];
    }
	~chunk() {} // { delete []line; fclose(f); }

	bool pop_line() {
        return fgets(line, LINE_MAX_SIZE, f);
    }

	FILE*	f;
	char*	line;
};

int main( int argc, char** argv ) {
	FILE*	 in	= NULL;
	FILE*    out = NULL;

	if(argc > 2) {
        in = fopen( argv[ 1 ], "rb" );
        out = fopen( argv[ 2 ], "wt" );
	}
	else {
        in = fopen( "big.txt", "rb" );
        out = fopen( "out.txt", "wt" );
	}

	char*	 line	= new char[LINE_MAX_SIZE]; // memory buffer
	unsigned read_count = MAX_READ_COUT;

	vector<chunk>	chunks;
	vector<string>	lines;

	auto add_chunk = [&]() {
        //
        // sort lines and write in chunk file
        //

		std::sort( lines.begin(), lines.end(), [&]( const string& a, const string& b ) { return a < b; } );

		string fname = "sort";
		fname += std::string(1, '0' + chunks.size());

		_p("add chunk " + fname)

		FILE* _out = fopen( fname.c_str(), "wb+" );

		for( auto& s : lines )
			fwrite( s.c_str(), strlen( s.c_str() ), 1, _out );

		fseek( _out, 0, SEEK_SET );

		chunks.push_back( _out );
		chunks.back().pop_line();

		//clear lines
		lines.clear();
	};

	while( fgets( line, LINE_MAX_SIZE, in ) ) {
		lines.push_back( string(line) );
		read_count--;

		if( read_count <= 0)
		{
			add_chunk();
			read_count = MAX_READ_COUT;
        }
	}

	// if after 'while' lines not empty, create one more chunk
	if( !lines.empty() ) add_chunk();

	_p("use " << chunks.size() << " chunks");

	while( !chunks.empty() ) {
		std::sort( chunks.begin(), chunks.end(), [&]( const chunk& a, const chunk& b ) { return strcmp(a.line, b.line) < 0; } );
		fwrite( chunks.back().line, strlen( chunks.back().line ) - 1, 1, out );
		if( !chunks.back().pop_line() ) chunks.pop_back();
	}

	_p("done");

	fclose( out );
	fclose( in );
	delete []line;
	chunks.clear();
	lines.clear();
}
