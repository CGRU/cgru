#include <sstream>
#include <boost/circular_buffer.hpp>

// https://onegazhang.wordpress.com/2008/09/30/redirect-python-stdoutstderr-via-boost-python/
class PythonStdIoRedirect {
public:
    typedef boost::circular_buffer<std::string> ContainerType;
    void Write( std::string const& str ) {
        if (m_outputs.capacity()<100)
            m_outputs.resize(100);
        m_outputs.push_back(str);
    }
    static std::string GetOutput()
    {
        std::string ret;
        std::stringstream ss;
        for(boost::circular_buffer<std::string>::const_iterator it=m_outputs.begin();
            it!=m_outputs.end();
            it++)
        {
            ss << *it;
        }
        m_outputs.clear();
        ret =  ss.str();
        return ret;
    }
private:
    static ContainerType m_outputs; // must be static, otherwise output is missing
};