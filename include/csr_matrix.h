#pragma once

#include <cstdlib>
#include <iostream>
#include <random>

template <typename ValT = double>
class CSR_matrix {
public:
    size_t n_rows = 0; // число строк
    size_t n_cols = 0; // число столбцов
    size_t nz = 0; // число ненулевых элементов
    size_t capacity = 0; // объем памяти для хранения значений
    size_t* RowIndex = nullptr;
    size_t* Col = nullptr;
    ValT* Val = nullptr;

    CSR_matrix(size_t _m = 1, size_t _n = 1) : n_rows(_m), n_cols(_n) {
        RowIndex = new size_t[n_rows + 1];
    }

    CSR_matrix(size_t _m, size_t _n, size_t _nz) : n_rows(_m), n_cols(_n), nz(_nz) {
        RowIndex = new size_t[n_rows + 1];
        Col = new size_t[nz];
        Val = new ValT[nz]();
        capacity = nz;
    }

    CSR_matrix(const CSR_matrix& copy) : n_rows(copy.n_rows), n_cols(copy.n_cols), nz(copy.nz), capacity(copy.capacity) {
        Col = new size_t[nz];
        memcpy(Col, copy.Col, nz * sizeof(size_t));
        RowIndex = new size_t[n_rows + 1];
        memcpy(RowIndex, copy.RowIndex, (n_rows + 1) * sizeof(size_t));
        if (copy.Val != nullptr) {
            Val = new ValT[nz];
            memcpy(Val, copy.Val, nz * sizeof(ValT));
        }
    }

    CSR_matrix(CSR_matrix&& mov) : n_rows(mov.n_rows), n_cols(mov.n_cols), nz(mov.nz), capacity(mov.capacity) {
        Col = mov.Col;
        RowIndex = mov.RowIndex;
        Val = mov.Val;

        mov.Col = nullptr;
        mov.RowIndex = nullptr;
        mov.Val = nullptr;
    }

    ~CSR_matrix() {
        delete[] Col;
        delete[] RowIndex;
        delete[] Val;
    }

    CSR_matrix& operator=(const CSR_matrix& copy) {
        if (this == &copy)
            return *this;

        if (n_rows != copy.n_rows) {
            delete[] RowIndex;
            RowIndex = new size_t[copy.n_rows + 1];
        }
        memcpy(RowIndex, copy.RowIndex, (copy.n_rows + 1) * sizeof(size_t));
        if (capacity < copy.nz) {
            delete[] Col;
            delete[] Val;

            Col = new size_t[copy.nz];
            Val = new ValT[copy.nz];
            capacity = copy.nz;
        }
        memcpy(Col, copy.Col, copy.nz * sizeof(size_t));
        memcpy(Val, copy.Val, copy.nz * sizeof(ValT));

        n_rows = copy.n_rows;
        n_cols = copy.n_cols;
        nz = copy.nz;

        return *this;
    }

    CSR_matrix& operator=(CSR_matrix&& mov) {
        if (this == &mov)
            return *this;

        Col = mov.Col;
        RowIndex = mov.RowIndex;
        Val = mov.Val;

        mov.Col = nullptr;
        mov.RowIndex = nullptr;
        mov.Val = nullptr;

        n_rows = mov.n_rows;
        n_cols = mov.n_cols;
        nz = mov.nz;
        capacity = mov.capacity;


        return *this;
    }
    
    // вектора cols, rows, vals содержат корректное описание матрицы в формате CSR
    void set_from_vectors(const std::vector<size_t>& cols, const std::vector<size_t>& rows, 
        const std::vector<ValT>& vals) {

        size_t new_nz = cols.size();
        if (nz != new_nz) {
            delete[] Col;
            delete[] Val;
            nz = new_nz;
            Col = new size_t[new_nz];
            Val = new ValT[new_nz];
        }
		if (n_rows != rows.size() - 1) {
			delete[] RowIndex;
			RowIndex = new size_t[n_rows + 1];
            n_rows = rows.size() - 1;
			n_cols = std::max(*std::max_element(cols.begin(), cols.end()) + 1, n_rows);
		}
		std::copy(cols.begin(), cols.end(), Col);
		std::copy(vals.begin(), vals.end(), Val);
        std::copy(rows.begin(), rows.end(), RowIndex);
    }

    void generate_matrix(const size_t avg_degree, ValT minVal, ValT maxVal) {
        size_t new_nz = n_rows * avg_degree;
        if (avg_degree > n_cols) {
            throw std::invalid_argument("avg_degree cannot be greater than the number of columns");
        }

        if (nz != new_nz) {
            delete[] Col;
            delete[] Val;

            nz = new_nz;
            capacity = nz;

            Col = new size_t[nz];
            Val = new ValT[nz];
        }

        std::random_device rd;
        std::mt19937 gen(rd());

        using DistT = std::conditional_t<std::is_floating_point_v<ValT>,
            std::uniform_real_distribution<ValT>,
            std::uniform_int_distribution<ValT>>;
        DistT val_dist(minVal, maxVal);

        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
        double p = static_cast<double>(avg_degree) / n_cols;

        size_t current_elem = 0;
        RowIndex[0] = 0;

        for (size_t i = 0; i < n_rows; ++i) {
            for (size_t j = 0; j < n_cols && current_elem < nz; ++j) {
                if (prob_dist(gen) < p) {
                    Col[current_elem] = j;
                    Val[current_elem] = val_dist(gen);
                    current_elem++;
                }
            }
            RowIndex[i + 1] = current_elem;
        }
        nz = RowIndex[n_rows];
        capacity = nz;
    }


	void print_coo() const {
		std::cout << "row  col val\n";
		for (size_t i = 0; i < n_rows; ++i)
			for (size_t j = RowIndex[i]; j < RowIndex[i + 1]; ++j)
				std::cout << i << " " << Col[j] << " " << Val[j] << '\n';
	}


    void print_dense() const {
		for (size_t i = 0; i < n_rows; ++i) {
			size_t k = 0;
			for (size_t j = RowIndex[i]; j < RowIndex[i + 1]; ++j, ++k) {
				while (k < Col[j]) {
					std::cout << 0 << " ";
					++k;
				}
				std::cout << Val[j] << " ";
			}
			while (k < n_cols) {
				std::cout << 0 << " ";
				++k;
			}
			std::cout << '\n';
		}
		std::cout << '\n';

    }

    // TODO:
	// сложение матриц в формате CSR
    CSR_matrix operator+(const CSR_matrix& other) const;
	// умножение матрицы на вектор
    std::vector<ValT> operator*(const std::vector<ValT>& x) const;
	// транспонирование матрицы в формате CSR
	CSR_matrix transpose() const;
	// конвертер из координатного формата в формат CSR
	CSR_matrix from_coo(size_t n_rows, size_t n_cols, const std::vector<size_t>& rows, const std::vector<size_t>& cols, const std::vector<ValT>& vals);
	
	// подумайте, как реализовать корректно для разных типов данных ValT
    bool operator==(const CSR_matrix& other) const;
};


