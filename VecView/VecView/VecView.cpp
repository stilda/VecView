#include "VecView.h"
#include <iostream>
#include <cassert>

namespace vevi
{
	using namespace std;

	void compile_usage()
	{
		int n = Num(2) + Num(3);
		assert(n == 5);

		int v1[] = { 1, 2 };
		int v2[] = { 3, 4 };
		int v[] = { 0, 0 };
		AVec(v, 2) = Vec(v1) + Vec(v2);
		assert(v[0] == 4 && v[1] == 6);

		int m = Dot(Vec(v1), Vec(v2, 2));
		assert(m == 11);
	}

	bool test_bin_add()
	{
		int v1[2] = { 1, 2 };
		int v2[2] = { 3, 4 };
		int v3[2] = { 0 };

		AVec(v3, 2) = Vec(v1) + Vec(v2);
		assert(v3[0] == 4 && v3[1] == 6);
		return true;
	}

	bool test_una_neg()
	{
		int v1[2] = { 1, 2 };

		AVec(v1, 2) = -Vec(v1);
		assert(v1[0] == -1 && v1[1] == -2);
		return true;
	}

	bool test_bin_add_chains()
	{
		int v1[2] = {1, 2};
		int v2[2] = { 3, 4 };
		int v3[2] = { 5, 6 };
		int v4[2] = { 0 };

		AVec(v4, 2) = Vec(v1) + Vec(v2) + Vec(v3);
		assert(v4[0] == 9 && v4[1] == 12);
		return true;
	}

	bool test_bin_sub_chains()
	{
		int v1[2] = { 1, 2 };
		int v2[2] = { 3, 4 };
		int v3[2] = { 5, 6 };
		int v4[2] = { 0 };

		AVec(v4, 2) = Vec(v1) - Vec(v2) - Vec(v3);
		assert(v4[0] == -7 && v4[1] == -8);
		return true;
	}

	bool test_dot_prod()
	{
		int v1[2] = { 1, 2 };
		int v2[2] = { 3, 4 };

		int dp = Dot(Vec(v1, 2), Vec(v2));
		assert(dp == 11);
		return true;
	}

	bool test_dot_prod_for_chains()
	{
		int v1[2] = { 1, 2 };
		int v2[2] = { 3, 4 };
		int v3[2] = { 5, 6 };

		int dp = Dot(Vec(v1) + Vec(v2), Vec(v3, 2));
		assert(dp == 56);
		int dp2 = Dot(Vec(v1), Vec(v3, 2)) + Dot(Vec(v1, 2), Vec(v3));
		assert(dp2 == 34);
		return true;
	}

	bool test_scalar_op()
	{
		int v1[2] = { 1, 2 };
		int v2[2] = { 3, 4 };

		//int p = Num(2) + 3;
		AVec(v2, 2) = Num(3) + Vec(v1);
		assert(v2[0] == 4 && v2[1] == 5);
		AVec(v2, 2) = Num(3);
		assert(v2[0] == 3 && v2[1] == 3);
		AVec(v2, 2) = Dot(Num(3) + Vec(v1, 2), Num(3) + Vec(v1, 2));
		assert(v2[0] == 41 && v2[1] == 41);
		//assert(r == 9);
		return true;
	}

	bool test_nodim_vectors()
	{
		int v1[2] = { 1, 2 };
		int v2[2] = { 1, 2 };

		AVec(v1, 2) = -Vec(v1);
		assert(v1[0] == -1 && v1[1] == -2);

		AVec(v2, 2) = Vec(v1) + Vec(v2);
		assert(v2[0] == 0 && v2[1] == 0);

		int p = Dot(Vec(v2) + Vec(v1), Vec(v1) + Vec(v2,2));
		assert(p == 5);

		return true;
	}

	bool test_cast()
	{
		double v1[] = { 0.7, 2.3 };
		double v2[] = { 0, 0 };

		AVec(v2, 2) = Cast<int>(Vec(v1));
		assert(abs(v2[0]) < 1e-9 && abs(v2[1] - 2.0) < 1e-9);

		return true;
	}

	bool tests()
	{
		compile_usage();
		test_bin_add();
		test_una_neg();
		test_bin_add_chains();
		test_bin_sub_chains();
		test_dot_prod();
		test_dot_prod_for_chains();
		test_scalar_op();
		test_nodim_vectors();
		test_cast();

		//std::cout << vevi::Operations::HasMemberDim<Operations::NumberView<int>>::value;
		//std::cout << vevi::Operations::HasMemberDim<Operations::VectorView<int>>::value;
		return true;
	}
}