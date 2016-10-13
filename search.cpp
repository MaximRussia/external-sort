
#include <stdlib.h>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include <string.h>
#include <queue>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;

typedef unsigned long long _uint64;

#define OFFSETOF(type, field)       ((_uint64) &(((type *) 0)->field))
#define SIZEOF(type, field)         (sizeof(((type *) 0)->field))

struct user {
	int id;
	time_t birthday;
	int gender;
	int city_id;
	time_t time_reg;

	user() {
		id = birthday = gender = city_id = time_reg = 0;
	}

	bool operator<(const user& row) const {
		return time_reg > row.time_reg;
	}

	bool operator == (const user row) const {
		return id == row.id && birthday == row.birthday && gender == row.gender && city_id == row.city_id && time_reg == row.time_reg;
	}

	friend ostream& operator << (ostream &s, const user &u) {
		s << u.id << "\t" << u.birthday << "\t" << u.gender << "\t" << u.city_id << "\t" << u.time_reg << endl;
	}
};

enum e_fields { e_id, e_birthday, e_gender, e_city_id, e_time_reg, ___e_count };

struct filter {
	bool use_filter[___e_count];
	user query;
	_uint64 cached_query;
	filter() {
		memset(use_filter, 0, ___e_count*sizeof(e_fields));
		cached_query = 0;
	}
};

_uint64 pack_user(user &u) {
	_uint64 res = 0;
	res |= u.id << (OFFSETOF(user, id) & 0xFF);
	res |= u.birthday << (OFFSETOF(user, birthday) & 0xFF);
	res |= u.gender << (OFFSETOF(user, gender) & 0xFF);
	res |= u.city_id << (OFFSETOF(user, city_id) & 0xFF);
	res |= u.time_reg << (OFFSETOF(user, time_reg) & 0xFF);
	return res;
}

bool cmp(_uint64 u1_u, _uint64 u2_u, const _uint64 mask) {
	_uint64 u1_masked = u1_u & mask;
	_uint64 u2_masked = u2_u & mask;
	return u1_masked == u2_masked;
}

bool cmp2(user &u1, user &u2, filter &f) {
	return
		(f.use_filter[e_id] && (u1.id == u2.id)) ||
		(f.use_filter[e_birthday] && (u1.birthday == u2.birthday)) ||
		(f.use_filter[e_gender] && (u1.gender == u2.gender)) ||
		(f.use_filter[e_city_id] && (u1.city_id == u2.city_id)) ||
		(f.use_filter[e_time_reg] && (u1.time_reg == u2.time_reg));
}

void fill_bits(_uint64 &mask, _uint64 from, _uint64 size) {
	_uint64 buff = (1 << (from + size)) - 1;
	buff &= ~((1 << from) - 1);
	mask |= buff;
}

_uint64 gen_mask_by_filters(filter &f) {
	_uint64 mask = 0;
	if (f.use_filter[e_id]) {
		fill_bits(mask, OFFSETOF(user, id), SIZEOF(user, id));
	}
	if (f.use_filter[e_birthday]) {
		fill_bits(mask, OFFSETOF(user, birthday), SIZEOF(user, birthday));
	}
	if (f.use_filter[e_gender]) {
		fill_bits(mask, OFFSETOF(user, gender), SIZEOF(user, gender));
	}
	if (f.use_filter[e_city_id]) {
		fill_bits(mask, OFFSETOF(user, city_id), SIZEOF(user, city_id));
	}
	if (f.use_filter[e_time_reg]) {
		fill_bits(mask, OFFSETOF(user, time_reg), SIZEOF(user, time_reg));
	}
	return mask;
}

void search(_uint64 &index, _uint64 limit, vector<user> &data, priority_queue<user> &result, vector<_uint64> &cached, filter &f) {
	_uint64 mask = gen_mask_by_filters(f);
	for (; index < cached.size(); index++) {
		if (cmp(cached[index], f.cached_query, mask)) {
			result.push(data[index]);
			if (result.size() == limit) break;
		}
	}
}

void search_wrapper(_uint64 limit, vector<user> &data, priority_queue<user> &result, vector<_uint64> &cached, filter &f) {
	_uint64 index = 0;
	while (index < cached.size()) {
		clock_t t1 = clock();
		search(index, limit, data, result, cached, f);
		clock_t t2 = clock();
		_uint64 size = result.size();
		while (!result.empty()) {
			cout << result.top();
			result.pop();
		}
		float took_time = (float)(t2 - t1);
		cout << "Search took " << took_time / CLOCKS_PER_SEC << " seconds." << endl;
		cout << "Result : " << size << endl;
		if (index < cached.size())
			cout << "goto next page...";
		else
			cout << "done...";
		cin.get();
	}
}

_uint64 next() {
	static _uint64 n;
	return n++;
}

int main() {

	srand(time(NULL));

	filter f;
	f.use_filter[e_birthday] = true;
	f.use_filter[e_city_id] = true;
	f.query.birthday = 25;
	f.query.city_id = 256;
	f.cached_query = pack_user(f.query);

	priority_queue<user> result;
	vector<user> data;
	vector<_uint64> cached;

	for (int i = 0; i < 10000000; i++) {
		user u;
		u.id = next();
		u.birthday = rand() % 365;
		u.gender = rand() % 2;
		u.city_id = rand() % 65535;
		u.time_reg = rand() % 65535;
		data.push_back(u);
		cached.push_back(pack_user(u));
	}

	cout << "Searching...\n" << endl;
	search_wrapper(100, data, result, cached, f);
	return 0;
}
