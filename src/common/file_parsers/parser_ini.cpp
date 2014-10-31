#include "common/file_parsers/parser_ini.h"

#include <fstream>

#ifdef BOOST_ENABLED
#include <boost/program_options/detail/config_file.hpp>
#endif

namespace common
{
    namespace file_parsers
    {
        namespace ini
        {
            bool parse_ini(const std::string& path, std::map<std::string, std::string> &ref)
            {
                 bool result = false;
                 std::ifstream config(path.c_str());
                 if(config){
                     std::set<std::string> options;
#ifdef BOOST_ENABLED
                     options.insert("*");
                     for (boost::program_options::detail::basic_config_file_iterator<char>  i(config, options), e ; i != e; ++i)
                     {
                          ref[i->string_key] = i->value[0];
                     }
#else
#pragma message("implement parsing")
#endif
                     result = true;
                     config.close();
                 }
                 return result;
            }
        }
    }
}
