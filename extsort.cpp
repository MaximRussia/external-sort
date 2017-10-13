#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <algorithm>
#include <utility>
#include <fstream>
#include <iterator>
#include <sstream>
#include <cassert>
#include <ctime>
#include <thread>
#include <limits>
#include <list>
#include <queue>
#include <set>
#include <forward_list>
#include <memory>
#include <exception>
using namespace std;

/*
base64 /dev/urandom | head -c 1000000 > big.txt
g++ -std=c++11 extsort.cpp && time ./a.out big.txt out.txt
*/

struct CHUNK {
    string line;
    string file;
    ifstream ifs;

    string next() {
        static size_t id = 0;
        stringstream ss;
        ss << id++;
        return ss.str();
    }

    CHUNK(vector<string> & data) {
        file = next() + ".txt";

        ofstream ofs(file, std::ios::binary);
        ostream_iterator<string> osi(ofs, "\n");
        copy(data.begin(), data.end(), osi);
        ofs.close();

        ifs.open(file);
    }

    ~CHUNK() {
        ifs.close();
        remove(file.c_str());
    }

    bool pop() {
        getline(ifs, line);
        return !ifs.eof();
    }

    string get() const {
        return line;
    }
};

struct SORTER {
    string line;
    vector<string> data;
    list< shared_ptr<CHUNK> > chunks;
    ifstream ifs;
    ofstream ofs;

    size_t fize_size(string s) {
        return ifstream(s, std::ifstream::ate | std::ifstream::binary).tellg();
    }

    SORTER(string in, string out) {
        ifs.open(in, ios::binary);
        ofs.open(out, ios::binary);

        if(fize_size(in) == 0) throw runtime_error("empty file ....");
		if (!ifs.good() || !ofs.good()) throw runtime_error("no files ....");
    }

    void run(long READ_NUM) {

        if(READ_NUM <= 0) throw runtime_error("bad args....");

        while(!ifs.eof()) {
            long read = READ_NUM;
            while(read-- && !ifs.eof()) {
                getline(ifs, line);
                data.push_back(line);
            }

            sort(data.begin(), data.end(), less<string>() );
            chunks.push_back( make_shared<CHUNK>(data) );
            chunks.back()->pop();
            data.clear();
        }

        ifs.close();

        while(!chunks.empty()) {
            auto res = min_element(chunks.begin(), chunks.end(),
                                   []( const shared_ptr<CHUNK> & ch1, const shared_ptr<CHUNK> & ch2)
                                   {
                                       return ch1->get() > ch2->get();
                                   });

            ofs << (*res)->get() << endl;
            if(!(*res)->pop()) {
                chunks.erase(res);
            }
        }

        ofs.close();
    }
};

int main(int argc, char *argv[]) {

    try {

        if(argc != 4) throw runtime_error("invalid args");

        SORTER s(argv[1], argv[2]);
        s.run(atol(argv[3]) / 128 /* LINE LEN */);

    } catch (exception &ex) {
        cout << ex.what() << endl;
    }

    return 0;
}
