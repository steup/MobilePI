#pragma once

#include <iostream>
#include <ctime>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

class Log : public boost::filesystem::ofstream{
  private:
    boost::filesystem::path mFileName;

    std::ostream& date(){
      char buffer[128];
      std::time_t now=std::time(nullptr);
      std::strftime(buffer, sizeof(buffer), "%F %T", std::localtime(&now));
      static_cast<std::ostream&>(*this) << "[" << buffer << "] ";
      return *this;
    }

  public:
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    Log(const boost::filesystem::path& fileName) : mFileName(fileName) {
      boost::filesystem::create_directory(fileName.parent_path());
      open(fileName, std::ios_base::app);
      date() << "Opening Log: " << mFileName.native() << std::endl;
    }

    virtual ~Log(){
      *this << "Closing Log: " << mFileName.native() << std::endl;
    }

    template<typename T>
    std::ostream& operator<<(T value){
      date();
      return static_cast<std::ostream&>(*this) << value;
    }
    std::ostream& operator<<(std::ostream& (*func)(std::ostream&)){
      return static_cast<std::ostream&>(*this) << func;
    }
};
