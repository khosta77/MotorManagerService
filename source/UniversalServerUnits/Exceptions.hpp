#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <ctime>
#include <exception>
#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

class MyException : public std::exception
{
  public:
    explicit MyException( const std::string &msg = "" );

    const char *what() const noexcept override;

  private:
    std::string m_msg;
};

class ErrorReadingFromSocket : public MyException
{
  public:
    const int sock;

    ErrorReadingFromSocket( const int socket_ );
};

class NotCorrectMessageToSend : public MyException
{
  public:
    NotCorrectMessageToSend();
};

class ErrorWritingToSocket : public MyException
{
  public:
    const int sock;

    ErrorWritingToSocket( const int socket_ );
};

class DeserializeJsonNoKey : public MyException
{
  public:
    DeserializeJsonNoKey( const std::string &name )
        : MyException(
              std::format( "In Deserialize json problem with {}", name ) )
    {
    }
};

class DeserializeJsonElementSomeProblem : public MyException
{
  public:
    DeserializeJsonElementSomeProblem( const std::string &name )
        : MyException( std::format(
              "In Deserialize json problem with element {}", name ) )
    {
    }
};

class SocketNotCreate : public MyException
{
  public:
    SocketNotCreate() : MyException( "Socket not create" ) {}
};

class BindFailure : public MyException
{
  public:
    BindFailure() : MyException( "bind some problem" ) {}
};

class ListenException : public MyException
{
  public:
    const int code;
    ListenException( const int code_ )
        : MyException( "Listen error" ), code( code_ )
    {
    }
};

class POLLDestroyed : public MyException
{
  public:
    POLLDestroyed() : MyException( "poll -" ) {}
};

class ModuleFT2xxException : public MyException
{
  private:
    std::unordered_map<int, std::string> ftdiErrorCodes_ = {
        { 1, "FT_INVALID_HANDLE" },
        { 2, "FT_DEVICE_NOT_FOUND" },
        { 3, "FT_DEVICE_NOT_OPENED" },
        { 4, "FT_IO_ERROR" },
        { 5, "FT_INSUFFICIENT_RESOURCES" },
        { 6, "FT_INVALID_PARAMETER" },
        { 7, "FT_INVALID_BAUD_RATE" },
        { 8, "FT_DEVICE_NOT_OPENED_FOR_ERASE" },
        { 9, "FT_DEVICE_NOT_OPENED_FOR_WRITE" },
        { 10, "FT_FAILED_TO_WRITE_DEVICE" },
        { 11, "FT_EEPROM_READ_FAILED" },
        { 12, "FT_EEPROM_WRITE_FAILED" },
        { 13, "FT_EEPROM_ERASE_FAILED" },
        { 14, "FT_EEPROM_NOT_PRESENT" },
        { 15, "FT_EEPROM_NOT_PROGRAMMED" },
        { 16, "FT_INVALID_ARGS" },
        { 17, "FT_NOT_SUPPORTED" },
        { 18, "FT_OTHER_ERROR" } };
    std::string m_msg;

    void currentTime( std::ostream &os );

  public:
    ModuleFT2xxException( const unsigned int &code );
};

#endif // EXCEPTION_HPP_
