#pragma once

#include <vector>
#include <random>
#include <algorithm>

namespace sudoku
{
    using skc_type = int;
    using sk_row = std::vector<skc_type>;
    using sk_vec = std::vector<skc_type>;
    using sk_matrix = std::vector<sk_row>;

    using namespace std;

    struct candidate_2d
    {
        sk_vec operator()(const sk_matrix& skm, size_t row, size_t col)
        {
            sk_vec candi;
            size_t sz = skm.size();
            vector<bool> trac(sz + 1, false);
            for (size_t i = 0; i < sz; i++)
            {
                trac[skm[row][i]] = true;
                trac[skm[i][col]] = true;
            }
            int candCount = 0;
            sk_vec cand(sz, 0);
            for (skc_type i = 1; i <= (skc_type)sz; i++)
            {
                if (!trac[i])
                {
                    candi.push_back(i);
                }
            }
            return candi;
        }
    };

    struct candidate_3d
    {
        sk_vec operator()(const sk_matrix& skm, size_t row, size_t col)
        {
            sk_vec candi;
            size_t sz = skm.size();
            vector<bool> trac(sz + 1, false);
            size_t _3td_r = row / 3;
            size_t _3td_c = col / 3;
            for (size_t i = 0; i < sz; i++)
            {
                trac[skm[row][i]] = true;
                trac[skm[i][col]] = true;
                trac[skm[_3td_r * 3 + i / 3][_3td_c * 3 + i % 3]] = true;
            }
            int candCount = 0;
            sk_vec cand(sz, 0);
            for (skc_type i = 1; i <= (skc_type)sz; i++)
            {
                if (!trac[i])
                {
                    candi.push_back(i);
                }
            }
            return candi;
        }
    };

    struct random_order
    {
        struct idxWeight
        {
            size_t idx;
            size_t weight;
        };

        sk_vec operator()(const sk_vec& vec)
        {
            static random_device s_ro_rdev;
            static mt19937 s_ro_eng(s_ro_rdev());

            if (vec.empty())
            {
                return vec;
            }

            vector<idxWeight> dw(vec.size());
            uniform_int_distribution<size_t> uirang(0, 10000);
            for (size_t i = 0; i < vec.size(); i++)
            {
                dw[i] = { i, uirang(s_ro_eng) };
            }
            sort(dw.begin(), dw.end(), [](typename vector<idxWeight>::const_reference l, typename vector<idxWeight>::const_reference r) { return l.weight > r.weight; });
            vector<int> result(vec.size());
            for (size_t i = 0; i < dw.size(); i++)
            {
                result[i] = vec[dw[i].idx];
            }
            return result;
        }
    };

    struct sequence_order
    {
        sk_vec operator()(const sk_vec& vec)
        {
            return vec;
        }
    };

    template<size_t _order = 6, typename _Candidate = candidate_2d, typename _OrderSeq = random_order>
    class sk_seek
    {
    public:

        static constexpr size_t _MyOrder = _order;

        using _MyCandidateType = _Candidate;
        using _MyOrderSeqType = _OrderSeq;

        sk_seek() : m_skm(_order, sk_row(_order, 0))
        {
            static_assert(_order >= 3 && _order <= 9, "invalid order for sudoku...");
        }
        ~sk_seek() {

        }

        sk_seek(const sk_seek& l) noexcept {
            *this = l;
        };

        sk_seek& operator=(const sk_seek& l) noexcept {
            m_skm = l.m_skm;
            return *this;
        };

        sk_seek(sk_seek&& r) noexcept = default;
        sk_seek& operator=(sk_seek&& l) noexcept = default;

        explicit sk_seek(const sk_matrix& src) {
            if (src.size() != _order || (!src.empty() && src.size() != _order))
            {
                throw std::invalid_argument("source sk matrix order is not equel to _order.");
            }
            m_skm = src;
            _check_item_con();
        }

        sk_seek(sk_matrix&& src) {
            if (src.size() != _order || (!src.empty() && src.size() != _order))
            {
                throw std::invalid_argument("source sk matrix order is not equel to _order.");
            }
            m_skm = std::move(src);
            _check_item_con();
        }

        const sk_matrix& gen() {
            size_t _gen_count = 0;
            while (!_gen_sudoku(m_skm, 0))
            {
                if (++_gen_count > _order)
                {
                    throw std::out_of_range("sk_seek generate sudoku seek out of times.");
                }
            }
            return m_skm;
        }

