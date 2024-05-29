#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <future>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <ctime>
using namespace std;

template <typename T>
class Matrix {
private:
    string name;
    int rows;
    int columns;
    vector<vector<T>> matr;
    ifstream FileMatrix;
public:
    Matrix() {
        rows = 0;
        columns = 0;
    }
    Matrix(string Name, int Rows, int Columns, vector<vector<T>> myVector) : name(Name), rows(Rows), columns(Columns), matr(myVector){}
    Matrix(string Name, int Rows, int Columns) : name(Name), rows(Rows), columns(Columns) {
        matr.resize(rows, vector<T>(columns));
    }
    Matrix (string Name, int Rows, int Columns, string fname) : name(Name), rows(Rows), columns(Columns) {
        matr.resize(rows, vector<T>(columns));
        inputfile(fname);
    }
    //Конструктор копирования
    Matrix(const Matrix &obj) {
        rows = obj.rows;
        columns = obj.columns;
        matr = obj.matr;
    }
    
    void setValue(int row, int column, T value) {
        matr[row][column] = value;
    }
    
    // Функция inputfile(string fname) записывает в матрицу данные из файла
    void inputfile(string fname) {
        FileMatrix.open(fname);
        if (FileMatrix.is_open()) {
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < columns; j++) {
                    FileMatrix >> matr[i][j];
                }
            }
        }
    }
    // Функция outputfile(string fname) записывает в файл матрицу
    void outputfile(string fname) {
        ofstream file;
        file.open(fname);
        if (file.is_open()) {
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < columns; j++) {
                    file << matr[i][j] << " ";
                }
                file << endl;
            }
        }
        file.close();
    }
    //Деструктор
    ~Matrix() {
        FileMatrix.close();
    }
    
    // Для ввода матрицы из консоли
    void input(istream &is) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                is >> matr[i][j];
            }
        }
    }
    // Для вывода матрицы в консоль
    void output(ostream &os) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < columns; ++j) {
                os << matr[i][j] << " ";
            }
            os << endl;
        }
    }
    // Перегрузка оператора +
    Matrix operator +(const Matrix & other) {
        if (rows != other.rows || columns != other.columns) {
            throw "Введены матрицы разного размера, сложение невозможно";
            }
        Matrix sum("sum", rows, columns);
        vector<thread> threads;
        for (int i = 0; i < rows; i++) {
            threads.emplace_back([this, &other, i, &sum] {
                for (int j = 0; j < columns; j++) {
                    sum.matr[i][j] = matr[i][j] + other.matr[i][j];
                }
            });
        }
        for (auto &thread : threads) {
            thread.join();
        }
        return sum;
    }
    
    // Сложение, используя фьючерс
    Matrix blockSum(const Matrix & other, int blockSize) {
        if (rows != other.rows || columns != other.columns) {
            throw "Введены матрицы разного размера, сложение невозможно";
        }
        Matrix sum("sum", rows, columns);
        vector<future<void>> futures;
        for (int i = 0; i < rows; i += blockSize) {
            for (int j = 0; j < columns; j += blockSize) {
                futures.push_back(async(launch::async, [i, j, blockSize, this, &other, &sum] {
                    for (int indexI = i; indexI < min(i+blockSize, rows); indexI++) {
                        for (int indexJ = j; indexJ < min(j+blockSize, columns); indexJ++) {
                            sum.matr[indexI][indexJ] = matr[indexI][indexJ] + other.matr[indexI][indexJ];
                        }
                    }
                }));
            }
        }
        for (auto &future : futures) {
            future.get();
        }
        return sum;
    }
    
    // Перегрузка оператора -
    Matrix operator -(const Matrix & other) {
        if (rows != other.rows || columns != other.columns) {
            throw "Введены матрицы разного размера, вычитание невозможно";
        }
        Matrix dif("dif", rows, columns);
        vector <thread> threads;
        for (int i = 0; i < rows; i++) {
            threads.emplace_back([this, &other, i, &dif] {
                for (int j = 0; j < columns; j++) {
                    dif.matr[i][j] = matr[i][j] - other.matr[i][j];
                }
            });
        }
        for (auto &thread : threads) {
            thread.join();
        }
        return dif;
    }
    
    // Вычитание, используя фьючерс
    Matrix blockDif(const Matrix & other, int blockSize) {
        if (rows != other.rows || columns != other.columns) {
            throw "Введены матрицы разного размера, вычитание невозможно";
        }
        Matrix dif("dif", rows, columns);
        vector<future<void>> futures;
        for (int i = 0; i < rows; i += blockSize) {
            for (int j = 0; j < columns; j += blockSize) {
                futures.emplace_back(async(launch::async, [i, j, this, &dif, &other, blockSize] {
                    for (int indexI = i; indexI < min(i+blockSize, rows); indexI++) {
                        for (int indexJ = j; indexJ < min(j+blockSize, columns); indexJ++) {
                            dif.matr[indexI][indexJ] = matr[indexI][indexJ] - other.matr[indexI][indexJ];
                        }
                    }
                }));
            }
        }
        for (auto &future : futures) {
            future.get();
        }
        return dif;
    }
    
    // Перегрузка оператора * для умножения матриц
    Matrix operator *(const Matrix & other) {
        if (columns != other.rows) {
            throw "Такие матрицы нельзя перемножить, так как количество столбцов первой матрицы не равно количеству строк второй матрицы";
        }
        Matrix multi("multi", rows, other.columns);
        vector<thread> threads;
            for (int i = 0; i < rows; i++) {
                threads.emplace_back([this, &other, i, &multi] {
                    for (int j = 0; j < other.columns; j++) {
                        T summa = 0;
                        for (int k = 0; k < columns; k++) {
                            summa += matr[i][k] * other.matr[k][j];
                        }
                        multi.matr[i][j] = summa;
                        summa = 0;
                    }
                });
            }
        for (auto &thread : threads) {
            thread.join();
        }
        return multi;
    }
    
    // Умножение на матрицу, используя фьючерс
    Matrix blockMulti(const Matrix & other, int blockSize) {
        if (columns != other.rows) {
            throw "Такие матрицы нельзя перемножить, так как количество столбцов первой матрицы не равно количеству строк второй матрицы";
        }
        Matrix multi("multi", rows, other.columns);
        vector<future<void>> futures;
        for (int i = 0; i < rows; i += blockSize) {
            for (int j = 0; j < other.columns; j += blockSize) {
                futures.emplace_back(async(launch::async, [i, j, this, &other, &multi, blockSize] {
                    for (int indexI = i; indexI < min(i+blockSize, rows); indexI++) {
                        for (int indexJ = j; indexJ < min(j+blockSize, other.columns); indexJ++) {
                            T summa = 0;
                            for (int k = 0; k < columns; k++) {
                                summa += matr[indexI][k] * other.matr[k][indexJ];
                            }
                            multi.matr[indexI][indexJ] = summa;
                            summa = 0;
                        }
                    }
                }));
            }
        }
        for (auto &future : futures) {
            future.get();
        }
        return multi;
    }
    
    //Перегрузка оператора * для умножения матрицы на число
    Matrix operator *(T sc) {
        Matrix multi("multi", rows, columns);
        vector <thread> threads;
        for (int i = 0; i < rows; i++) {
            threads.emplace_back([this, sc, i, &multi] {
                for (int j = 0; j < columns; j++) {
                    multi.matr[i][j] = sc * matr[i][j];
                }
            });
        }
        for (auto &thread : threads) {
            thread.join();
        }
        return multi;
    }
    
    // Умножение на число, используя фьючерс
    Matrix blockMultiSc (T sc, int blockSize) {
        Matrix multi("multi", rows, columns);
        vector<future<void>> futures;
        for (int i = 0; i < rows; i += blockSize) {
            for (int j = 0; j < columns; j += blockSize) {
                futures.emplace_back(async(launch::async, [sc, i, j, this, &multi, blockSize] {
                    for (int indexI = i; indexI < min(i+blockSize, rows); indexI++) {
                        for (int indexJ = j; indexJ < min(j+blockSize, columns); indexJ++) {
                            multi.matr[indexI][indexJ] = sc * matr[indexI][indexJ];
                        }
                    }
                }));
            }
        }
        for (auto &future : futures) {
            future.get();
        }
        return multi;
    }
    
    //Перегрузка оператора присваивания
    Matrix& operator=(const Matrix &m) {
        rows = m.rows;
        columns = m.columns;
        matr = m.matr;
        return *this;
    }
    
    // Перегрузка оператора ввода >>
    friend istream& operator>>(istream &is, Matrix &m) {
        m.input(is);
        return is;
    }
    // Перегрузка оператора вывода <<
    friend ostream& operator<<(ostream &os, Matrix &m) {
        m.output(os);
        return os;
    }
    //Алгебраическое дополнение
    Matrix getMinor(int i, int j) {
        i--;
        j--;
        Matrix matrix(name, rows-1, columns-1);
        int k = 0;
        int f = 0;
        vector<thread> threads1;
        for (int indexI = 0; indexI < rows; indexI++) {
            if (indexI == i) {
                k--;
                continue;
            }
            threads1.emplace_back([this, indexI, j, &matrix, f, k] () mutable {
                for (int indexJ = 0; indexJ < columns; indexJ++) {
                    if (indexJ == j) {
                        f--;
                        continue;
                    }
                    matrix.matr[indexI+k][indexJ+f] = matr[indexI][indexJ];
                }
            });
        }
        for (auto &thread : threads1) {
            thread.join();
        }
        return matrix;
    }
      
    // Определитель
    double getDet() {
        if (rows != columns) {
            throw "Невозможно найти определитель матрицы, так как она не квадратная";
        }
            
        if (rows == 1) {
            return matr[0][0];
        }
        if (rows == 2) {
            return matr[0][0]*matr[1][1]  - matr[0][1]*matr[1][0];
        }
        double res = 0.0;
        vector<thread> threads;
        for (int j = 0; j < columns; j++) {
            threads.emplace_back([this, &res, j] {
                Matrix M = getMinor(1, j + 1);
                res += (j % 2 == 0 ? 1 : -1) * M.getDet() * matr[0][j];
            });
        }
        for (auto &thread : threads) {
            thread.join();
        }
        return res;
    }
    
    // Определитель, используя фьючерсы
    double blockGetDet(int blockSize) {
        if (rows != columns) {
            throw "Невозможно найти определитель матрицы, так как она не квадратная";
        }
        if (rows == 1) {
            return matr[0][0];
        }
        if (rows == 2) {
            return matr[0][0]*matr[1][1]  - matr[0][1]*matr[1][0];
        }
        double res = 0.0;
        vector<future<void>> futures;
        for (int j = 0; j < columns; j += blockSize) {
            futures.push_back(async(launch::async, [&, blockSize, j] {
                for (int indexJ = j; indexJ < min(j+blockSize, columns); indexJ++) {
                    Matrix M = getMinor(1, indexJ + 1);
                    res += (indexJ % 2 == 0 ? 1 : -1) * M.blockGetDet(blockSize)*matr[0][indexJ];
                }
            }));
        }
        for (auto &future : futures) {
            future.get();
        }
        return res;
    }
    
    //Транспонирование
    Matrix transpose() {
        Matrix transpose("transpose", columns, rows);
        vector <thread> threads;
        for (int i = 0; i < rows; i++) {
            threads.emplace_back([this, i, &transpose] {
                for (int j = 0; j < columns; j++) {
                    transpose.matr[j][i] = matr[i][j];
                }
            });
        }
        for (auto &thread : threads) {
            thread.join();
        }
        return transpose;
    }
    
    // Транспонирование, используя фьючерсы
    Matrix blockTranspose(int blockSize) {
        Matrix transpose("transpose", columns, rows);
        vector<future<void>> futures;
        for (int i = 0; i < rows; i += blockSize) {
            for (int j = 0; j < columns; j += blockSize) {
                futures.emplace_back(async(launch::async, [=, this, &transpose] {
                    for (int indexI = i; indexI < min(i+blockSize, rows); indexI++) {
                        for (int indexJ = j; indexJ < min(j+blockSize, columns); indexJ++) {
                            transpose.matr[indexJ][indexI] = matr[indexI][indexJ];
                        }
                    }
                }));
            }
        }
        for (auto &future : futures) {
            future.get();
        }
        return transpose;
    }
    
    //Поиск обратной матрицы
    Matrix operator!() {
        Matrix current("curr", rows, columns);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++) {
                current.matr[i][j] = matr[i][j];
            }
        }
        if (columns != rows || current.getDet() == 0) {
            throw "Матрица должна быть квадратной и невырожденной";
        }
        Matrix minor("minor", rows, columns);
        if (rows == 1 && columns == 1) {
            current.matr[0][0] = 1/current.matr[0][0];
            return current;
        }
        vector<thread> threads;
        if (rows > 1) {
            for (int i = 0; i < rows; i++) {
                threads.emplace_back([i, this, &minor] {
                    for (int j = 0; j < columns; j++) {
                        minor.matr[i][j] = ((i + j) % 2 == 0? 1 : -1) * getMinor(i + 1, j + 1).getDet();
                    }
                });
            }
            for (auto &thread : threads) {
                thread.join();
            }
        }
        Matrix inverse = minor.transpose() * (1 / current.getDet());
        return inverse;
    }
    
    // Поиск обратной матрицы, используя фьючерсы
    Matrix blockInverce(int blockSize) {
        Matrix current("curr", rows, columns);
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++) {
                current.matr[i][j] = matr[i][j];
            }
        }
        if (columns != rows || current.blockGetDet(blockSize) == 0) {
            throw "Матрица должна быть квадратной и невырожденной";
        }
        Matrix minor("minor", rows, columns);
        if (rows == 1 && columns == 1) {
            current.matr[0][0] = 1/current.matr[0][0];
            return current;
        }
        vector<future<void>> futures;
        for (int i = 0; i < rows; i += blockSize) {
            futures.emplace_back(async(launch::async, [this, i, blockSize, &minor] {
                for (int indexI = i; indexI < min(i+blockSize, rows); indexI++) {
                    for (int indexJ = 0; indexJ < columns; indexJ++) {
                        minor.matr[indexI][indexJ] = ((indexI + indexJ) % 2 == 0? 1 : -1) * getMinor(indexI + 1, indexJ + 1).blockGetDet(blockSize);
                    }
                }
            }));
        }
        for (auto &future : futures) {
            future.get();
        }
        Matrix inverse = minor.blockTranspose(blockSize) * (1 / current.blockGetDet(blockSize));
        return inverse;
    }
    
    //Нулевая матрица заданного размера
    static Matrix zero(int Rows, int Columns, int blockSize) {
        Matrix zero("zero", Rows, Columns);
        vector<future<void>> futures;
        for (int i = 0; i < Rows; i += blockSize) {
            for (int j = 0; j < Columns; j += blockSize) {
                futures.emplace_back(async(launch::async, [=, &zero] {
                    for (int indexI = i; indexI < min(i+blockSize, Rows); indexI++) {
                        for (int indexJ = j; indexJ < min(j+blockSize, Columns); indexJ++) {
                            zero.matr[indexI][indexJ] = 0;
                        }
                    }
                }));
            }
        }
        for (auto &future : futures) {
            future.get();
        }
        return zero;
    }
    //Единичная матрица заданного размера
    static Matrix id(int Rows, int Columns, int blockSize) {
        Matrix id("id", Rows, Columns);
        vector<future<void>> futures;
        for (int i = 0; i < Rows; i += blockSize) {
            for (int j = 0; j < Columns; j += blockSize) {
                futures.emplace_back(async(launch::async, [=, &id] {
                    for (int indexI = i; indexI < min(i+blockSize, Rows); indexI++) {
                        for (int indexJ = j; indexJ < min(j+blockSize, Columns); indexJ++) {
                            if (indexI == indexJ) {
                                id.matr[indexI][indexJ] = 1;
                            }
                            else {
                                id.matr[indexI][indexJ] = 0;
                            }
                        }
                    }
                }));
            }
        }
        for (auto &future : futures) {
            future.get();
        }
        return id;
    }
};
    
