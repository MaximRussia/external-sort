
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>
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

typedef unsigned long long int _uint64;

_uint64 LINE_MAX_SIZE = 256;

struct chunk {
	chunk(FILE* f) :f(f) {
		line = new char[LINE_MAX_SIZE];
	}
	~chunk() {};

	void fini() {
		clean();
		fclose(f);
	}

	void clean() {
		delete[]line;
	}

	bool pop_line() {
		return fgets(line, LINE_MAX_SIZE, f) != NULL;
	}

	FILE*	f;
	char*	line;
};
bool cmp_1(const string& a, const string& b) { return a < b; }
bool cmp_2(const chunk& a, const chunk& b) { return strcmp(a.line, b.line) > 0; }

FILE*	 in = NULL;
FILE*    out = NULL;
char*    line = NULL;

vector<chunk>	chunks;
vector<string>	lines;
vector<string>  files;

string next() {
	static _uint64 num = 0;
	stringstream ss;
	ss << num++;
	return ss.str();
}

void write_chunk(FILE* _out) {
	sort(lines.begin(), lines.end(), cmp_1);
	for (_uint64 i = 0; i < lines.size(); ++i) {
		fwrite(lines[i].c_str(), strlen(lines[i].c_str()), 1, _out);
	}
	lines.clear();
}

typedef priority_queue< string, std::vector<string>, std::greater<string> > reverse_priority_queue;

void write_chunk(FILE* _out, reverse_priority_queue &q) {

    while(!q.empty()) {
        string ch = q.top(); q.pop();
        fwrite(ch.c_str(), strlen(ch.c_str()), 1, _out);
    }
}

void add_chunk() {
	string fname = "sort_" + next();
	FILE* _out = fopen(fname.c_str(), "wb+");
	write_chunk(_out);
	fseek(_out, 0, SEEK_SET);
	chunks.push_back(chunk(_out));
	files.push_back(fname);
	chunks.back().pop_line();
};

void complete() {
	fclose(out);
	fclose(in);
	delete[] line;
	for (_uint64 i = 0; i < chunks.size(); ++i) {
		chunks[i].fini();
	}
	for (_uint64 i = 0; i < files.size(); ++i) {
		remove(files[i].c_str());
	}
	files.clear();
	chunks.clear();
	lines.clear();
}

_uint64 fize_size(char* s) {
	return ifstream(s, std::ifstream::ate | std::ifstream::binary).tellg();
}

int main(int argc, char* argv[]) {

	try {

		if (argc != 4) {
			throw ("invalid args");
		}

		_uint64 BUF_SIZE = atoll(argv[3]);

		// check allocation
		char* buf = new char[BUF_SIZE];
		delete[]buf;

		_uint64 MAX_READ_COUT = (BUF_SIZE / LINE_MAX_SIZE) + 1;
		_uint64 read_count = MAX_READ_COUT;

		in = fopen(argv[1], "rb");

		if (!in) {
			throw ("input file error");
		}

		_uint64 fileSize = fize_size(argv[1]);
		if (!fileSize) {
			throw ("empty file error");
		}

		out = fopen(argv[2], "wb");
		line = new char[LINE_MAX_SIZE];

		if (fileSize <= BUF_SIZE) { // sort in memory, lucky!
			while (fgets(line, LINE_MAX_SIZE, in) != NULL) {
				lines.push_back(string(line));
			}
			write_chunk(out);
			complete();
			return 0;
		}

		while (fgets(line, LINE_MAX_SIZE, in) != NULL) {
			lines.push_back(string(line));
			read_count--;
			if (read_count <= 0) {
				add_chunk();
				read_count = MAX_READ_COUT;
			}
		}

		if (!lines.empty()) {
			add_chunk();
		}


	/*
		while (!chunks.empty()) {
			sort(chunks.begin(), chunks.end(), cmp_2);
			fwrite(chunks.back().line, strlen(chunks.back().line), 1, out);
			if (!chunks.back().pop_line()) {
				chunks.back().fini();
				chunks.pop_back();
			}
		}
	*/
    
		reverse_priority_queue q;
		read_count = MAX_READ_COUT;
		while (!chunks.empty()) {
			while (!chunks.empty() && read_count) {
				for (_uint64 i = 0; i < chunks.size();) {
					q.push(string(chunks[i].line));
					if (!chunks[i].pop_line()) {
						chunks[i].fini();
						chunks.erase(chunks.begin() + i);
					}
					else i++;

					read_count--;
				}
			}

			write_chunk(out, q);
			read_count = MAX_READ_COUT;
		}
    
		complete();
	}
	catch (const char *ex) {
		cout << "RUNTIME    : " << ex << endl;
		complete();
	}
	catch (bad_alloc &ex) {
		cout << "ALLOCATOR  : " << ex.what() << endl;
		complete();
	}
	catch (exception &ex) {
		cout << "COMMON     : " << ex.what() << endl;
		complete();
	}

	return 0;
}
