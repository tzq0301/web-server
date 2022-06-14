//
// Created by 戴佳 on 2022/5/29.
//

#ifndef WEBSERVER_FILE_UTIL_HPP
#define WEBSERVER_FILE_UTIL_HPP

#include <fstream>

class FileUtil {
public:
    /**
     * 获得指定路径的文件
     *
     * @param filepath 文件名（路径）
     * @return 字节数组（字符串）与是否获取成功
     */
    static std::pair<std::string, bool> getContent(const std::string &filepath) {
        std::ifstream is(filepath, std::ios::in);

        if (!is.is_open())
            return std::make_pair("", false);

        std::string result((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        return std::make_pair(result, true);
    }

    /**
     * 获得 statics/ 文件夹下的文件
     *
     * @param filepath statics/ 文件夹下的文件名（路径），不包括 statics/
     * @return 字节数组（字符串）与是否获取成功
     */
    static std::pair<std::string, bool> getStaticResource(const std::string &filepath) {
        if (auto result = getContent("statics/" + filepath); result.second) {
            return std::make_pair(result.first, true);
        } else {
            return std::make_pair("", false);
        }
    }
};

#endif //WEBSERVER_FILE_UTIL_HPP
