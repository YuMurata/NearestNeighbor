// LSH.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include"LSH.h"

int main()
{
	LSH::LSHClass<double> obj(4,10,3,2);

	using data = std::vector<double>;

	obj.add(data{ 1, 2, 3 });
	obj.add(data{0, 0, 0});
	obj.add(data{ 1,1,1 });

	auto f=obj.query(data{ 2,2,2 });

    return 0;
}