int main() {
    srand(static_cast<unsigned int> (time(NULL)));
    int blockSize = 0;
    /*for (int i = 0; i <= 1500; i += 50) {
        Matrix<double> A("A", i, i), B("B", i, i);
        for (int indexI = 0; indexI < i; indexI++) {
            for (int indexJ = 0; indexJ < i; indexJ++) {
                A.setValue(indexI, indexJ, rand()%100+1);
                B.setValue(indexI, indexJ, rand()%100+1);
            }
        }
        auto start = chrono::high_resolution_clock::now();
        Matrix C = A.blockMulti(B, blockSize);
        auto end = chrono::high_resolution_clock::now();
        auto duration = end - start;
        auto iMillis = chrono::duration_cast<chrono::milliseconds>(duration);
        cout << "Размер матриц: " << i << " * " << i << ". Время выполнения: " << iMillis.count() << endl;
        blockSize += 10;
    }*/
    /*for (int i = 2; i <= 8; i += 2) {
        blockSize = i/2;
        Matrix<double> A("A", i, i);
        for (int indexI = 0; indexI < i; indexI++) {
            for (int indexJ = 0; indexJ < i; indexJ++) {
                A.setValue(indexI, indexJ, rand()%100+1);
            }
        }
        
        try {
            auto start = chrono::high_resolution_clock::now();
            Matrix C = A.blockInverce(blockSize);
            auto end = chrono::high_resolution_clock::now();
            auto duration = end - start;
            auto iMillis = chrono::duration_cast<chrono::milliseconds>(duration);
            cout << "Размер матриц: " << i << " * " << i << ". Время выполнения: " << iMillis.count() << endl;
        }
        catch (const char* errorMessage) {
            cout << errorMessage << endl;
        }
        blockSize += 1;
    }*/
    /*Matrix<double> A("A",6,6);
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            A.setValue(i, j, rand()%100+1);
            
        }
    }
    auto start = chrono::high_resolution_clock::now();
    Matrix C = !A;
    auto end = chrono::high_resolution_clock::now();
    auto duration = end - start;
    auto iMillis = chrono::duration_cast<chrono::milliseconds>(duration);
    cout << iMillis.count() << endl;*/
    /*int rows, columns;
    cout << "Введите количество строк матрицы А:" << endl;
    cin >> rows;
    cout << "Введите количество столбцов матрицы А:" << endl;
    cin >> columns;
    Matrix<double> A("A", rows, columns);
    cout << "Введите матрицу A: " << endl;
    cin >> A;
    auto start = chrono::high_resolution_clock::now();
    Matrix C = A.blockInverce(2);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<float> duration = end-start;
    auto start1 = chrono::high_resolution_clock::now();
    Matrix C1 = !A;
    auto end1 = chrono::high_resolution_clock::now();
    auto dur1 = end1 - start1;
    auto i_millis = std::chrono::duration_cast<std::chrono::milliseconds>(dur1);
    cout << duration.count() << " " << i_millis.count() << endl;*/
    return 0;
}
