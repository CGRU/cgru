#pragma once


#include <map>
#include <iostream>
#include <functional>
#include <string>

#include "common.h"

#include <QtCore>

namespace afermer
{

class RotateValue
{

public:
    AFERMER_TYPEDEF_SMART_PTRS(RotateValue)
    AFERMER_DEFINE_CREATE_FUNC(RotateValue)

    RotateValue();
    ~RotateValue() {};

    static size_t make_hash(const std::string& i_str)
    {
        std::hash<std::string> m_hash_fn;
        return m_hash_fn(i_str);
    }

    void addValue(int);
    QList<int> getValues() const;

protected:
    QList<int> m_list;

};

}

