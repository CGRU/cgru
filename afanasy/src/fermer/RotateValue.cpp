#include "RotateValue.h"
#include <algorithm>    // std::rotate

using namespace afermer;

RotateValue::RotateValue()
{
  for (size_t i = 0; i < 25; ++i)
        m_list.append(0);
}


void RotateValue::addValue(int i_val)
{
    std::rotate(m_list.rbegin(),m_list.rbegin()+1,m_list.rend());
    m_list[0] = i_val;
}


QList<int> RotateValue::getValues() const
{
    return m_list;
}
