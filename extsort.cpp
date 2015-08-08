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
#define LINE_MAX_SIZE ( 1024 )
#define MAX_READ_COUT ( BUF_SIZE / LINE_MAX_SIZE )

struct chunk
{
	chunk( FILE* f ) : f_( f ) { line_.resize(LINE_MAX_SIZE); }
    ~chunk() {}

	bool pop_line( void )
	{
        return fgets( const_cast<char*>(line_.c_str()) /* OH! GUSH! SO NAUGHTY! */, LINE_MAX_SIZE, f_ );
    }

	FILE*	f_;
	string	line_;
};

int main( int argc, char** argv )
{

    assert(MAX_READ_COUT > 0); // so kek

	FILE*	 in	= fopen( argv[ 1 ], "rb" );
	char*	 line	= (char*) malloc( LINE_MAX_SIZE ); // memory buffer
    unsigned read_count = MAX_READ_COUT;

	vector<chunk>	chunks;
	vector<string>	lines;

	auto add_chunk = [&]()
	{
        //
        // sort lines and write in chunk file
        //

		std::sort( lines.begin(), lines.end(), [&]( const string& a, const string& b ) { return a < b; } );

		string fname = "sort";
		fname += to_string(chunks.size());

        _p("add chunk " + fname)

		FILE* out = fopen( fname.c_str(), "wb+" );
		chunks.push_back( out );

		for( auto& s : lines )
			fwrite( s.c_str(), strlen( s.c_str() ), 1, out );

		fseek( out, 0, SEEK_SET );

		//clear lines
		lines.clear();
	};

	while( fgets( line, LINE_MAX_SIZE, in ) )
	{
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

	_p("use " + to_string(chunks.size()) + " chunks");

	FILE* out = fopen( argv[ 2 ], "wt" );
	for( auto& c : chunks ) c.pop_line(); // read line from chunks

	while( chunks.size() )
	{
		std::sort( chunks.begin(), chunks.end(), [&]( const chunk& a, const chunk& b ) { return a.line_ < b.line_; } );
		fwrite( chunks.back().line_.c_str(), strlen( chunks.back().line_.c_str() ), 1, out );

		if( !chunks.back().pop_line() )
			chunks.pop_back();
	}

    _p("done");

	fclose( out );
	fclose ( in );
	delete []line;
}
