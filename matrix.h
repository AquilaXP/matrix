#pragma once

#include <map>
#include <tuple>
#include <functional>

namespace detail{

//===========================================================/
//  Шаблон для генерации tuple с N элементов одного типа     /
//===========================================================/
template <size_t I, class T, class... P>
struct gen_tuple
{
    using type = typename gen_tuple<I - 1, T, T, P...>::type;
};

template<class T, class... P>
struct gen_tuple<0, T, P...>
{
    using type = std::tuple<P...>;
};

template<class T, size_t I>
using gen_tuple_t = typename gen_tuple<I, T>::type;
//===========================================================/

/// Тип индексов
template<size_t Size>
using index_t = gen_tuple_t<size_t, Size>;

/// Тип контейенра для хранения разряженной матрицы
template<class Ty, size_t Size>
using mat_t = std::map<gen_tuple_t<size_t, Size>, Ty>;

/// Прокси-класс для работы со значением
template<class Ty, size_t N>
class ProxyValue
{
public:
    ProxyValue( mat_t<Ty, N>& data, index_t<N>& indices, const Ty& default_value )
        : m_data( data ), m_indices( indices ), m_default( default_value )
    {}

    ProxyValue& operator = ( const Ty& value )
    {
        if( value == m_default )
        {
            auto iter = m_data.find( m_indices );
            if( iter != m_data.end() )
            {
                m_data.erase( iter );
            }
        }
        else
        {
            m_data[m_indices] = value;
        }
        return ( *this );
    }

    operator Ty()
    {
        auto iter = m_data.find( m_indices );
        if( iter != m_data.end() )
        {
            return iter->second;
        }
        return m_default;
    }

private:
    mat_t<Ty, N>& m_data;
    const Ty& m_default;
    index_t<N>& m_indices;
};

/// Прокси-класс для реализация N мерного оператора []
template<class Ty, size_t N, size_t I>
class ProxyMatrix
{
public:
    ProxyMatrix( mat_t<Ty, N>& data, index_t<N>& indices, const Ty& default_value )
        : m_data( data ), m_indices( indices ), m_default( default_value )
    {}

    auto operator[]( size_t index )
    {
        std::get<I - 1>( m_indices ) = index;
        return ProxyMatrix<Ty, N, I - 1>( m_data, m_indices, m_default );
    }

private:
    mat_t< Ty, N >& m_data;
    const Ty& m_default;
    index_t< N >& m_indices;
};

template<class Ty, size_t N>
class ProxyMatrix<Ty, N, 1>
{
public:
    ProxyMatrix( mat_t<Ty, N>& data, index_t<N>& indices, const Ty& default_value )
        : m_data( data ), m_indices( indices ), m_default( default_value )
    {}

    auto operator[]( size_t index )
    {
        std::get<0>( m_indices ) = index;
        return ProxyValue<Ty, N>( m_data, m_indices, m_default );
    }

private:
    mat_t<Ty, N>& m_data;
    const Ty& m_default;
    index_t<N>& m_indices;
};

} // namespace detail

/// Итератор
template<class T, size_t N>
struct iterator_matrix
{
    using iterator_category = typename detail::mat_t<T, N>::iterator::iterator_category;

    iterator_matrix() = default;
    iterator_matrix( const iterator_matrix& rhs ) = default;
    iterator_matrix( typename detail::mat_t<T, N>::iterator iter )
        : m_iter( iter )
    {
    }

    auto operator*()
    {
        return std::tuple_cat( m_iter->first, std::make_tuple( std::ref( m_iter->second ) ) );
    }

    auto operator->()
    {
        return std::tuple_cat( m_iter->first, std::make_tuple( std::ref( m_iter->second ) ) );
    }

    iterator_matrix& operator++()
    {
        ++m_iter;
        return ( *this );
    }

    iterator_matrix operator++( int )
    {
        iterator_matrix it( m_iter );
        ++( *this );
        return it;
    }

    iterator_matrix& operator--()
    {
        --m_iter;
        return ( *this );
    }

    iterator_matrix operator--( int )
    {
        iterator_matrix it( m_iter );
        --( *this );
        return it;
    }

    bool operator == ( const iterator_matrix& rhs ) const
    {
        return m_iter == rhs.m_iter;
    }

    bool operator != ( const iterator_matrix& rhs ) const
    {
        return !( ( *this ) == rhs );
    }

private:
    typename detail::mat_t<T, N>::iterator m_iter;
};

/// Класс матрицы
template<class Ty, size_t N = 2>
class Matrix
{
public:
    using value_type = Ty;
    using iterator = iterator_matrix<Ty, N>;

    Matrix( const Ty& default_value = Ty() )
        : m_default( default_value )
    {
    }

    auto operator[]( size_t index )
    {
        std::get<N - 1>( m_need_index ) = index;
        return detail::ProxyMatrix<Ty, N, N - 1>( m_data, m_need_index, m_default );
    }

    iterator begin()
    {
        return iterator( m_data.begin() );
    }

    iterator end()
    {
        return iterator( m_data.end() );
    }

    size_t size() const
    {
        return m_data.size();
    }
    
private:
    detail::index_t<N> m_need_index;
    detail::mat_t<Ty, N> m_data;
    Ty m_default;
};
