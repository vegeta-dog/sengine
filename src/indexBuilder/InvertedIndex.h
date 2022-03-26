#pragma once

#include <string>
#include <list>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/list.hpp>

namespace indexBuilder::InvertedIndex
{
    class InvertedIndex_List
    {

        friend class boost::serialization::access;

    public:
        InvertedIndex_List(std::string key);
        ~InvertedIndex_List()=default;

        // 倒排列表的key
        std::string key;
        // 倒排列表的 list
        std::list<std::string> list;
        
        // 声明用于序列化的模板函数
        template<class Archive>
        void serialize(Archive &ar, const unsigned int version)
        {
            ar& key;
            ar& list;
        }
    };

    InvertedIndex_List::InvertedIndex_List(std::string key)
    {
        this->key = key;
    }   

    // 倒排列表中的结点
    struct list_node
    {
        list_node(unsigned int id, unsigned int ofs)
        {
            idWebPage = id;
            offset = ofs;
        }
        unsigned int idWebPage; // 网页id
        unsigned int offset;    // 偏移量
    };
};