        const sk_matrix& content() const {
            return m_skm;
        }

        const sk_matrix& next()
        {
            clean();
            return gen();
        }

        void clean() {
            m_skm.assign(_order, sk_row(_order, 0));
        }

        bool validate() const noexcept {
            for (size_t r = 0; r < _order; ++r)
            {
                for (size_t c = 0; c < _order; c++)
                {
                    if (0 == m_skm[r][c] || (_Candidate()(m_skm, r, c)).empty())
                    {
                        return false;
                    }
                }
            }
            bool bval = true;
        }
    protected:

        bool _gen_sudoku(sk_matrix& src, size_t nitem)
        {
            size_t rows = src.size();
            size_t nCount = rows * rows;
            if (nitem >= nCount)
            {
                return true;
            }

            size_t row = nitem / rows;
            size_t col = nitem % rows;

            if (src[row][col] != 0)
            {
                return _gen_sudoku(src, nitem + 1);
            }

            auto candidate = _OrderSeq()(_Candidate()(src, row, col));
            if (!candidate.empty())
            {
                for (size_t i = 0; i < candidate.size(); i++)
                {
                    src[row][col] = candidate[i];
                    if (_gen_sudoku(src, nitem + 1))
                    {
                        return true;
                    }
                }
                src[row][col] = 0;
            }

            return false;
        }

        void _check_item_con()
        {
            for (size_t r = 0; r < _order; ++r)
            {
                for (size_t c = 0; c < _order; c++)
                {
                    if (0 >= m_skm[r][c] || m_skm[r][c] > static_cast<skc_type>(_order))
                    {
                        m_skm[r][c] = 0;
                    }
                }
            }
        }

    private:
        sk_matrix m_skm;
    };

    using seek_norm = sk_seek<>;
    using seek_6 = sk_seek<6>;
    using seek_9 = sk_seek<9, candidate_3d>;

    template<typename _Ty>
    struct is_suduku : public std::false_type
    {
    };

    template<size_t _Order, typename _Candidate, typename _OrderSeq>
    struct is_suduku<sk_seek<_Order, _Candidate, _OrderSeq>> : public std::true_type
    {
    };
    
    template<size_t _IncRatio = 2, typename _Seek = seek_norm, typename _RandomAlgorithm = random_order>
    class incomplete
    {
    public:
        using _MySeekType = typename std::enable_if<is_suduku<typename _Seek>::value, _Seek>::type;
        static constexpr size_t _MyIncRatio = _IncRatio;
    public:
        incomplete()
        {
            static_assert(_MySeekType::_MyOrder > 0 && _MySeekType::_MyOrder <= 9, "Invalidate sudoku seek type.");
            static_assert(_IncRatio < _MySeekType::_MyOrder, "incomplete number is too large...");
        }

        sk_matrix operator()(const _MySeekType& seek)
        {
            constexpr size_t _Nums = _MySeekType::_MyOrder * _MySeekType::_MyOrder;
            constexpr size_t _incNums = _MyIncRatio * _MySeekType::_MyOrder;
            static_assert(_incNums < _Nums, "Too many numbers should be delete.");
            
            auto skm = seek.content();
            sk_vec vk = sequence_vec(std::make_index_sequence<_Nums>{});
            vk = _RandomAlgorithm()(vk);
            for (size_t r = 0; r < _incNums; r++)
            {
                skm[vk[r] / _MySeekType::_MyOrder][vk[r] % _MySeekType::_MyOrder] = 0;
            }
            return skm;
        }

    private:
        template<size_t ...N>
        constexpr sk_vec sequence_vec(std::index_sequence<N...>)
        {
            return sk_vec{ N... };
        }

    };

}

namespace std
{
    template <class _Elem, class _Traits>
    basic_ostream<_Elem, _Traits>& operator<<(basic_ostream<_Elem, _Traits>& _Ostr, const sudoku::sk_matrix& _Val)
    {
        for (size_t i = 0; i < _Val.size(); i++)
        {
            std::copy(_Val[i].begin(), _Val[i].end(), ostream_iterator<sudoku::skc_type, _Elem, _Traits>(_Ostr, " "));
            _Ostr << endl;
        }
        return _Ostr;
    }
}