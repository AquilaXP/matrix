#include <cassert>

#include <tuple>
#include <type_traits>
#include <array>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <vector>
#include <map>

template<class T>
using ColumnT_t = std::map<size_t, T>;
template<class T>
using RowT_t = std::map<size_t, ColumnT_t<T>>;

template<class T>
using MatrixT_t = std::map<size_t, ColumnT_t<T>>;


template<class T>
struct iterator_matrix
{
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename std::tuple<size_t,size_t, T&>;

    iterator_matrix() = default;
    iterator_matrix( const iterator_matrix& rhs ) = default;
    iterator_matrix( MatrixT_t<T>& data, typename MatrixT_t<T>::iterator m, typename ColumnT_t<T>::iterator c )
        : data( data ), m(m), c(c)
    {
    }

    value_type operator*()
    {
        return std::make_tuple( m->first, c->first,  std::ref(c->second) );
    }
    value_type operator->()
    {
        return std::make_tuple( m->first, c->first, std::ref( c->second ) );
    }
    iterator_matrix& operator++()
    {
        ++c;
        if( c == m->second.end() )
        {
            ++m;
            if( m != data.end() )
            {
                c = m->second.begin();
            }
            else
            {
                c = typename ColumnT_t<T>::iterator();
            }
        }

        return ( *this );
    }

    iterator_matrix operator++( int )
    {
        iterator_matrix it( data, m, c );
        ++( *this );
        return it;
    }

    /*iterator_matrix& operator--()
    {        
        ++c;
        if( c == m->second.end() )
        {
            ++m;
            if( m != data.end() )
            {
                c = m->second.begin();
            }
        }
        return ( *this );
    }

    iterator_matrix operator--( int )
    {
        iterator_matrix it( ptr );
        --( *this );
        return it;
    }*/

    bool operator == ( const iterator_matrix& rhs ) const
    {
        return m == rhs.m && data.end() != m && c == rhs.c;
    }

    bool operator != ( const iterator_matrix& rhs ) const
    {
        return !((*this) == rhs);
    }

private:
    MatrixT_t<T>& data;
    typename MatrixT_t<T>::iterator m;
    typename ColumnT_t<T>::iterator c;
};

template<class T>
class ProxyValue
{
public:
    ProxyValue( MatrixT_t<T>& data, size_t nRow, size_t nColumn, const T& default, size_t& size ) :
        m_data(data), m_default(default), m_Row(nRow), m_Column(nColumn), m_Size(size)
    {
    }

    ProxyValue& operator = ( const T& value )
    {
        if( value == m_default )
        {
            auto row = m_data.find( m_Row );
            if( row != m_data.end() )
            {
                auto column = row->second.find( m_Column );
                if( column != row->second.end() )
                {
                    row->second.erase( column );
                    --m_Size;
                }
                if( row->second.empty() )
                {
                    m_data.erase( row );
                }
            }
        }
        else
        {            
            auto& col = m_data[m_Row];
            auto prevSize = col.size();
            col[m_Column] = value;
            m_Size += col.size() - prevSize;
        }
        return ( *this );
    }

    operator T()
    {        
        auto row = m_data.find( m_Row );
        if( row != m_data.end() )
        {
            auto column = row->second.find( m_Column );
            if( column != row->second.end() )
            {
                return ( column->second );                
            }
        }
        return m_default;
    }

private:
    MatrixT_t<T>& m_data;
    const T& m_default;
    size_t& m_Size;
    size_t m_Row;
    size_t m_Column;
};

template<class T>
class ProxyMatrix
{
public:
    ProxyMatrix( MatrixT_t<T>& data, size_t nRow, const T& default, size_t& size ) 
        : m_Data(data), m_NumRow(nRow), m_Default(default), m_Size(size)
    {}

    decltype(auto) operator[]( size_t nColumn )
    {
        return ProxyValue<T>( m_Data, m_NumRow, nColumn, m_Default, m_Size );
    }
   /* const T& operator[]( size_t nColumn ) const
    {
        return m_Data[m_NumRow][nColumn];
    }*/
private:
    size_t m_NumRow;
    MatrixT_t<T>& m_Data;
    const T& m_Default;
    size_t& m_Size;
};

template<class T>
class Matrix
{
public:
    using value_type = T;

    Matrix( const T& defaultValue = T() ) 
        : m_Default( defaultValue )
    {
    }
    decltype( auto ) operator[]( size_t nRow )
    {
        return ProxyMatrix<T>( m_Data, nRow, m_Default, m_Size );
    }
    decltype( auto ) operator[]( size_t nRow ) const
    {
        return ProxyMatrix<T>( m_Data, nRow, m_Default, m_Size );
    }

    iterator_matrix<T> begin()
    {
        ColumnT_t<T>::iterator c;
        if( m_Data.empty() == false )
            c = m_Data.begin()->second.begin();
        return iterator_matrix<T>( m_Data, m_Data.begin(), c );
    }
    iterator_matrix<T> end()
    {
        ColumnT_t<T>::iterator c;        
        return iterator_matrix<T>( m_Data, m_Data.end(), c );
    }

    size_t size() const
    {
        return m_Size;
    }
private:
  
    std::map<size_t, std::map<size_t, T> > m_Data;
    size_t m_Size = 0;
    T m_Default;
};


int main()
{
    Matrix<int> matrix( -1 );
    assert( matrix.size() == 0 );

    auto a = matrix[0][0];
    assert( a == -1 );

    assert( matrix.size() == 0 );
    matrix[100][100] = 314;

    assert( matrix[100][100] == 314 );
    assert( matrix.size() == 1 );

    
    for( auto c : matrix )
    {
        int x, y, v;
        std::tie( x, y, v ) = c;
        std::cout << x << ' ' << y << ' ' << v << std::endl;
    }


    Matrix<int> mat( 0 );
    for( size_t i = 0; i < 10; ++i )
    {
        mat[i][i] = i;
        mat[i][9 - i] = 9 - i;
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
        std::cout << x << ' ' << y << ' ' << v << std::endl;
    }

    return 0;
}