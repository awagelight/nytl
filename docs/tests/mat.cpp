#include "test.hpp"

#include <nytl/matOps.hpp>
#include <nytl/mat.hpp>
#include <nytl/vecOps.hpp>
#include <nytl/vec.hpp>

#include <array>
#include <type_traits>
#include <limits>
#include <cfloat>
#include <iostream>
#include <stack>

// tests the echolon form operations
void echolon()
{
	{
		nytl::Mat<3, 5, double> a {
			{2.0, 1.0, -1.0, 8.0, 80.0},
		 	{-3.0, -1.0, 2.0, -11.0, -110.0},
		 	{-2.0, 1.0, 2.0, -3.0, -30.0}
		};

		nytl::Mat<3, 5, double> reduced {
			{1.0, 0.0, 0.0, 2.0, 20.0},
		 	{0.0, 1.0, 0.0, 3.0, 30.0},
		 	{0.0, 0.0, 1.0, -1.0, -10.0}
		};

		nytl::mat::reducedRowEcholon(a);
		CHECK_EXPECT(a, reduced);
	}
}

// tests the lu decomposition operations
void lu()
{
	{
		nytl::Mat<3, 3, double> a {
			{2.0, 2.0, 3.0},
			{1.0, 1.0, -1.0},
			{1.0, 0.0, 2.0},
		};

		auto lups = nytl::mat::luDecomp(a);
		const auto& l = std::get<0>(lups);
		const auto& u = std::get<1>(lups);
		const auto& p = std::get<2>(lups);

		CHECK_EXPECT(l * u, p * a);

		auto lups2 = nytl::mat::luDecomp(p * a);
		const auto& l2 = std::get<0>(lups2);
		const auto& u2 = std::get<1>(lups2);

		CHECK_EXPECT(l2 * u2, p * a);
	}

	{
		nytl::Mat<3, 3, double> a {
			{3.0, -.1, -.2},
			{0.1, 7, -.3},
			{.3, -.2, 10}
		};

		nytl::Vec<3, double> b {7.85, -19.3, 71.4};
		nytl::Vec<3, double> x {3.0, -2.5, 7.0};

		auto lups = nytl::mat::luDecomp(a);
		const auto& l = std::get<0>(lups);
		const auto& u = std::get<1>(lups);
		const auto& p = std::get<2>(lups);

		CHECK_EXPECT(l * u, p * a);
		CHECK_EXPECT(nytl::mat::luEvaluate(l, u, b), p * x)
	}
}

// tests the inverse and determinant operations
void inverse()
{
	{
		nytl::Mat<5, 5, double> a {
			{1, -2, 3, 5, 8},
			{0, -1, -1, 2, 3},
			{2, 4, -1, 3, 1},
			{0, 0, 5, 0, 0},
			{1, 3, 0, 4, -1}
		};

		auto lups = nytl::mat::luDecomp(a);
		const auto& l = std::get<0>(lups);
		const auto& u = std::get<1>(lups);
		const auto& p = std::get<2>(lups);
		CHECK_EXPECT(l * u, p * a);

		CHECK_EXPECT(nytl::mat::determinant(a), -135.0);
		CHECK_EXPECT(nytl::mat::invertible(a), true);

		auto inv = nytl::mat::inverse(a);
		auto inv1 = nytl::mat::inverse(l, u);
		CHECK_EXPECT(inv, inv1 * p);

		nytl::Mat<5, 5, double> identity;
		nytl::mat::identity(identity);

		CHECK_EXPECT(a * inv, identity);
		CHECK_EXPECT(inv * a, identity);
	}

	{
		nytl::Mat<5, 5, double> a {
			{1, -2, 3, 5, 8},
			{0, -1, -1, 0, 3},
			{2, 4, -1, 10, 1},
			{0, 0, 5, 0, 0},
			{1, 3, 0, 5, -1}
		};

		auto lups = nytl::mat::luDecomp(a);
		const auto& l = std::get<0>(lups);
		const auto& u = std::get<1>(lups);
		const auto& p = std::get<2>(lups);
		CHECK_EXPECT(l * u, p * a);

		CHECK_EXPECT(nytl::mat::determinant(a), 0.0);
		CHECK_EXPECT(nytl::mat::invertible(a), false);

		CHECK_ERROR(nytl::mat::inverse(a), std::invalid_argument);
		CHECK_ERROR(nytl::mat::inverse(l, u), std::invalid_argument);
	}
}

int main()
{
	echolon();
	lu();
	inverse();

	std::cout << (failed ? std::to_string(failed) : "no") << " tests failed!\n";
}