#pragma once

#include <iostream>
#include <array>
#include <algorithm>
#include <numeric>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include<DataBase.h>
#include<typeinfo>
#include<Randomer.h>

namespace LSH {
	using namespace std;

	// hash cell -> http://www.mit.edu/~andoni/LSH/manual.pdf section 3.5.2
	struct lshHash 
	{
		int h1, h2;
		bool operator ==(lshHash const& lsh) const 
		{
			return h1 == lsh.h1 && h2 == lsh.h2;
		}
	};

	// h(x)
	template<typename T>
	struct H :public Randomer
	{
	private:
		const int d;
		const int r;

		vector<T> a;
		double b;

	public:
		H(const int &d,const int &r) :
			d(d),r(r),a(d)
		{
			this->b = rand<uniform_real_distribution<>>(0, r);

			for (auto& value : this->a)
			{
				value = this->rand<normal_distribution<double>>(0, 1);
			}
		}

		int operator () (vector<T> const& x) const {
			return static_cast<int>((inner_product(x.begin(), x.end(), a.begin(), 0) + b) / r);
		}
	};

	// g(x) = [h1(x),h2(x), ....hk(x)]
	template<typename T>
	struct G:public Randomer
	{
	private:
		const int d;
		const int k;
		const int r;

		vector<H<T>> h;
		vector<int> r1;
		vector<int> r2;

	public:
		G(const int &d,const int &k,const int &r)
			:d(d),k(k),r(r),
			h(k,H<T>(d,r)),r1(k),r2(k)
		{
			for (int i = 0; i < k; i++)
			{
				r1[i] = this->rand<uniform_int_distribution<int>>();
				r2[i] = this->rand<uniform_int_distribution<int>>();
			}
		}

		lshHash operator ()(vector<T> const& x) const
		{
			lshHash hash = {};
			vector<int> a(this->k);

			for (int i = 0; i < k; i++)
			{
				a[i] = h[i](x);
			}

			hash.h1 = inner_product(a.begin(), a.end(), r1.begin(), 0);
			hash.h2 = inner_product(a.begin(), a.end(), r2.begin(), 0);
			return hash;
		}
	};

	template<typename DataType>
	class LSHClass:public DataBase<vector<vector<DataType>>>
	{
	private:
		const int K;
		const int L;
		const int d;
		const int r;

		vector<G<DataType>> g; // Hash-Familiy
		vector<unordered_multimap<lshHash, int>> hash_tables;

	public:

	//K=ハッシュ長
	//L=バケット数
	//d=次元数
	//r=hash_paramater
		LSHClass(const int &d,const int &k=4, const int &l=10,  const int &r=2)
			:K(k), L(l), d(d), r(r) ,
			g(l,G<DataType>(d,k,r)),hash_tables(l){}

		using DataList = vector<DataType>;

		void add(const DataList &val) 
		{
			this->data.push_back(val);
			auto& v = this->data.back();

			// L個のハッシュテーブルに値を格納
			for (int i = 0; i < L; i++)
			{
				hash_tables[i].insert(make_pair(g[i](v), size(this->data) - 1));
			}
		}

		unordered_set<const DataList*> query(const DataList &query_data) const 
		{
			unordered_set<const DataList*> result;

			for (int i = 0; i < L; i++) 
			{
				auto h = g[i](query_data);
				auto range = hash_tables[i].equal_range(h);
				
				while (range.first != range.second) 
				{
					result.insert(&this->data[range.first->second]);
					++range.first;
					
					if (result.size() >= 2 * L)
					{
						return result;
					}
				}
			}
			return result;
		}

		bool LoadFile(const string &file_name)override
		{
			auto data_list = this->PreLoad(file_name);
		
			if (size(data_list[0]) != d)
			{
				return false;
			}

			auto func = [](const vector<string> &strs)
			{
				DataList ret(size(strs));
				transform(begin(strs), end(strs), begin(ret), [](const string &str) {return stod(str); });

				return ret;
			};

			transform(begin(data_list), end(data_list), begin(this->data),func);

			return true;
		}

		bool WriteFile(const string &file_name)
		{
			vector<vector<string>> data_list(size(this->data));

			auto func = [](const DataList &x)
			{
				vector<string> ret(size(x));
				transform(begin(x), end(x), begin(ret), [](const DataType &t) {return to_string(t); });

				return ret;
			};

			transform(begin(this->data), end(this->data), begin(data_list), func);

			return this->PreWrite(file_name,data_list);
		}
	};
}

namespace std {
	// unordered_map使うための部分特殊化
	template<>
	struct hash<LSH::lshHash> {
		size_t operator()(LSH::lshHash const& h) const {
			return h.h1;
		}
	};
}