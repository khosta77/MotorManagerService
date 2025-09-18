#ifndef TRANSFORMATIONCORE_HPP_
#define TRANSFORMATIONCORE_HPP_

#include <bit>

#include "ModuleFT232RL.hpp"
#include "UniversalServer.hpp"
#include "dataframe.hpp"
#include "utils.hpp"

#define MOTOR_COUNT 16
#define MOTOR_CELLS_COUNT 4

class MotorControlServiceCore : public UniversalServerCore,
                                public UniversalServerMethods
{
  public:
    MotorControlServiceCore();
    MotorControlServiceCore( const std::shared_ptr<ModuleFT232RL> & );
    ~MotorControlServiceCore() override;

    void Init() override;
    void Process( const int id, const std::string &name,
                  const std::string &msg ) override;
    void Launch() override;
    void Stop() override;

  private:
    std::shared_ptr<ModuleFT232RL> module_;

    void checkMotorNum( const uint32_t, const std::string & );
    void setMotor( std::vector<uint32_t> &, const size_t, const uint32_t,
                   const mcs::MotorSettings & );
    void setMotors( std::vector<uint32_t> &, const size_t, const uint32_t,
                    const mcs::MotorsSettings & );
    void setCAE( std::vector<uint32_t> &,
                 const mcs::MotorsGroupSettings & ); // configureAllEngines
    void setSS( std::vector<uint32_t> &,
                const mcs::MotorsGroupSettings & ); // startSimultaneously
    void setSI( std::vector<uint32_t> &,
                const mcs::MotorsGroupSettings & ); // startImmediately
};

#endif // TRANSFORMATIONCORE_HPP_
