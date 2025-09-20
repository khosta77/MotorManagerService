#ifndef NETWORK_SERIALIZER_HPP_
#define NETWORK_SERIALIZER_HPP_

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#include <iostream>
#include <vector>
#include <string>
#include <format>
#include <memory>

#include "judge.hpp"
#include "i_socket.hpp"
#include "exceptions.hpp"
#include "socket.hpp"
#include "i_core.hpp"

/*
 * Структура для установки взаимодействия с модулем сервера
 * */
BOOST_FUSION_DEFINE_STRUCT((pkg), WhoWantsToTalkToMe, (std::string, name))

/*
 * Структура с сообщением сервера, несущее в себе информацию о дальнейшем действие
 * */
BOOST_FUSION_DEFINE_STRUCT((pkg), Message, (int, id)(std::string, text))

/*
 * Сообщение о том, что по соответствующему id все было отработано
 * */
BOOST_FUSION_DEFINE_STRUCT((pkg), ImOkay, (int, status))

class NetworkSerializer
{
private:
    const size_t MAX_BUFFER_COUNT;
    std::unique_ptr<ISocket> socketInterface_;

public:
    NetworkSerializer();
    ~NetworkSerializer() = default;

    explicit NetworkSerializer(std::unique_ptr<ISocket>);
    explicit NetworkSerializer(const NetworkSerializer&) = delete;
    explicit NetworkSerializer(NetworkSerializer&&) = delete;

    NetworkSerializer& operator=(const NetworkSerializer&) = delete;
    NetworkSerializer& operator=(NetworkSerializer&&) = delete;

    /*
     * @brief Чтение из сокета сообщения/пачки сообщений, оканчивающихся на \n\n
     * @param socket_ сокет
     * */
    std::string readFromSock(const int socket_);

    /*
     * @brief Запись в сокет сообщения длины msg.size(), важно, данный метод,
     *        самостоятельно добавляет \n\n в конце
     * @param socket_ сокет в который отправлять
     * @param msg сообщение
     * */
    void writeToSock(const int socket_, std::string msg);

    /*
     * @brief Дробление всей посылки на малые части -> отдельные сообщения, дробление по \n\n
     * @param msg - входное ссобщение, состаящие из посылок разделенных \n\n
     *
     * Важно, \n\n вырезается, то есть \n\n -> 0 \n\n\n -> \n
     * */
    std::vector<std::string> split(const std::string& msg);

    /*
     * @brief Метод для десериализации строки json
     * @param json_str - строка
     * */
    template <typename FusionT>
    FusionT deserialize(std::string_view json_str)
    {
        Json json_obj = Json::parse(json_str);
        FusionT fusion_obj{};

        // Надо проверять так же json, которые приходят на вход. Просто из условия возможна такая ситуация,
        // что на вход придет либо не полный json. Либо с лишним элементом...
        std::unordered_map<std::string, bool> content(json_obj.size());
        for (const auto& it : json_obj.items())
            content[it.key()] = false;

        boost::fusion::for_each(
            boost::mpl::range_c<unsigned, 0, boost::fusion::result_of::size<FusionT>::value>(),
            [&](auto index) {
                using type = typename boost::fusion::result_of::value_at<FusionT, decltype(index)>::type;

                const auto name = boost::fusion::extension::struct_member_name<FusionT, index>::call();

                // Тут проверка на то, есть ли имя переменной, что хотим найти в json
                if (content.count(name) == 0)
                    throw DeserializeJsonNoKey(std::string(name));
                content[name] = true;

                if constexpr (is_valid_vector_t<type>::value)
                {
                    if constexpr (is_fusion_struct<typename type::value_type>() == 1)
                    {
                        const std::vector<Json> values = json_obj[name].template get<std::vector<Json>>();
                        type results(values.size());
                        size_t i = 0;
                        for (const auto& value : values)
                        {
                            const auto obj = deserialize<typename type::value_type>(value.dump());
                            results[i++] = obj;
                        }
                        boost::fusion::at_c<index>(fusion_obj) = results;
                    }
                    else
                    {
                        const type value = json_obj[name].template get<type>();
                        boost::fusion::at_c<index>(fusion_obj) = value;
                    }
                }
                else
                {
                    if constexpr (is_fusion_struct<type>() == 1)
                    {
                        const type value = deserialize<type>(json_obj[name].dump());
                        boost::fusion::at_c<index>(fusion_obj) = value;
                    }
                    else
                    {
                        const type value = json_obj[name].template get<type>();
                        boost::fusion::at_c<index>(fusion_obj) = value;
                    }
                }
            });

        // Если какой-то переменной не найдем, вернет ошибку
        for (const auto& [key, value] : content)
        {
            if (!value)
                throw DeserializeJsonElementSomeProblem(std::string(key));
        }

        return fusion_obj;
    }

    /*
     * @brief метод для сериализации структуру в json
     * @param fusion_obj структуру
     * */
    template <typename FusionT>
    std::string serialize(const FusionT& fusion_obj)
    {
        Json json_obj;

        boost::fusion::for_each(
            boost::mpl::range_c<unsigned, 0, boost::fusion::result_of::size<FusionT>::value>(),
            [&](auto index) {
                using type = typename boost::fusion::result_of::value_at<FusionT, decltype(index)>::type;

                const auto name = boost::fusion::extension::struct_member_name<FusionT, index>::call();
                if constexpr (is_valid_vector_t<type>::value)
                {
                    if constexpr (is_fusion_struct<typename type::value_type>() == 1)
                    {
                        const auto values = boost::fusion::at_c<index>(fusion_obj);
                        std::vector<Json> results(values.size());
                        size_t i = 0;
                        for (const auto& value : values)
                        {
                            std::string obj = serialize(value);
                            results[i++] = Json::parse(std::move(obj));
                        }
                        json_obj[name] = results;
                    }
                    else
                        json_obj[name] = boost::fusion::at_c<index>(fusion_obj);
                }
                else
                {
                    if constexpr (is_fusion_struct<type>() == 1)
                    {
                        const std::string obj = serialize(boost::fusion::at_c<index>(fusion_obj));
                        json_obj[name] = Json::parse(obj);
                    }
                    else
                        json_obj[name] = boost::fusion::at_c<index>(fusion_obj);
                }
            });

        return json_obj.dump();
    }
};

#endif // NETWORK_SERIALIZER_HPP_
