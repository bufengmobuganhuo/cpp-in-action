#include "ConfigLoader.h"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <drogon/drogon.h>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#error "ConfigLoader requires C++17 filesystem support"
#endif

namespace
{
    std::string trim(const std::string& value)
    {
        const auto begin = value.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos)
        {
            return {};
        }
        const auto end = value.find_last_not_of(" \t\r\n");
        return value.substr(begin, end - begin + 1);
    }

    std::string strip_quotes(const std::string& value)
    {
        if (value.size() >= 2)
        {
            const char first = value.front();
            const char last = value.back();
            if ((first == '"' && last == '"') || (first == '\'' && last == '\''))
            {
                return value.substr(1, value.size() - 2);
            }
        }
        return value;
    }

    bool path_exists(const fs::path& path)
    {
        std::error_code ec;
        return fs::exists(path, ec);
    }

    fs::path find_project_root()
    {
        fs::path current = fs::current_path();
        for (int i = 0; i < 5; ++i)
        {
            if (path_exists(current / "config" / "config.json") || path_exists(current / "CMakeLists.txt"))
            {
                return current;
            }
            if (!current.has_parent_path())
            {
                break;
            }
            current = current.parent_path();
        }
        return fs::current_path();
    }

    void load_dot_env(const fs::path& root)
    {
        const fs::path env_path = root / ".env";
        if (!path_exists(env_path))
        {
            return;
        }

        std::ifstream input(env_path);
        if (!input.is_open())
        {
            throw std::runtime_error("failed to open .env: " + env_path.string());
        }

        std::string line;
        while (std::getline(input, line))
        {
            std::string text = trim(line);
            if (text.empty() || text.front() == '#')
            {
                continue;
            }
            if (text.rfind("export ", 0) == 0)
            {
                text = trim(text.substr(7));
            }

            const auto pos = text.find('=');
            if (pos == std::string::npos)
            {
                continue;
            }

            const std::string key = trim(text.substr(0, pos));
            const std::string value = strip_quotes(trim(text.substr(pos + 1)));
            if (!key.empty())
            {
                setenv(key.c_str(), value.c_str(), 0);
            }
        }
    }

    std::string get_env_value(const std::string& key)
    {
        const char* value = std::getenv(key.c_str());
        return value == nullptr ? "" : value;
    }

    std::string get_profile()
    {
        std::string profile = get_env_value("PROFILES_ACTIVE");
        if (profile.empty())
        {
            profile = get_env_value("STOCK_LAB_ENV");
        }
        return profile.empty() ? "dev" : profile;
    }

    Json::Value read_json_file(const fs::path& path)
    {
        std::ifstream input(path);
        if (!input.is_open())
        {
            throw std::runtime_error("failed to open config file: " + path.string());
        }

        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        if (!Json::parseFromStream(builder, input, &root, &errors))
        {
            throw std::runtime_error("failed to parse config file: " + path.string() + ", error=" + errors);
        }
        return root;
    }

    void merge_json(Json::Value& base, const Json::Value& override)
    {
        if (!base.isObject() || !override.isObject())
        {
            base = override;
            return;
        }

        for (const auto& key : override.getMemberNames())
        {
            if (base.isMember(key))
            {
                merge_json(base[key], override[key]);
            }
            else
            {
                base[key] = override[key];
            }
        }
    }

    std::string resolve_placeholder(const std::string& expression)
    {
        const auto colon_pos = expression.find(':');
        const std::string key = colon_pos == std::string::npos
                                    ? expression
                                    : expression.substr(0, colon_pos);
        const std::string default_value = colon_pos == std::string::npos
                                              ? ""
                                              : expression.substr(colon_pos + 1);

        std::string value = get_env_value(key);
        if (!value.empty())
        {
            return value;
        }
        if (colon_pos != std::string::npos)
        {
            return default_value;
        }
        throw std::runtime_error("missing required environment variable: " + key);
    }

    std::string resolve_string(const std::string& value)
    {
        std::string result;
        std::size_t pos = 0;
        while (pos < value.size())
        {
            const auto start = value.find("${", pos);
            if (start == std::string::npos)
            {
                result.append(value.substr(pos));
                break;
            }

            result.append(value.substr(pos, start - pos));
            const auto end = value.find('}', start + 2);
            if (end == std::string::npos)
            {
                throw std::runtime_error("invalid config placeholder: " + value);
            }

            result.append(resolve_placeholder(value.substr(start + 2, end - start - 2)));
            pos = end + 1;
        }
        return result;
    }

    void resolve_env_placeholders(Json::Value& value)
    {
        if (value.isObject())
        {
            for (const auto& key : value.getMemberNames())
            {
                resolve_env_placeholders(value[key]);
            }
            return;
        }

        if (value.isArray())
        {
            for (Json::ArrayIndex i = 0; i < value.size(); ++i)
            {
                resolve_env_placeholders(value[i]);
            }
            return;
        }

        if (value.isString())
        {
            value = resolve_string(value.asString());
        }
    }
}

void util::ConfigLoader::load()
{
    const fs::path root = find_project_root();
    load_dot_env(root);

    const std::string profile = get_profile();
    const fs::path config_dir = root / "config";
    Json::Value config = read_json_file(config_dir / "config.json");

    const fs::path profile_config = config_dir / ("config." + profile + ".json");
    if (path_exists(profile_config))
    {
        Json::Value profile_value = read_json_file(profile_config);
        merge_json(config, profile_value);
    }
    else
    {
        LOG_WARN << "profile config file not found, profile=" << profile
                 << ", path=" << profile_config.string();
    }

    resolve_env_placeholders(config);
    drogon::app().loadConfigJson(std::move(config));
}
