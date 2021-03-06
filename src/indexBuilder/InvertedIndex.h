#pragma once

#include <string>
#include <utility>
#include <vector>
#include<set>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include<boost/serialization/vector.hpp>

namespace indexBuilder::InvertedIndex
{

    // 倒排列表中的结点
    class list_node
    {
    public:
        friend class boost::serialization::access;
        list_node(){};
        list_node(unsigned int id, unsigned int ofs)
        {
            idWebPage = id;
            offset = ofs;
        }

        unsigned int idWebPage; // 网页id
        unsigned int offset;    // 偏移量

        template <class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
            ar &idWebPage;
            ar &offset;
        }
    };

    class InvertedIndex_List
    {

        friend class boost::serialization::access;

    public:
        InvertedIndex_List() = default;
        explicit InvertedIndex_List(std::string key){
        this->key = std::move(key);
    };
        ~InvertedIndex_List() = default;

        // 倒排列表的key
        std::string key;
        // 倒排列表的 list
        std::vector<indexBuilder::InvertedIndex::list_node> list;
        std::set<unsigned int> page_set;
        // 声明用于序列化的模板函数
        template <class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
            ar &key;
            ar &list;
            ar &page_set;
        }

    private:
    };

    static bool cmp_list_node(const indexBuilder::InvertedIndex::list_node &a, const indexBuilder::InvertedIndex::list_node &b)
    {
        if (a.idWebPage < b.idWebPage)
            return true;
        else if (a.idWebPage > b.idWebPage)
            return false;
        else
            return (a.offset < b.offset);
    }

    
    

};
