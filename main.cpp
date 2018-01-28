#include <cassert>
#include <iostream>
#include <tuple>

#include "matrix.h"


int main()
{
    Matrix<int> matrix(-1);
    assert( matrix.size() == 0 );

    auto a = matrix[0][0];
    assert( a == -1 );

    assert( matrix.size() == 0 );
    matrix[100][100] = 314L;

    assert( matrix[100][100] == 314 );
    assert( matrix.size() == 1 );


    Matrix<int> mat( 0 );
    for( size_t i = 0; i < 10; ++i )
    {
        mat[i][i] = i;
        mat[i][9 - i] = i;
    }

    for( size_t i = 1; i < 9; ++i )
    {
        for( size_t j = 1; j < 9; ++j )
        {
            std::cout << mat[i][j] << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << "Mat size = " << mat.size() << '\n';

    for( auto c : mat )
    {
        int x, y, v;
        std::tie( x, y, v ) = c;
        std::cout << "[ " << x << " ][ " << y << " ] = " << v << std::endl;
    }

    return 0;
}