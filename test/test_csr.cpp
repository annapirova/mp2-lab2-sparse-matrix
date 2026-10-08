#include <gtest/gtest.h>
#include "csr_matrix.h"


TEST(TestMatrix, can_create_matrix)
{
  ASSERT_NO_THROW(CSR_matrix m(10, 10, 20));
}


TEST(TestMatrix, can_generate_matrix)
{
	CSR_matrix<double> m(10, 10);
	ASSERT_NO_THROW(m.generate_matrix(3, 0.5, 2.5));
	//m.print_coo();
}

TEST(TestMatrix, can_set_from_vectors)
{
	CSR_matrix m(5, 5);
	std::vector<size_t> cols = {0, 2, 4, 1, 3};
	std::vector<size_t> rows = {0, 2, 2, 4, 5};
	std::vector<double> vals = {1.0, 2.0, 3.0, 4.0, 5.0};
	m.set_from_vectors(cols, rows, vals);
	//m.print_coo();
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}